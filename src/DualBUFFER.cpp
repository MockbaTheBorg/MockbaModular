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
		configInput(_A1_INPUT, "A1");
		configInput(_A2_INPUT, "A2");
		configInput(_A3_INPUT, "A3");
		configOutput(_Q1_OUTPUT, "Q1");
		configOutput(_Q2_OUTPUT, "Q2");
		configOutput(_Q3_OUTPUT, "Q3");
	}

	void process(const ProcessArgs& args) override;
};

void DualBUFFER::process(const ProcessArgs& args) {
    // Treat disconnected inputs as false (0V)
    bool q1 = inputs[_A1_INPUT].isConnected() ? 
        (inputs[_A1_INPUT].getVoltage() > GATE_THRESHOLD) : false;
    bool q2 = inputs[_A2_INPUT].isConnected() ? 
        (inputs[_A2_INPUT].getVoltage() > GATE_THRESHOLD) : false;
    bool q3 = inputs[_A3_INPUT].isConnected() ? 
        (inputs[_A3_INPUT].getVoltage() > GATE_THRESHOLD) : false;
    
    // Set outputs
    outputs[_Q1_OUTPUT].setVoltage(q1 ? GATE_VOLTAGE : 0.0f);
    outputs[_Q2_OUTPUT].setVoltage(q2 ? GATE_VOLTAGE : 0.0f);
    outputs[_Q3_OUTPUT].setVoltage(q3 ? GATE_VOLTAGE : 0.0f);
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