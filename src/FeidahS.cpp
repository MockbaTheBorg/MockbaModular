// Simple stereo voltage fader by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct FeidahS : Module {
	enum ParamIds {
		_KNOB_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_VCA_INPUT,
		_LEFT_INPUT,
		_RIGHT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_LEFT_OUTPUT,
		_RIGHT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	bool exponential = false;

	FeidahS() {
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

void FeidahS::process(const ProcessArgs& args) {
	float atten = params[_KNOB_PARAM].getValue();
	if (exponential)
		atten = logPot(atten);
	float outL;
	float outR;
	// Iterate over each channel
	int channels = max(inputs[_LEFT_INPUT].getChannels() , max(inputs[_LEFT_INPUT].getChannels(), 1));
	for (int c = 0; c < channels; ++c) {
		if (inputs[_VCA_INPUT].isConnected()) {
			outL = inputs[_LEFT_INPUT].getPolyVoltage(c) * atten * (inputs[_VCA_INPUT].getPolyVoltage(c) / 10);
			if (inputs[_RIGHT_INPUT].isConnected()) {
				outR = inputs[_RIGHT_INPUT].getPolyVoltage(c) * atten * (inputs[_VCA_INPUT].getPolyVoltage(c) / 10);
			} else {
				outR = outL;
			}
		} else {
			outL = inputs[_LEFT_INPUT].getPolyVoltage(c) * atten;
			if (inputs[_RIGHT_INPUT].isConnected()) {
				outR = inputs[_RIGHT_INPUT].getPolyVoltage(c) * atten;
			} else {
				outR = outL;
			}
		}
		outputs[_LEFT_OUTPUT].setVoltage(outL, c);
		outputs[_RIGHT_OUTPUT].setVoltage(outR, c);
	}
	outputs[_LEFT_OUTPUT].setChannels(channels);
	outputs[_RIGHT_OUTPUT].setChannels(channels);
}

struct FeidahSWidget : ModuleWidget {
	FeidahSWidget(FeidahS* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/FeidahS.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 57.0)), module, FeidahS::_KNOB_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, FeidahS::_VCA_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, FeidahS::_LEFT_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, FeidahS::_RIGHT_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, FeidahS::_LEFT_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, FeidahS::_RIGHT_OUTPUT));
	}

	struct Exponential : MenuItem {
		FeidahS* module;
		void onAction(const event::Action& e) override {
			module->exponential ^= 0x1;
		}
	};

	void appendContextMenu(Menu* menu) override {
		FeidahS* module = dynamic_cast<FeidahS*>(this->module);
		assert(module);

		menu->addChild(new MenuLabel());

		Exponential* htItem = createMenuItem<Exponential>("Exponential", CHECKMARK(module->exponential != 0));
		htItem->module = module;
		menu->addChild(htItem);
	}
};

Model* modelFeidahS = createModel<FeidahS, FeidahSWidget>("FeidahS");