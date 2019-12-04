// Simple NOT gates by Mockba the Borg

#include "plugin.hpp"

struct DualNOT : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		_A1_INPUT,
		_A2_INPUT,
		_A3_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_Q1_OUTPUT,
		_Q2_OUTPUT,
		_Q3_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	DualNOT() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override;
};

void DualNOT::process(const ProcessArgs& args) {
	int a1 = inputs[_A1_INPUT].getVoltage() <= 0;
	int a2 = inputs[_A2_INPUT].getVoltage() <= 0;
	int a3 = inputs[_A3_INPUT].getVoltage() <= 0;
	outputs[_Q1_OUTPUT].setVoltage(a1 * 10);
	outputs[_Q2_OUTPUT].setVoltage(a2 * 10);
	outputs[_Q3_OUTPUT].setVoltage(a3 * 10);
}

struct DualNOTWidget : ModuleWidget {
	DualNOTWidget(DualNOT* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DualNOT.svg")));

		// Screws
		addChild(createWidget<_Screw>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 57.0)), module, DualNOT::_A1_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, DualNOT::_A2_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, DualNOT::_A3_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, DualNOT::_Q1_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, DualNOT::_Q2_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, DualNOT::_Q3_OUTPUT));
	}
};

Model* modelDualNOT = createModel<DualNOT, DualNOTWidget>("DualNOT");