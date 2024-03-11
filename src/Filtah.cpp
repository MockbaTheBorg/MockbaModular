// A Moog style filter by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct Filtah : Module {
	enum ParamIds {
		_SL1_PARAM,		// Type: LP, HP, BP
		_SL2_PARAM,		// Cutoff
		_SL3_PARAM,		// Resonance
		_SL5_PARAM,		// Output
		NUM_PARAMS
	};
	enum InputIds {
		_MODC_INPUT,
		_MODR_INPUT,
		_SP0_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_SP0_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	_Filter <float_4>fil[4];

	Filtah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam<_FilterMode>(_SL1_PARAM, 0.f, 2.f, 0.f, "");
		configParam(_SL2_PARAM, 0.f, 100.f, 100.f, "%");
		configParam(_SL3_PARAM, 0.f, 0.85f, 0.f, "");
		configParam(_SL5_PARAM, -25.f, 25.f, 0.f, "dB");
	}

	void process(const ProcessArgs& args) override;
};

void Filtah::process(const ProcessArgs& args) {
	// Get parameters
	int ftype = params[_SL1_PARAM].getValue();
	float sx = 16.f + params[_SL2_PARAM].getValue() * 1.20103f;
	float res = params[_SL3_PARAM].getValue();
	float outgain = powf(10.f, params[_SL5_PARAM].getValue() / 20.f);
	// Iterate over each channel
	int channels = max(inputs[_SP0_INPUT].getChannels(), 1);
	for (int c = 0; c < channels; c += 4) {
		int nc = c / 4;
		// Get the filter
		auto* filter = &fil[nc];

		float_4 sxV = sx;
		if (inputs[_MODC_INPUT].isConnected())
			sxV *= inputs[_MODC_INPUT].getVoltageSimd<float_4>(c) / 10.f;
		float_4 cutoff = simd::floor(simd::exp(sxV * simd::log(1.059f)) * 8.17742f);
		float_4 resV = res;
		if (inputs[_MODR_INPUT].isConnected())
			resV *= inputs[_MODR_INPUT].getVoltageSimd<float_4>(c) / 10.f;

		filter->setType(ftype);
		filter->setCutoff(cutoff);
		filter->setRes(resV);
		filter->setGain(outgain);

		filter->process(inputs[_SP0_INPUT].getVoltageSimd<float_4>(c), args.sampleRate);
		outputs[_SP0_OUTPUT].setVoltageSimd(filter->_Out(), c);
	}
	outputs[_SP0_OUTPUT].setChannels(channels);
}

struct FiltahWidget : ModuleWidget {
	FiltahWidget(Filtah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Filtah.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Hsw3>(mm2px(Vec(5.1, 35.0)), module, Filtah::_SL1_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 46.0)), module, Filtah::_SL2_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 68.0)), module, Filtah::_SL3_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 90.0)), module, Filtah::_SL5_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 57.0)), module, Filtah::_MODC_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, Filtah::_MODR_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Filtah::_SP0_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, Filtah::_SP0_OUTPUT));
	}
};

Model* modelFiltah = createModel<Filtah, FiltahWidget>("Filtah");