// Moog style oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

template <int OVERSAMPLE, int QUALITY, typename T>
struct _Osc {
	int wave;
	T freq;
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

	void process(float delta) {
		// Calculate phase
		T deltaPhase = simd::clamp(freq * delta, 1e-6f, 0.35f);
		phase += deltaPhase;
		phase -= simd::floor(phase);

		outValue = oscStep(phase, wave);
		outValue += oscMinBlep.process();
	}

	T oscStep(T phase, int wave) {
		// Calculate the wave step
		T a, b, c, d, v;
		switch (wave) {
		case 0:		// Triangle
			a = phase + phase - 1.f;
			b = simd::sgn(a);
			c = simd::fmod(phase + phase + phase + phase, 2.f) - 1.f;
			d = (c * c - 1) / 3;
			v = b * d - b * c;
			break;
		case 1:		// Shark Tooth
			a = simd::fmod(phase + phase + phase, 1.5f) - 1.f;
			b = 1.f - simd::fmod(phase + phase + phase + phase, 2.f);
			c = (b * b - 1.f) / 5.f;
			v = simd::ifelse(phase < 0.5f, a - c, b + c);
			break;
		case 2:		// Saw
			a = phase + phase - 1.f;
			b = (a * a - 1) / 2;
			v = a - b;
			break;
		case 3:		// Square .48
			a = simd::cos(1.f - phase) - 1.54f;
			b = -a;
			v = simd::ifelse(phase < 0.48f, b, a - 0.3f);
			break;
		case 4:		// Square .29
			a = simd::cos(1.f - phase) - 1.54f;
			b = -a;
			v = simd::ifelse(phase < 0.29f, b, a - 0.2f);
			break;
		case 5:		// Square .17
			a = simd::cos(1.f - phase) - 1.54f;
			b = -a;
			v = simd::ifelse(phase < 0.17f, b, a - 0.1f);
			break;
		case 6:
			a = (1.f - phase) + (1.f - phase) - 1.f;
			b = (a * a - 1) / 4;
			v = a + b;
			break;
		}
		return v;
	}

	T out() {
		return outValue;
	}
};

struct MaugOsc : Module {
	enum ParamIds {
		_WAVE_PARAM,
		_LFO_PARAM,
		_FREQ_PARAM,
		_FINE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_MODF_INPUT,
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

	MaugOsc() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_WAVE_PARAM, 0, 6, 0, "");
		configParam(_LFO_PARAM, 0, 1, 0, "OFF ON");
		configParam(_FREQ_PARAM, -54.f, 54.f, 0.f, "Frequency", " Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);
		configParam(_FINE_PARAM, -1.f, 1.f, 0.f, "Fine frequency / LFO Offset");
	}

	void onAdd() override;

	void onReset() override;

	void process(const ProcessArgs& args) override;
};

void MaugOsc::onAdd() {
}

void MaugOsc::onReset() {
	onAdd();
}

void MaugOsc::process(const ProcessArgs& args) {
	// Get the frequency parameters
	float freqParam = params[_FREQ_PARAM].getValue() / 12.f;
	// Get the selected wave
	int wave = params[_WAVE_PARAM].getValue();
	// LFO mode
	if (params[_LFO_PARAM].getValue())
		freqParam = (freqParam * 2) - 5;
	freqParam += dsp::quadraticBipolar(params[_FINE_PARAM].getValue()) * 3.f / 12.f;
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
		// Process and output
		oscillator->process(args.sampleTime);
		float_4 off = params[_LFO_PARAM].getValue() * params[_FINE_PARAM].getValue() * 5.f;
		outputs[_WAVE_OUTPUT].setVoltageSimd(5.f * oscillator->out() + off, c);
	}
	outputs[_WAVE_OUTPUT].setChannels(channels);
}

struct MaugOscWidget : ModuleWidget {
	MaugOscWidget(MaugOsc* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MaugOsc.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Selector>(mm2px(Vec(5.1, 57.0)), module, MaugOsc::_WAVE_PARAM));
		addParam(createParamCentered<_Hsw>(mm2px(Vec(5.1, 68.0)), module, MaugOsc::_LFO_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 79.0)), module, MaugOsc::_FREQ_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 90.0)), module, MaugOsc::_FINE_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, MaugOsc::_MODF_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, MaugOsc::_WAVE_OUTPUT));
	}
};

Model* modelMaugOsc = createModel<MaugOsc, MaugOscWidget>("MaugOsc");