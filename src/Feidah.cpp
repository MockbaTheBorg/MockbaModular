// Simple voltage fader by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct Feidah : Module {
	enum ParamIds {
		_KNOB_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_VOLTAGE_INPUT,
		_VCA_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_VOLTAGE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	bool exponential = false;

	Feidah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_KNOB_PARAM, 0.f, 1.f, 1.f, "");
	}

	void process(const ProcessArgs& args) override;

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		// Constant Power
		json_object_set_new(rootJ, "exponential", json_integer(exponential));
		return rootJ;
	}


	void dataFromJson(json_t* rootJ) override {
		// Constant Power
		json_t* exponentialJ = json_object_get(rootJ, "exponential");
		if (exponentialJ)
			exponential = json_integer_value(exponentialJ);
	}
};

void Feidah::process(const ProcessArgs& args) {
	float atten = params[_KNOB_PARAM].getValue();
	if (exponential)
		atten = logPot(atten);
	float out;
	// Iterate over each channel
	int channels = max(inputs[_VOLTAGE_INPUT].getChannels(), 1);
	for (int c = 0; c < channels; ++c) {
		if (inputs[_VOLTAGE_INPUT].isConnected()) {
			if (inputs[_VCA_INPUT].isConnected()) {
				out = inputs[_VOLTAGE_INPUT].getPolyVoltage(c) * atten * (inputs[_VCA_INPUT].getPolyVoltage(c) / 10);
			} else {
				out = inputs[_VOLTAGE_INPUT].getPolyVoltage(c) * atten;
			}
		} else {
			out = params[_KNOB_PARAM].getValue() * 10;
		}
		outputs[_VOLTAGE_OUTPUT].setVoltage(out, c);
	}
	outputs[_VOLTAGE_OUTPUT].setChannels(channels);
}

struct FeidahWidget : ModuleWidget {
	FeidahWidget(Feidah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Feidah.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 57.0)), module, Feidah::_KNOB_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, Feidah::_VCA_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, Feidah::_VOLTAGE_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Feidah::_VOLTAGE_OUTPUT));
	}

	struct Exponential : MenuItem {
		Feidah* module;
		void onAction(const event::Action& e) override {
			module->exponential ^= 0x1;
		}
	};

	void appendContextMenu(Menu* menu) override {
		Feidah* module = dynamic_cast<Feidah*>(this->module);
		assert(module);

		menu->addChild(new MenuLabel());

		Exponential* htItem = createMenuItem<Exponential>("Exponential", CHECKMARK(module->exponential != 0));
		htItem->module = module;
		menu->addChild(htItem);
	}
};

Model* modelFeidah = createModel<Feidah, FeidahWidget>("Feidah");