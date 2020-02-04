// Casio CZ style oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct _CZOsc : _MMOsc {
	_CZWave<float_4> osc;

	float_4 oscStep(float_4 phase, float_4 shape, int wave) override {
		// Calculate the wave step
		float_4 v;
		switch (wave) {
		case 0:		// Saw
			v = osc.Saw(phase, shape);
			break;
		case 1:		// Square
			v = osc.Square(phase, shape);
			break;
		case 2:		// Pulse
			v = osc.Pulse(phase, shape);
			break;
		case 3:		// DblSine
			v = osc.DblSine(phase, shape);
			break;
		case 4:		// SawPulse
			v = osc.SawPulse(phase, shape);
			break;
		case 5:		// Reso1
			v = osc.Reso1(phase, shape);
			break;
		case 6:		// Reso2
			v = osc.Reso2(phase, shape);
			break;
		case 7:		// Reso3
			v = osc.Reso3(phase, shape);
			break;
		}
		return v;
	}
};

struct CZOsc : Module {
	enum ParamIds {
		_WAVE_PARAM,
		_LFO_PARAM,
		_FREQ_PARAM,
		_FINE_PARAM,
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

	_CZOsc osc[4];

	CZOsc() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam<_CZWaveNames>(_WAVE_PARAM, 0, 7, 0, "");
		configParam<_OnOff>(_LFO_PARAM, 0, 1, 0, "");
		configParam(_FREQ_PARAM, -54.f, 54.f, 0.f, "Frequency", " Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);
		configParam(_FINE_PARAM, -1.f, 1.f, 0.f, "Fine frequency / LFO Offset");
		configParam(_SHAPE_PARAM, 0.f, 1.f, 0.f, "Shape");
	}

	void onAdd() override;

	void onReset() override;

	void process(const ProcessArgs& args) override;
};

void CZOsc::onAdd() {
	osc->init();
}

void CZOsc::onReset() {
	onAdd();
}

void CZOsc::process(const ProcessArgs& args) {
	// Get the frequency parameters
	float freqParam = params[_FREQ_PARAM].getValue() / 12.f;
	// Get the selected wave
	int wave = params[_WAVE_PARAM].getValue();
	// LFO mode
	if (params[_LFO_PARAM].getValue())
		freqParam = (freqParam * 2) - 5;
	freqParam += dsp::quadraticBipolar(params[_FINE_PARAM].getValue()) * 3.f / 12.f;
	// Get the shape parameter
	float shapeParam = params[_SHAPE_PARAM].getValue();
	// Iterate over each channel
	int channels = max(inputs[_MODF_INPUT].getChannels(), 1);
	for (int c = 0; c < channels; c += 4) {
		// Get the oscillator
		auto* oscillator = &osc[c / 4];
		oscillator->setWave(wave);
		float_4 pitch = freqParam;
		// Set the pitch
		pitch += inputs[_MODF_INPUT].getVoltageSimd<float_4>(c);
		oscillator->setPitch(pitch, 1.f);
		// Set the shape
		float_4 shape = shapeParam;
		if (inputs[_MODS_INPUT].isConnected())
			shape *= inputs[_MODS_INPUT].getVoltageSimd<float_4>(c) / 10.f;
		oscillator->setShape(shape);
		// Process and output
		oscillator->process(args.sampleTime);
		float_4 off = params[_LFO_PARAM].getValue() * params[_FINE_PARAM].getValue() * 5.f;
		outputs[_WAVE_OUTPUT].setVoltageSimd(5.f * oscillator->_Out() + off, c);
	}
	outputs[_WAVE_OUTPUT].setChannels(channels);
}

struct CZOscWidget : ModuleWidget {
	CZOscWidget(CZOsc* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CZOsc.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Selector>(mm2px(Vec(5.1, 35.0)), module, CZOsc::_WAVE_PARAM));
		addParam(createParamCentered<_Hsw>(mm2px(Vec(5.1, 46.0)), module, CZOsc::_LFO_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 57.0)), module, CZOsc::_FREQ_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 68.0)), module, CZOsc::_FINE_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 90.0)), module, CZOsc::_SHAPE_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, CZOsc::_MODF_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, CZOsc::_MODS_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, CZOsc::_WAVE_OUTPUT));
	}
};

Model* modelCZOsc = createModel<CZOsc, CZOscWidget>("CZOsc");