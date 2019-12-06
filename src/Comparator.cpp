// Simple voltage comparator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct Comparator : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		_A_INPUT,
		_B_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_LT_OUTPUT,
		_EQ_OUTPUT,
		_NE_OUTPUT,
		_GT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Comparator() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override;
};

void Comparator::process(const ProcessArgs& args) {
	// Get inputs
	float a = inputs[_A_INPUT].getVoltage();
	float b = inputs[_B_INPUT].getVoltage();
	// Calculate results
	int lt = a < b;
	int eq = a == b;
	int ne = a != b;
	int gt = a > b;
	// Set outputs
	outputs[_LT_OUTPUT].setVoltage(lt * 10);
	outputs[_EQ_OUTPUT].setVoltage(eq * 10);
	outputs[_NE_OUTPUT].setVoltage(ne * 10);
	outputs[_GT_OUTPUT].setVoltage(gt * 10);
}

struct ComparatorWidget : ModuleWidget {
	ComparatorWidget(Comparator* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comparator.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 57.0)), module, Comparator::_A_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, Comparator::_B_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, Comparator::_LT_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, Comparator::_EQ_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Comparator::_NE_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, Comparator::_GT_OUTPUT));
	}
};

Model* modelComparator = createModel<Comparator, ComparatorWidget>("Comparator");