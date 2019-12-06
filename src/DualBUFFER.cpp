// Simple BUFFER gates by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct DualBUFFER : Module {
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

	DualBUFFER() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override;
};

void DualBUFFER::process(const ProcessArgs& args) {
	// Get inputs
	int a1 = inputs[_A1_INPUT].getVoltage() > 0;
	int a2 = inputs[_A2_INPUT].getVoltage() > 0;
	int a3 = inputs[_A3_INPUT].getVoltage() > 0;
	// Set outputs
	outputs[_Q1_OUTPUT].setVoltage(a1 * 10);
	outputs[_Q2_OUTPUT].setVoltage(a2 * 10);
	outputs[_Q3_OUTPUT].setVoltage(a3 * 10);
}

struct DualBUFFERWidget : ModuleWidget {
	DualBUFFERWidget(DualBUFFER* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DualBUFFER.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 57.0)), module, DualBUFFER::_A1_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, DualBUFFER::_A2_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, DualBUFFER::_A3_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, DualBUFFER::_Q1_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, DualBUFFER::_Q2_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, DualBUFFER::_Q3_OUTPUT));
	}
};

Model* modelDualBUFFER = createModel<DualBUFFER, DualBUFFERWidget>("DualBUFFER");