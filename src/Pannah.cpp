// Simple stereo panner by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct Pannah : Module {
	enum ParamIds {
		_KNOB_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_MOD_INPUT,
		_VOLTAGE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_VOLTAGEL_OUTPUT,
		_VOLTAGER_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	bool constant = false;

	Pannah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_KNOB_PARAM, 0.f, 1.f, 0.5f, "pan");
	}

	void process(const ProcessArgs& args) override;

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		// Constant Power
		json_object_set_new(rootJ, "constantPower", json_integer(constant));
		return rootJ;
	}


	void dataFromJson(json_t* rootJ) override {
		// Constant Power
		json_t* constantPowerJ = json_object_get(rootJ, "constantPower");
		if (constantPowerJ)
			constant = json_integer_value(constantPowerJ);
	}
};

void Pannah::process(const ProcessArgs& args) {
	float pan = params[_KNOB_PARAM].getValue();
	bool polyPan = false;
	if (inputs[_MOD_INPUT].isConnected()) {
		if (inputs[_MOD_INPUT].getChannels() > 1) {
			polyPan = true;
		} else {
			pan = clamp(inputs[_MOD_INPUT].getVoltage(), -5.f, 5.f) / 10.f + .5f;
		}
	}
	float in, position, angle;
	float panL, panR;
	// Iterate over each channel
	int channels = max(inputs[_VOLTAGE_INPUT].getChannels(), 1);
	for (int c = 0; c < channels; ++c) {
		if(polyPan)
			pan = clamp(inputs[_MOD_INPUT].getVoltage(c), -5.f, 5.f) / 10.f + .5f;
		position = pan + pan - 1;
		angle = ((position + 1) / 2) * M_PI_2;
		if (constant) {
			panL = mmCos(angle);
			panR = mmSin(angle);
		} else {
			panL = 1.f - pan;
			panR = pan;
		}
		in = inputs[_VOLTAGE_INPUT].getVoltage(c);
		outputs[_VOLTAGEL_OUTPUT].setVoltage(in * panL, c);
		outputs[_VOLTAGER_OUTPUT].setVoltage(in * panR, c);
	}
	outputs[_VOLTAGEL_OUTPUT].setChannels(channels);
	outputs[_VOLTAGER_OUTPUT].setChannels(channels);
}

struct PannahWidget : ModuleWidget {
	PannahWidget(Pannah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Pannah.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 57.0)), module, Pannah::_KNOB_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, Pannah::_MOD_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, Pannah::_VOLTAGE_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, Pannah::_VOLTAGEL_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Pannah::_VOLTAGER_OUTPUT));
	}

	struct ConstantPower : MenuItem {
		Pannah* module;
		void onAction(const event::Action& e) override {
			module->constant ^= 0x1;
		}
	};

	void appendContextMenu(Menu* menu) override {
		Pannah* module = dynamic_cast<Pannah*>(this->module);
		assert(module);

		menu->addChild(new MenuLabel());

		ConstantPower* htItem = createMenuItem<ConstantPower>("Constant Power", CHECKMARK(module->constant != 0));
		htItem->module = module;
		menu->addChild(htItem);
	}
};

Model* modelPannah = createModel<Pannah, PannahWidget>("Pannah");