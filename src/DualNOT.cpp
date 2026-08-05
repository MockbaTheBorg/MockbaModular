// Simple NOT gates by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct _NotMode : ParamQuantity {
	std::string getDisplayValueString() override {
		int v = getValue();
		std::string result;
		switch (v) {
		case 0:
			result = "Digital";
			break;
		case 1:
			result = "Analog";
			break;
		default:
			result = "???";
		}
		return result;
	}
};

struct DualNOT : Module {
	enum ParamIds {
		_DA_PARAM,
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
		configParam<_NotMode>(_DA_PARAM, 0, 1, 0, "Mode");
		configInput(_A1_INPUT, "A1");
		configInput(_A2_INPUT, "A2");
		configInput(_A3_INPUT, "A3");
		configOutput(_Q1_OUTPUT, "Q1");
		configOutput(_Q2_OUTPUT, "Q2");
		configOutput(_Q3_OUTPUT, "Q3");
	}

	void process(const ProcessArgs& args) override;
};

void DualNOT::process(const ProcessArgs& args) {
    if (params[_DA_PARAM].getValue()) {
        // Analog mode: Invert analog inputs
        outputs[_Q1_OUTPUT].setVoltage(-inputs[_A1_INPUT].getVoltage());
        outputs[_Q2_OUTPUT].setVoltage(-inputs[_A2_INPUT].getVoltage());
        outputs[_Q3_OUTPUT].setVoltage(-inputs[_A3_INPUT].getVoltage());
    } else {
        // Digital mode: Get inputs, treating disconnected inputs as false (0V)
        bool a1 = inputs[_A1_INPUT].isConnected() ? 
            (inputs[_A1_INPUT].getVoltage() > GATE_THRESHOLD) : false;
        bool a2 = inputs[_A2_INPUT].isConnected() ? 
            (inputs[_A2_INPUT].getVoltage() > GATE_THRESHOLD) : false;
        bool a3 = inputs[_A3_INPUT].isConnected() ? 
            (inputs[_A3_INPUT].getVoltage() > GATE_THRESHOLD) : false;
        
        // Calculate NOT results (invert the boolean values)
        bool q1 = !a1;
        bool q2 = !a2;
        bool q3 = !a3;
        
        // Set outputs using proper boolean to voltage conversion
        outputs[_Q1_OUTPUT].setVoltage(q1 ? GATE_VOLTAGE : 0.0f);
        outputs[_Q2_OUTPUT].setVoltage(q2 ? GATE_VOLTAGE : 0.0f);
        outputs[_Q3_OUTPUT].setVoltage(q3 ? GATE_VOLTAGE : 0.0f);
    }
}

struct DualNOTWidget : ModuleWidget {
	DualNOTWidget(DualNOT* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DualNOT.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Hsw>(mm2px(Vec(5.1, 46.0)), module, DualNOT::_DA_PARAM));

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