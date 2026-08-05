// Simple stereo chorus by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

static const float MAX_DELAY_SEC = 0.05f;  // buffer headroom, actual sweep is much smaller
static const float CENTER_DELAY = 0.015f;  // 15ms base tap
static const float DEPTH_MAX = 0.010f;     // +/-10ms LFO sweep at full depth

struct Chorusah : Module {
	enum ParamIds {
		_RATE_PARAM,
		_DEPTH_PARAM,
		_MIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_RATE_INPUT,
		_AUDIO_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_LEFT_OUTPUT,
		_RIGHT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	std::vector<float> buffer[16];
	int writeIndex[16] = { 0 };
	int bufferSize = 0;
	float phase[16] = { 0.f };
	int lastChannels = 0;

	// Smoothed so a sudden Depth change (knob snap, scroll, typed value) can't jump the read
	// pointer instantly - the LFO sweep itself is already smooth, Depth scaling it wasn't
	float smoothedDepth = 0.f;
	bool depthInit = false;

	Chorusah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_RATE_PARAM, 0.1f, 5.f, 0.8f, "Rate", " Hz");
		configParam(_DEPTH_PARAM, 0.f, 1.f, 0.5f, "Depth", "%", 0.f, 100.f);
		configParam(_MIX_PARAM, 0.f, 1.f, 0.5f, "Mix", "%", 0.f, 100.f);
		configInput(_RATE_INPUT, "Rate CV");
		configInput(_AUDIO_INPUT, "Audio");
		configOutput(_LEFT_OUTPUT, "Left");
		configOutput(_RIGHT_OUTPUT, "Right");
		configBypass(_AUDIO_INPUT, _LEFT_OUTPUT);
		configBypass(_AUDIO_INPUT, _RIGHT_OUTPUT);
	}

	void onReset() override {
		for (int c = 0; c < 16; c++) {
			std::fill(buffer[c].begin(), buffer[c].end(), 0.f);
			writeIndex[c] = 0;
			phase[c] = 0.f;
		}
		depthInit = false;
	}

	void onBypass(const BypassEvent& e) override { onReset(); }
	void onUnBypass(const UnBypassEvent& e) override { onReset(); }

	// Fractional read `delaySec` behind the current write position
	float readTap(int c, float delaySec, float sampleRate) {
		float delaySamples = clamp(delaySec * sampleRate, 1.f, (float)(bufferSize - 1));
		float readPos = writeIndex[c] - delaySamples;
		while (readPos < 0.f)
			readPos += bufferSize;
		// Float rounding in the wrap-around above can land readPos exactly on bufferSize
		// (a tiny sub-ULP deficit vanishes when added to a much larger number) - guard it
		if (readPos >= bufferSize)
			readPos -= bufferSize;
		int idx0 = (int)readPos;
		float frac = readPos - idx0;
		int idx1 = (idx0 + 1) % bufferSize;
		return buffer[c][idx0] * (1.f - frac) + buffer[c][idx1] * frac;
	}

	void process(const ProcessArgs& args) override;
};

void Chorusah::process(const ProcessArgs& args) {
	// Buffer sized for the worst-case delay time at the current sample rate
	int wantedSize = (int)std::ceil(args.sampleRate * MAX_DELAY_SEC) + 1;
	if (bufferSize != wantedSize) {
		bufferSize = wantedSize;
		for (int c = 0; c < 16; c++) {
			buffer[c].assign(bufferSize, 0.f);
			writeIndex[c] = 0;
		}
	}

	float rateKnob = params[_RATE_PARAM].getValue();
	float mix = params[_MIX_PARAM].getValue();

	float targetDepth = params[_DEPTH_PARAM].getValue() * DEPTH_MAX;
	if (!depthInit) {
		smoothedDepth = targetDepth;
		depthInit = true;
	}
	const float smoothTime = 0.01f;
	float depthSmooth = 1.f - std::exp(-1.f / (smoothTime * args.sampleRate));
	smoothedDepth += (targetDepth - smoothedDepth) * depthSmooth;
	float depth = smoothedDepth;

	int channels = max(inputs[_AUDIO_INPUT].getChannels(), 1);
	// A channel that just came back into use had its delay line frozen, not silenced -
	// clear it so reactivating a voice doesn't resurface old, unrelated audio as a pop
	for (int c = lastChannels; c < channels; c++) {
		std::fill(buffer[c].begin(), buffer[c].end(), 0.f);
		writeIndex[c] = 0;
		phase[c] = 0.f;
	}
	lastChannels = channels;

	for (int c = 0; c < channels; c++) {
		float rate = rateKnob;
		if (inputs[_RATE_INPUT].isConnected())
			rate = clamp(rate + inputs[_RATE_INPUT].getPolyVoltage(c), 0.1f, 5.f);
		phase[c] += rate * args.sampleTime;
		phase[c] -= std::floor(phase[c]);

		// L and R LFOs run in phase opposition for stereo widening
		float lfoL = mmSin(phase[c] * M_2PI);
		float lfoR = mmSin((phase[c] + 0.5f) * M_2PI);

		float wetL = readTap(c, CENTER_DELAY + depth * lfoL, args.sampleRate);
		float wetR = readTap(c, CENTER_DELAY + depth * lfoR, args.sampleRate);

		float in = inputs[_AUDIO_INPUT].getPolyVoltage(c);
		buffer[c][writeIndex[c]] = in;
		writeIndex[c] = (writeIndex[c] + 1) % bufferSize;

		outputs[_LEFT_OUTPUT].setVoltage(crossfade(in, wetL, mix), c);
		outputs[_RIGHT_OUTPUT].setVoltage(crossfade(in, wetR, mix), c);
	}
	outputs[_LEFT_OUTPUT].setChannels(channels);
	outputs[_RIGHT_OUTPUT].setChannels(channels);
}

struct ChorusahWidget : ModuleWidget {
	ChorusahWidget(Chorusah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Chorusah.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 46.0)), module, Chorusah::_RATE_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 68.0)), module, Chorusah::_DEPTH_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 79.0)), module, Chorusah::_MIX_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 57.0)), module, Chorusah::_RATE_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, Chorusah::_AUDIO_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Chorusah::_LEFT_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, Chorusah::_RIGHT_OUTPUT));
	}
};

Model* modelChorusah = createModel<Chorusah, ChorusahWidget>("Chorusah");
