// Simple Priority Selector by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct PSelectah : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		_LOW_INPUT,
		_RAISE_INPUT,
		_HIGH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_Q_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	PSelectah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override;
};

void PSelectah::process(const ProcessArgs& args) {
	// Get inputs
	int low = inputs[_LOW_INPUT].getVoltage() > 0;
	int raise = inputs[_RAISE_INPUT].getVoltage() > 0;
	int high = inputs[_HIGH_INPUT].getVoltage() > 0;
	// Calculate results
	int q = (low && raise) || high;
	// Set outputs
	outputs[_Q_OUTPUT].setVoltage(q * 10);
}

struct PSelectahWidget : ModuleWidget {
	PSelectahWidget(PSelectah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PSelectah.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, PSelectah::_LOW_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, PSelectah::_RAISE_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, PSelectah::_HIGH_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, PSelectah::_Q_OUTPUT));
	}
};

Model* modelPSelectah = createModel<PSelectah, PSelectahWidget>("PSelectah");