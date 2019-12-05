// Simple voltage fader by Mockba the Borg

#include "plugin.hpp"

struct Feidah : Module {
	enum ParamIds {
		_KNOB_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_VOLTAGE_INPUT,
		_VCA_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_VOLTAGE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Feidah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_KNOB_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override;
};

void Feidah::process(const ProcessArgs& args) {
	float atten = params[_KNOB_PARAM].getValue();
	float out;
	// Iterate over each channel
	int channels = max(inputs[_VOLTAGE_INPUT].getChannels(), 1);
	for (int c = 0; c < channels; ++c) {
		if (inputs[_VOLTAGE_INPUT].isConnected()) {
			if (inputs[_VCA_INPUT].isConnected()) {
				out = inputs[_VOLTAGE_INPUT].getVoltage(c) * atten * (inputs[_VCA_INPUT].getVoltage(c) / 10);
			} else {
				out = inputs[_VOLTAGE_INPUT].getVoltage(c) * atten;
			}
		} else {
			out = params[_KNOB_PARAM].getValue() * 10;
		}
		outputs[_VOLTAGE_OUTPUT].setVoltage(out, c);
	}
	outputs[_VOLTAGE_OUTPUT].setChannels(channels);
}

struct FeidahWidget : ModuleWidget {
	FeidahWidget(Feidah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Feidah.svg")));

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 57.0)), module, Feidah::_KNOB_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, Feidah::_VCA_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, Feidah::_VOLTAGE_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Feidah::_VOLTAGE_OUTPUT));
	}
};

Model* modelFeidah = createModel<Feidah, FeidahWidget>("Feidah");