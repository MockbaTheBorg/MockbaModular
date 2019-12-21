// Casio CZ style oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

template <int OVERSAMPLE, int QUALITY, typename T>
struct _Osc {
	int wave;
	T freq;
	T shape;
	T phase = 0.f;
	T outValue = 0.f;

	dsp::MinBlepGenerator<QUALITY, OVERSAMPLE, T> oscMinBlep;

	void setWave(int waveV) {
		wave = waveV;
	}

	void setPitch(T pitchV) {
		freq = dsp::FREQ_C4 * dsp::approxExp2_taylor5(pitchV + 30) / 1073741824;
		for (int i = 0; i < 4; i++)
			freq[i] += i / DETUNE;
	}

	void setShape(T shapeV) {
		shape = simd::clamp(shapeV, 0.01f, 0.99f);
	}

	void process(float delta) {
		// Calculate phase
		T deltaPhase = simd::clamp(freq * delta, 1e-6f, 0.35f);
		phase += deltaPhase;
		phase -= simd::floor(phase);

		outValue = oscStep(phase, wave, shape);
		outValue += oscMinBlep.process();
	}

	T oscStep(T phase, int wave, T shape) {
		// Calculate the wave step
		T a, b, c, d, v;
		switch (wave) {
			case 0:		// Saw
				a = 0.5f - (shape * 0.5f);
				b = phase * ((0.5f - a) / a);
				c = (-phase + 1.f) * ((0.5f - a) / (1.f - a));
				d = phase + simd::fmin(b, c);
				v = simd::cos(d * M_2PI);
				break;
			case 1:		// Square
				a = simd::sgn(0.5f - phase);
				b = simd::fmod(phase + phase, 1.f);
				c = (-b + 1.f) * (shape / (1.f - shape));
				d = 0.5f * (b - simd::fmin(b, c));
				v = simd::cos(d * M_2PI) * a;
				break;
			case 2:		// Pulse
				a = (1.f - phase) / (1.f - shape);
				b = simd::fmax(0.f, 1.f - a);
				c = simd::fmin(a, b);
				v = simd::cos(c * M_2PI);
				break;
			case 3:		// DblSine
				a = 0.5f - (shape * 0.5f);
				b = phase * ((0.5f - a) / a);
				c = (-phase + 1.f) * ((0.5f - a) / (1.f - a));
				d = phase + simd::fmin(b, c);
				v = simd::cos(2.f * d * M_2PI);
				break;
			case 4:		// SawPulse
				a = -2.f * phase + 2.f;
				b = (-a + 1.f) * (shape / (1.f - shape));
				c = 0.5f * (a - simd::fmin(a, b));
				d = simd::fmin(c, phase);
				v = simd::cos(d * M_2PI);
				break;
			case 5:		// Reso1
				a = 1.f - phase;
				b = phase * (.0625 + shape) * 16.f;
				v = simd::cos(b * M_2PI) * a + phase;
				break;
			case 6:		// Reso2
				a = simd::fmin(2.f - (phase + phase), phase + phase);
				b = simd::fmod(phase * (.0625 + shape) * 16.f, 1);
				v = simd::cos(b * M_2PI) * a + (1 - a);
				break;
			case 7:		// Reso3
				a = simd::fmin(2.f - (phase + phase), 1);
				b = phase * (.0625 + shape) * 16.f;
				v = simd::cos(b * M_2PI) * a + (1 - a);
				break;
		}
		return v;
	}

	T out() {
		return outValue;
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

	_Osc<16, 16, float_4> osc[4];

	CZOsc() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_WAVE_PARAM, 0, 7, 0, "");
		configParam(_LFO_PARAM, 0, 1, 0, "OFF ON");
		configParam(_FREQ_PARAM, -54.f, 54.f, 0.f, "Frequency", " Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);
		configParam(_FINE_PARAM, -1.f, 1.f, 0.f, "Fine frequency / LFO Offset");
		configParam(_SHAPE_PARAM, 0.f, 1.f, 0.f, "Shape");
	}

	void onAdd() override;

	void onReset() override;

	void process(const ProcessArgs& args) override;
};

void CZOsc::onAdd() {
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
		oscillator->setPitch(pitch);
		// Set the shape
		float_4 shape = shapeParam;
		if (inputs[_MODS_INPUT].isConnected())
			shape *= inputs[_MODS_INPUT].getVoltageSimd<float_4>(c) / 10.f;
		oscillator->setShape(shape);
		// Process and output
		oscillator->process(args.sampleTime);
		float_4 off = params[_LFO_PARAM].getValue() * params[_FINE_PARAM].getValue() * 5.f;
		outputs[_WAVE_OUTPUT].setVoltageSimd(5.f * oscillator->out() + off, c);
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