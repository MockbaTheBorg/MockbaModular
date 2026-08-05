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
		configInput(_LOW_INPUT, "Low");
		configInput(_RAISE_INPUT, "Raise");
		configInput(_HIGH_INPUT, "High");
		configOutput(_Q_OUTPUT, "Q");
	}

	void process(const ProcessArgs& args) override;
};

void PSelectah::process(const ProcessArgs& args) {
    // Get inputs, treating disconnected inputs as false (0V)
    bool low = inputs[_LOW_INPUT].isConnected() ? 
        (inputs[_LOW_INPUT].getVoltage() > GATE_THRESHOLD) : false;
    bool raise = inputs[_RAISE_INPUT].isConnected() ? 
        (inputs[_RAISE_INPUT].getVoltage() > GATE_THRESHOLD) : false;
    bool high = inputs[_HIGH_INPUT].isConnected() ? 
        (inputs[_HIGH_INPUT].getVoltage() > GATE_THRESHOLD) : false;
    
    // Calculate priority selector result: HIGH has priority, otherwise LOW && RAISE
    bool q = high || (low && raise);
    
    // Set outputs using proper boolean to voltage conversion
    outputs[_Q_OUTPUT].setVoltage(q ? GATE_VOLTAGE : 0.0f);
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