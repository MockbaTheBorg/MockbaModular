// Simple binary clock divider by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct Dividah : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		_CLOCK_INPUT,
		_RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_BY2_OUTPUT,
		_BY4_OUTPUT,
		_BY8_OUTPUT,
		_BY16_OUTPUT,
		_BY32_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Dividah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	int8_t value;
	bool clock_was0;
	bool reset_was0;

	void onAdd() override;

	void onReset() override;

	void process(const ProcessArgs& args) override;
};

void Dividah::onAdd() {
	clock_was0 = true;
	value = 0;
	for (int i = 0; i < NUM_OUTPUTS; i++) {
		outputs[i].setVoltage(0.0);
	}
}

void Dividah::onReset() {
	onAdd();
}

void Dividah::process(const ProcessArgs& args) {
	// Process RESET signal
	if (inputs[_RESET_INPUT].getVoltage() > 0.0) {
		if (reset_was0) {
			reset_was0 = false;
			onReset();
		}
	} else {
		reset_was0 = true;
	}

	// Process CLOCK signal
	if (inputs[_CLOCK_INPUT].getVoltage() > 0.0) {
		if (clock_was0) {
			clock_was0 = false;
			--value;

			outputs[_BY2_OUTPUT].setVoltage(value & 1 ? 10.0 : 0.0);
			outputs[_BY4_OUTPUT].setVoltage(value & 2 ? 10.0 : 0.0);
			outputs[_BY8_OUTPUT].setVoltage(value & 4 ? 10.0 : 0.0);
			outputs[_BY16_OUTPUT].setVoltage(value & 8 ? 10.0 : 0.0);
			outputs[_BY32_OUTPUT].setVoltage(value & 16 ? 10.0 : 0.0);
		}
	} else {
		clock_was0 = true;
	}

}

struct DividahWidget : ModuleWidget {
	DividahWidget(Dividah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Dividah.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 46.0)), module, Dividah::_CLOCK_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 57.0)), module, Dividah::_RESET_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, Dividah::_BY2_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, Dividah::_BY4_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, Dividah::_BY8_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Dividah::_BY16_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, Dividah::_BY32_OUTPUT));
	}
};

Model* modelDividah = createModel<Dividah, DividahWidget>("Dividah");