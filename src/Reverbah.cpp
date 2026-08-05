// Simple stereo reverb by Mockba the Borg
// Freeverb-style: 8 parallel combs + 4 series allpasses per side, public-domain algorithm by Jezar at Dreampoint

#include "plugin.hpp"
#include "MockbaModular.hpp"

static const int NUM_COMBS = 8;
static const int NUM_ALLPASS = 4;
static const int STEREO_SPREAD = 23;
static const int COMB_TUNING[NUM_COMBS] = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
static const int ALLPASS_TUNING[NUM_ALLPASS] = { 556, 441, 341, 225 };
static const float FIXED_GAIN = 0.015f;
static const float WET_MAKEUP = 5.f;  // compensates FIXED_GAIN so wet is comparable to dry at mid Size
static const float ALLPASS_FEEDBACK = 0.5f;

struct _CombFilter {
	std::vector<float> buffer;
	int index = 0;
	float filterStore = 0.f;

	void init(int size) {
		buffer.assign(size, 0.f);
		index = 0;
		filterStore = 0.f;
	}
	float process(float input, float feedback, float damp1, float damp2) {
		float output = buffer[index];
		filterStore = output * damp2 + filterStore * damp1;
		buffer[index] = input + filterStore * feedback;
		index = (index + 1) % (int)buffer.size();
		return output;
	}
};

struct _AllpassFilter {
	std::vector<float> buffer;
	int index = 0;

	void init(int size) {
		buffer.assign(size, 0.f);
		index = 0;
	}
	float process(float input, float feedback) {
		float bufout = buffer[index];
		float output = bufout - input;
		buffer[index] = input + bufout * feedback;
		index = (index + 1) % (int)buffer.size();
		return output;
	}
};

struct Reverbah : Module {
	enum ParamIds {
		_SIZE_PARAM,
		_DAMP_PARAM,
		_MIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
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

	_CombFilter combL[NUM_COMBS];
	_CombFilter combR[NUM_COMBS];
	_AllpassFilter allpassL[NUM_ALLPASS];
	_AllpassFilter allpassR[NUM_ALLPASS];
	float lastSampleRate = -1.f;

	Reverbah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_SIZE_PARAM, 0.f, 1.f, 0.5f, "Size", "%", 0.f, 100.f);
		configParam(_DAMP_PARAM, 0.f, 1.f, 0.5f, "Damp", "%", 0.f, 100.f);
		configParam(_MIX_PARAM, 0.f, 1.f, 0.5f, "Mix", "%", 0.f, 100.f);
		configInput(_LEFT_INPUT, "Left");
		configInput(_RIGHT_INPUT, "Right");
		configOutput(_LEFT_OUTPUT, "Left");
		configOutput(_RIGHT_OUTPUT, "Right");
		configBypass(_LEFT_INPUT, _LEFT_OUTPUT);
		configBypass(_RIGHT_INPUT, _RIGHT_OUTPUT);
	}

	void onReset() override {
		if (lastSampleRate > 0.f)
			setup(lastSampleRate);
	}

	// Bypass freezes the comb/allpass buffers instead of decaying them (process() just doesn't
	// run), so without this any resonance stored right before bypass would blast back out on un-bypass
	void onBypass(const BypassEvent& e) override { onReset(); }
	void onUnBypass(const UnBypassEvent& e) override { onReset(); }

	void setup(float sampleRate) {
		lastSampleRate = sampleRate;
		float scale = sampleRate / 44100.f;
		for (int i = 0; i < NUM_COMBS; i++) {
			combL[i].init((int)(COMB_TUNING[i] * scale));
			combR[i].init((int)((COMB_TUNING[i] + STEREO_SPREAD) * scale));
		}
		for (int i = 0; i < NUM_ALLPASS; i++) {
			allpassL[i].init((int)(ALLPASS_TUNING[i] * scale));
			allpassR[i].init((int)((ALLPASS_TUNING[i] + STEREO_SPREAD) * scale));
		}
	}

	void process(const ProcessArgs& args) override;
};

void Reverbah::process(const ProcessArgs& args) {
	if (args.sampleRate != lastSampleRate)
		setup(args.sampleRate);

	float feedback = params[_SIZE_PARAM].getValue() * 0.28f + 0.7f;
	float damp1 = params[_DAMP_PARAM].getValue() * 0.4f;
	float damp2 = 1.f - damp1;
	float mix = params[_MIX_PARAM].getValue();

	// Mono-only patches stay full level; stereo patches average so loudness matches either side alone
	float input = inputs[_LEFT_INPUT].getVoltageSum();
	if (inputs[_RIGHT_INPUT].isConnected())
		input = (input + inputs[_RIGHT_INPUT].getVoltageSum()) * 0.5f;
	float combIn = input * FIXED_GAIN;

	float outL = 0.f, outR = 0.f;
	for (int i = 0; i < NUM_COMBS; i++) {
		outL += combL[i].process(combIn, feedback, damp1, damp2);
		outR += combR[i].process(combIn, feedback, damp1, damp2);
	}
	for (int i = 0; i < NUM_ALLPASS; i++) {
		outL = allpassL[i].process(outL, ALLPASS_FEEDBACK);
		outR = allpassR[i].process(outR, ALLPASS_FEEDBACK);
	}

	outputs[_LEFT_OUTPUT].setVoltage(crossfade(input, outL * WET_MAKEUP, mix));
	outputs[_RIGHT_OUTPUT].setVoltage(crossfade(input, outR * WET_MAKEUP, mix));
}

struct ReverbahWidget : ModuleWidget {
	ReverbahWidget(Reverbah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Reverbah.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 46.0)), module, Reverbah::_SIZE_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 57.0)), module, Reverbah::_DAMP_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 68.0)), module, Reverbah::_MIX_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, Reverbah::_LEFT_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, Reverbah::_RIGHT_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Reverbah::_LEFT_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, Reverbah::_RIGHT_OUTPUT));
	}
};

Model* modelReverbah = createModel<Reverbah, ReverbahWidget>("Reverbah");
