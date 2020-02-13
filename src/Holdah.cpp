// Simple sample & hold by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct Holdah : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		_TRIGGER1_INPUT,
		_VOLTAGE1_INPUT,
		_TRIGGER2_INPUT,
		_VOLTAGE2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_VOLTAGE1_OUTPUT,
		_VOLTAGE2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Holdah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	float last1 = 0.f;
	float last2 = 0.f;
	bool trg1_was0 = true;
	bool trg2_was0 = true;

	void process(const ProcessArgs& args) override;
};

void Holdah::process(const ProcessArgs& args) {
	// Process Trigger1 signal
	if (inputs[_TRIGGER1_INPUT].getVoltage() > 0.0) {
		if (trg1_was0) {
			trg1_was0 = false;
			if (inputs[_VOLTAGE1_INPUT].isConnected()) {
				last1 = inputs[_VOLTAGE1_INPUT].getVoltage();
			} else {
				last1 = random::normal() * 2.5f;
			}
		}
	} else {
		trg1_was0 = true;
	}
	// Process Trigger2 signal
	if (inputs[_TRIGGER2_INPUT].getVoltage() > 0.0) {
		if (trg2_was0) {
			trg2_was0 = false;
			if (inputs[_VOLTAGE2_INPUT].isConnected()) {
				last2 = inputs[_VOLTAGE2_INPUT].getVoltage();
			} else {
				last2 = random::normal() * 2.5f;
			}
		}
	} else {
		trg2_was0 = true;
	}
	outputs[_VOLTAGE1_OUTPUT].setVoltage(last1);
	outputs[_VOLTAGE2_OUTPUT].setVoltage(last2);
}

struct HoldahWidget : ModuleWidget {
	HoldahWidget(Holdah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Holdah.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 57.0)), module, Holdah::_TRIGGER1_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, Holdah::_VOLTAGE1_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, Holdah::_TRIGGER2_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Holdah::_VOLTAGE2_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, Holdah::_VOLTAGE1_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, Holdah::_VOLTAGE2_OUTPUT));
	}
};

Model* modelHoldah = createModel<Holdah, HoldahWidget>("Holdah");