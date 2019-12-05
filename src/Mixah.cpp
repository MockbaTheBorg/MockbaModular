// Simple voltage mixer by Mockba the Borg

#include "plugin.hpp"

struct Mixah : Module {
	enum ParamIds {
		_KNOB_PARAM,
		_PHASE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_MOD_INPUT,
		_VCA_INPUT,
		_A_INPUT,
		_B_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_MIX_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Mixah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_KNOB_PARAM, 0.f, 1.f, 0.5f, "");
	}

	void process(const ProcessArgs& args) override;
};

void Mixah::process(const ProcessArgs& args) {
	float mix;
	if (inputs[_MOD_INPUT].isConnected()) {
		mix = (5.0f + clamp(inputs[_MOD_INPUT].getVoltage(), -5.f, 5.0f)) / 10;
	} else {
		mix = params[_KNOB_PARAM].getValue();
	}
	mix = 1.0f - mix;
	float out;
	float inA, inB;
	// Iterate over each channel
	int channels = max(max(inputs[_A_INPUT].getChannels(), inputs[_B_INPUT].getChannels()), 1);
	for (int c = 0; c < channels; ++c) {
		if (inputs[_A_INPUT].isConnected()) {
			inA = inputs[_A_INPUT].getVoltage(c);
			inB = inputs[_B_INPUT].getVoltage(c);
			if (params[_PHASE_PARAM].getValue() == 1.0)
				inB = -inB;
			out = crossfade(inB, inA, mix);
			if (inputs[_VCA_INPUT].isConnected())
				out *= inputs[_VCA_INPUT].getVoltage(c) / 10;
		} else {
			out = params[_KNOB_PARAM].getValue() * 10 - 5;
		}
		outputs[_MIX_OUTPUT].setVoltage(out, c);
	}
	outputs[_MIX_OUTPUT].setChannels(channels);
}

struct MixahWidget : ModuleWidget {
	MixahWidget(Mixah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Mixah.svg")));

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 46.0)), module, Mixah::_KNOB_PARAM));
		addParam(createParamCentered<_Hsw>(mm2px(Vec(5.1, 79.0)), module, Mixah::_PHASE_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 57.0)), module, Mixah::_MOD_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, Mixah::_VCA_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, Mixah::_A_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Mixah::_B_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, Mixah::_MIX_OUTPUT));
	}
};

Model* modelMixah = createModel<Mixah, MixahWidget>("Mixah");