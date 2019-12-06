// Simple XOR gates by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct DualXOR : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		_A1_INPUT,
		_B1_INPUT,
		_A2_INPUT,
		_B2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_Q1_OUTPUT,
		_Q2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	DualXOR() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override;
};

void DualXOR::process(const ProcessArgs& args) {
	// Get inputs
	int a1 = inputs[_A1_INPUT].getVoltage() > 0;
	int b1 = inputs[_B1_INPUT].getVoltage() > 0;
	int a2 = inputs[_A2_INPUT].getVoltage() > 0;
	int b2 = inputs[_B2_INPUT].getVoltage() > 0;
	// Calculate results
	int q1 = a1 != b1;
	int q2 = a2 != b2;
	// Set outputs
	outputs[_Q1_OUTPUT].setVoltage(q1 * 10);
	outputs[_Q2_OUTPUT].setVoltage(q2 * 10);
}

struct DualXORWidget : ModuleWidget {
	DualXORWidget(DualXOR* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DualXOR.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 57.0)), module, DualXOR::_A1_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, DualXOR::_B1_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, DualXOR::_A2_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, DualXOR::_B2_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, DualXOR::_Q1_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, DualXOR::_Q2_OUTPUT));
	}
};

Model* modelDualXOR = createModel<DualXOR, DualXORWidget>("DualXOR");