#include "plugin.hpp"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

struct CZSaw : Module {
	enum ParamIds {
		_FREQ_PARAM,
		_SHAPE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_MODF_INPUT,
		_MODS_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_WAVE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	CZSaw() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_FREQ_PARAM, -3.0f, 3.0f, 0.0f);
		configParam(_SHAPE_PARAM, 0.0f, 10.0f, 0.0f);
	}

	void onAdd() override;

	void onReset() override;

	void process(const ProcessArgs& args) override;

	float phase = 0.0f;
};

void CZSaw::onAdd() {
}

void CZSaw::onReset() {
	onAdd();
}

void CZSaw::process(const ProcessArgs& args) {
	// Calculate desired frequency
	float freq = clamp(params[_FREQ_PARAM].getValue() + inputs[_MODF_INPUT].getVoltage(), -3.0f, 3.0f);
	freq = 261.626f * powf(2.0f, freq);
	// Calculate phase
	phase += freq * args.sampleTime;
	if (phase >= 1.0f)
		phase -= 1.0f;
	// Calculate the shape index
	float shape = max((params[_SHAPE_PARAM].getValue() + inputs[_MODS_INPUT].getVoltage()) * 0.05f, 0.001f);
	// Calculate the wave
	float a = phase * ((0.5f - shape) / shape);
	float b = (-1 * phase + 1) * ((0.5f - shape) / (1 - shape));
	float m = phase + min(a, b);
	float out = cos(m * (2 * M_PI));

	outputs[_WAVE_OUTPUT].setVoltage(out * 5);
}

struct CZSawWidget : ModuleWidget {
	CZSawWidget(CZSaw* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CZSaw.svg")));

		// Screws
		addChild(createWidget<_Screw>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 68.0)), module, CZSaw::_FREQ_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 90.0)), module, CZSaw::_SHAPE_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, CZSaw::_MODF_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, CZSaw::_MODS_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, CZSaw::_WAVE_OUTPUT));
	}
};

Model* modelCZSaw = createModel<CZSaw, CZSawWidget>("CZSaw");