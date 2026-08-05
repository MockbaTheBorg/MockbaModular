// Simple delay by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

static const float MAX_DELAY_SEC = 2.f;

// Time knob is squared for finer control at short delays; tooltip shows the actual time
struct _DelayTime : ParamQuantity {
	std::string getDisplayValueString() override {
		float norm = getValue();
		float sec = 0.001f + (MAX_DELAY_SEC - 0.001f) * norm * norm;
		if (sec < 1.f)
			return string::f("%.1f ms", sec * 1000.f);
		return string::f("%.3f s", sec);
	}
};

struct Delayah : Module {
	enum ParamIds {
		_TIME_PARAM,
		_FEEDBACK_PARAM,
		_MIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_TIME_INPUT,
		_LEFT_INPUT,
		_RIGHT_INPUT,
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

	// Dual mono: side 0 = left, side 1 = right, sharing the same Time/Feedback/Mix knobs
	std::vector<float> buffer[2][16];
	int writeIndex[2][16] = { 0 };
	int bufferSize = 0;

	// Smoothed so the read pointer glides instead of jumping when Time changes - an instant jump
	// would inject a click that then repeats and decays forever through the feedback loop
	float smoothedDelaySamples[16] = { 0.f };
	bool smoothInit[16] = { false };

	Delayah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam<_DelayTime>(_TIME_PARAM, 0.f, 1.f, 0.3f, "Time");
		configParam(_FEEDBACK_PARAM, 0.f, 0.98f, 0.3f, "Feedback", "%", 0.f, 100.f);
		configParam(_MIX_PARAM, 0.f, 1.f, 0.5f, "Mix", "%", 0.f, 100.f);
		configInput(_TIME_INPUT, "Time CV");
		configInput(_LEFT_INPUT, "Left");
		configInput(_RIGHT_INPUT, "Right");
		configOutput(_LEFT_OUTPUT, "Left");
		configOutput(_RIGHT_OUTPUT, "Right");
		configBypass(_LEFT_INPUT, _LEFT_OUTPUT);
		configBypass(_RIGHT_INPUT, _RIGHT_OUTPUT);
	}

	void onReset() override {
		for (int s = 0; s < 2; s++) {
			for (int c = 0; c < 16; c++) {
				std::fill(buffer[s][c].begin(), buffer[s][c].end(), 0.f);
				writeIndex[s][c] = 0;
			}
		}
		for (int c = 0; c < 16; c++)
			smoothInit[c] = false;
	}

	// Bypass freezes our feedback buffers instead of decaying them (process() just doesn't run),
	// so without this a loud moment stored right before bypass would blast back out on un-bypass
	void onBypass(const BypassEvent& e) override { onReset(); }
	void onUnBypass(const UnBypassEvent& e) override { onReset(); }

	float processTap(int side, int c, float in, float delaySamples, float feedback) {
		float readPos = writeIndex[side][c] - delaySamples;
		while (readPos < 0.f)
			readPos += bufferSize;
		// Float rounding in the wrap-around above can land readPos exactly on bufferSize
		// (a tiny sub-ULP deficit vanishes when added to a much larger number) - guard it
		if (readPos >= bufferSize)
			readPos -= bufferSize;
		int idx0 = (int)readPos;
		float frac = readPos - idx0;
		int idx1 = (idx0 + 1) % bufferSize;
		float wet = buffer[side][c][idx0] * (1.f - frac) + buffer[side][c][idx1] * frac;
		buffer[side][c][writeIndex[side][c]] = in + wet * feedback;
		writeIndex[side][c] = (writeIndex[side][c] + 1) % bufferSize;
		return wet;
	}

	void process(const ProcessArgs& args) override;
};

void Delayah::process(const ProcessArgs& args) {
	// Buffer sized for the worst-case delay time at the current sample rate
	int wantedSize = (int)std::ceil(args.sampleRate * MAX_DELAY_SEC) + 1;
	if (bufferSize != wantedSize) {
		bufferSize = wantedSize;
		for (int s = 0; s < 2; s++) {
			for (int c = 0; c < 16; c++) {
				buffer[s][c].assign(bufferSize, 0.f);
				writeIndex[s][c] = 0;
			}
		}
	}

	float feedback = params[_FEEDBACK_PARAM].getValue();
	float mix = params[_MIX_PARAM].getValue();
	float timeKnob = params[_TIME_PARAM].getValue();

	int channels = max(inputs[_LEFT_INPUT].getChannels(), max(inputs[_RIGHT_INPUT].getChannels(), 1));
	for (int c = 0; c < channels; c++) {
		float norm = timeKnob;
		if (inputs[_TIME_INPUT].isConnected())
			norm += inputs[_TIME_INPUT].getPolyVoltage(c) / 10.f;
		norm = clamp(norm, 0.f, 1.f);
		float timeSec = 0.001f + (MAX_DELAY_SEC - 0.001f) * norm * norm;
		float targetDelaySamples = clamp(timeSec * args.sampleRate, 1.f, (float)(bufferSize - 1));

		if (!smoothInit[c]) {
			smoothedDelaySamples[c] = targetDelaySamples;
			smoothInit[c] = true;
		}
		const float smoothTime = 0.005f;
		float smooth = 1.f - std::exp(-1.f / (smoothTime * args.sampleRate));
		smoothedDelaySamples[c] += (targetDelaySamples - smoothedDelaySamples[c]) * smooth;

		float inL = inputs[_LEFT_INPUT].getPolyVoltage(c);
		// Right normals to left, so patching only Left still behaves like a plain mono delay
		float inR = inputs[_RIGHT_INPUT].isConnected() ? inputs[_RIGHT_INPUT].getPolyVoltage(c) : inL;

		float wetL = processTap(0, c, inL, smoothedDelaySamples[c], feedback);
		float wetR = processTap(1, c, inR, smoothedDelaySamples[c], feedback);

		outputs[_LEFT_OUTPUT].setVoltage(crossfade(inL, wetL, mix), c);
		outputs[_RIGHT_OUTPUT].setVoltage(crossfade(inR, wetR, mix), c);
	}
	outputs[_LEFT_OUTPUT].setChannels(channels);
	outputs[_RIGHT_OUTPUT].setChannels(channels);
}

struct DelayahWidget : ModuleWidget {
	DelayahWidget(Delayah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Delayah.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 35.0)), module, Delayah::_TIME_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 57.0)), module, Delayah::_FEEDBACK_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 68.0)), module, Delayah::_MIX_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 46.0)), module, Delayah::_TIME_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, Delayah::_LEFT_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, Delayah::_RIGHT_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Delayah::_LEFT_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, Delayah::_RIGHT_OUTPUT));
	}
};

Model* modelDelayah = createModel<Delayah, DelayahWidget>("Delayah");
