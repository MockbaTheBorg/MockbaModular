// Simple 3 channel voltage mixer by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct Mixah3 : Module {
	enum ParamIds {
		_A_PARAM,
		_B_PARAM,
		_C_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_A_INPUT,
		_B_INPUT,
		_C_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_MIX_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Mixah3() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_A_PARAM, 0.f, 1.f, 0.5f, "");
		configParam(_B_PARAM, 0.f, 1.f, 0.5f, "");
		configParam(_C_PARAM, 0.f, 1.f, 0.5f, "");
	}

	void process(const ProcessArgs& args) override;
};

void Mixah3::process(const ProcessArgs& args) {
	float mix[16] = {};
	int maxChannels = 1;

	float sum = 0.f;
	for (int i = 0; i < 3; i++) {
		int channels = 1;
		float in[16] = {};
		if (inputs[i].isConnected()) {
			channels = inputs[i].getChannels();
			maxChannels = max(maxChannels, channels);
			inputs[i].readVoltages(in);
			float gain = std::pow(params[i].getValue(), 2.f);
			sum += gain;
			for (int c = 0; c < channels; c++) {
				in[c] *= gain;
			}
			for (int c = 0; c < channels; c++) {
				mix[c] += in[c];
			}
		}
	}
	for (int c = 0; c < 16; c++)
		mix[c] /= max(1, sum);

	// Mix output
	if (outputs[_MIX_OUTPUT].isConnected()) {
		// Set mix output
		outputs[_MIX_OUTPUT].setChannels(maxChannels);
		outputs[_MIX_OUTPUT].writeVoltages(mix);
	}
}

struct Mixah3Widget : ModuleWidget {
	Mixah3Widget(Mixah3* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Mixah3.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 46.0)), module, Mixah3::_A_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 68.0)), module, Mixah3::_B_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 90.0)), module, Mixah3::_C_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 57.0)), module, Mixah3::_A_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, Mixah3::_B_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Mixah3::_C_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, Mixah3::_MIX_OUTPUT));
	}
};

Model* modelMixah3 = createModel<Mixah3, Mixah3Widget>("Mixah3");