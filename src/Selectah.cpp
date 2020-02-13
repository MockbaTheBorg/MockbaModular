// Simple 4 to 1 selector by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct _InputSelect : ParamQuantity {
	std::string getDisplayValueString() override {
		int v = getValue();
		std::string result;
		switch (v) {
		case 0:
			result = "A";
			break;
		case 1:
			result = "B";
			break;
		case 2:
			result = "C";
			break;
		case 3:
			result = "D";
			break;
		default:
			result = "???";
		}
		return result;
	}
};

struct Selectah : Module {
	enum ParamIds {
		_SELECT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_SEL_INPUT,
		_A_INPUT,
		_B_INPUT,
		_C_INPUT,
		_D_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_SELECTED_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Selectah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam<_InputSelect>(_SELECT_PARAM, 0.f, 3.f, 0.f, "Input");
	}

	void process(const ProcessArgs& args) override;
};

void Selectah::process(const ProcessArgs& args) {
	int selection;
	if (inputs[_SEL_INPUT].isConnected()) {
		selection = clamp(floor(inputs[_SEL_INPUT].getVoltage() / 2.5f), 0.f, 3.f);
	} else {
		selection = int(params[_SELECT_PARAM].getValue());
	}
	int channels = max(inputs[selection + 1].getChannels(), 1);
	// Iterate over each channel
	for (int c = 0; c < channels; ++c) {
		outputs[_SELECTED_OUTPUT].setVoltage(inputs[selection + 1].getVoltage(c), c);
	}
	outputs[_SELECTED_OUTPUT].setChannels(channels);
}

struct SelectahWidget : ModuleWidget {
	SelectahWidget(Selectah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Selectah.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Selector>(mm2px(Vec(5.1, 46.0)), module, Selectah::_SELECT_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 57.0)), module, Selectah::_SEL_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, Selectah::_A_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, Selectah::_B_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, Selectah::_C_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Selectah::_D_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, Selectah::_SELECTED_OUTPUT));
	}
};


Model* modelSelectah = createModel<Selectah, SelectahWidget>("Selectah");