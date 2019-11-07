// Simple 4 to 1 selector by Mockba the Borg

#include "plugin.hpp"

struct Selectah : Module {
	enum ParamIds {
		_SELECT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_IN_A_INPUT,
		_IN_B_INPUT,
		_IN_C_INPUT,
		_IN_D_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_SELECTED_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Selectah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_SELECT_PARAM, 0.f, 3.f, 0.f, "");
	}

	void onAdd() override;

	void onReset() override;

	void process(const ProcessArgs& args) override;
};

void Selectah::onAdd() {
	params[_SELECT_PARAM].setValue(0);
}

void Selectah::onReset() {
	onAdd();
}

void Selectah::process(const ProcessArgs& args) {
	int selection = int(params[_SELECT_PARAM].getValue());
	switch (selection) {
	case 0:
		outputs[_SELECTED_OUTPUT].setVoltage(inputs[_IN_A_INPUT].getVoltage());
		break;
	case 1:
		outputs[_SELECTED_OUTPUT].setVoltage(inputs[_IN_B_INPUT].getVoltage());
		break;
	case 2:
		outputs[_SELECTED_OUTPUT].setVoltage(inputs[_IN_C_INPUT].getVoltage());
		break;
	default:
		outputs[_SELECTED_OUTPUT].setVoltage(inputs[_IN_D_INPUT].getVoltage());
	}
}

struct SelectahWidget : ModuleWidget {
	SelectahWidget(Selectah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Selectah.svg")));

		// Screws
		addChild(createWidget<_Screw>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Selector>(mm2px(Vec(5.1, 46.0)), module, Selectah::_SELECT_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, Selectah::_IN_A_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, Selectah::_IN_B_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, Selectah::_IN_C_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Selectah::_IN_D_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, Selectah::_SELECTED_OUTPUT));
	}
};


Model* modelSelectah = createModel<Selectah, SelectahWidget>("Selectah");