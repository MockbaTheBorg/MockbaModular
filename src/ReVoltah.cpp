// Simple voltage range adapter by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct ReVoltah : Module {
	enum ParamIds {
		_I_MIN,
		_I_MAX,
		_O_MIN,
		_O_MAX,
		NUM_PARAMS
	};
	enum InputIds {
		_V_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_V_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	ReVoltah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_I_MIN, -10.f, 10.f, -10.f, "I minimum");
		configParam(_I_MAX, -10.f, 10.f, 10.f, "I maximum");
		configParam(_O_MIN, -10.f, 10.f, -10.f, "O minimum");
		configParam(_O_MAX, -10.f, 10.f, 10.f, "O maximum");
	}

	void process(const ProcessArgs& args) override;
};

void ReVoltah::process(const ProcessArgs& args) {
	// Get params
	float i0 = params[_I_MIN].getValue();
	float i1 = params[_I_MAX].getValue();
	float o0 = params[_O_MIN].getValue();
	float o1 = params[_O_MAX].getValue();
	// Iterate over each channel
	int channels = max(inputs[_V_INPUT].getChannels(), 1);
	for (int c = 0; c < channels; ++c) {
		// Get inputs
		float v = inputs[_V_INPUT].getVoltage(c);
		// Calculate results
		float d0 = i1 - i0;
		if (d0 != 0.f)
			v = ((v - i0) / d0) * (o1 - o0) + o0;
		// Set outputs
		outputs[_V_OUTPUT].setVoltage(clamp(v, -12.f, 12.f), c);
	}
	outputs[_V_OUTPUT].setChannels(channels);
}

struct ReVoltahWidget : ModuleWidget {
	ReVoltahWidget(ReVoltah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ReVoltah.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 57.0)), module, ReVoltah::_I_MIN));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 68.0)), module, ReVoltah::_I_MAX));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 90.0)), module, ReVoltah::_O_MIN));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 101.0)), module, ReVoltah::_O_MAX));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, ReVoltah::_V_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, ReVoltah::_V_OUTPUT));
	}
};

Model* modelReVoltah = createModel<ReVoltah, ReVoltahWidget>("ReVoltah");