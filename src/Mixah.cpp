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

	void process(const ProcessArgs& args) override {
		float mix = params[_KNOB_PARAM].getValue();
		float outa = inputs[_A_INPUT].getVoltage() * (1 - mix);
		float outb = inputs[_B_INPUT].getVoltage() * mix;
		float out = outa + outb;
		out = clamp(out, -10.0f, 10.0f);
		outputs[_MIX_OUTPUT].setVoltage(out);
	}
};

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
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, Mixah::_A_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, Mixah::_B_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Mixah::_MIX_OUTPUT));
	}
};

Model* modelMixah = createModel<Mixah, MixahWidget>("Mixah");