// Simple voltage mixer by Mockba the Borg

#include "plugin.hpp"

struct Mixah : Module {
	enum ParamIds {
		_KNOB_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_A_INPUT,
		_B_INPUT,
		_VCA_INPUT,
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
	float mix = params[_KNOB_PARAM].getValue();
	float out;
	// Iterate over each channel
	int channels = max(max(inputs[_A_INPUT].getChannels(), inputs[_B_INPUT].getChannels()), 1);
	for (int c = 0; c < channels; c++) {
		float outa = inputs[_A_INPUT].getVoltage(c) * (1 - mix);
		float outb = inputs[_B_INPUT].getVoltage(c) * mix;
		if (inputs[_VCA_INPUT].isConnected()) {
			out = (outa + outb) * (inputs[_VCA_INPUT].getVoltage(c) / 10);
		} else {
			out = outa + outb;
		}
		out = clamp(out, -10.0f, 10.0f);
		outputs[_MIX_OUTPUT].setVoltage(out, c);
	}
	outputs[_MIX_OUTPUT].setChannels(channels);
}

struct MixahWidget : ModuleWidget {
	MixahWidget(Mixah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Mixah.svg")));

		// Screws
		addChild(createWidget<_Screw>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 57.0)), module, Mixah::_KNOB_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, Mixah::_VCA_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, Mixah::_A_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Mixah::_B_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, Mixah::_MIX_OUTPUT));
	}
};

Model* modelMixah = createModel<Mixah, MixahWidget>("Mixah");