// Casio CZ style Reso2 oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

template <int OVERSAMPLE, int QUALITY, typename T>
struct _Reso2 {
	T freq;
	T shape;
	T phase = 0.f;
	T outValue = 0.f;

	dsp::MinBlepGenerator<QUALITY, OVERSAMPLE, T> oscMinBlep;

	void setPitch(T pitchV) {
		freq = dsp::FREQ_C4 * dsp::approxExp2_taylor5(pitchV + 30) / 1073741824;
		for (int i = 0; i < 4; i++)
			freq[i] += i / DETUNE;
	}

	void setShape(T shapeV) {
		shape = simd::clamp(shapeV, 0.f, 10.f) * 0.1f;
	}

	void process(float delta) {
		// Calculate phase
		T deltaPhase = simd::clamp(freq * delta, 1e-6f, 0.35f);
		phase += deltaPhase;
		phase -= simd::floor(phase);

		outValue = oscStep(phase, shape);
		outValue += oscMinBlep.process();
	}

	T oscStep(T phase, T shape) {
		// Calculate the wave step
		T w = simd::fmin(2.f - (phase * 2.f), phase * 2.f);
		T m = simd::fmod(phase * ((.0625 + shape) * 16.f), 1);
		T v = simd::cos(m * M_2PI) * w + (1 - w);
		return v;
	}

	T out() {
		return outValue;
	}
};

struct CZReso2 : Module {
	enum ParamIds {
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

	_Reso2<16, 16, float_4> osc[4];

	CZReso2() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_FREQ_PARAM, -54.f, 54.f, 0.f, "Frequency", " Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);
		configParam(_FINE_PARAM, -1.f, 1.f, 0.f, "Fine frequency");
		configParam(_SHAPE_PARAM, 0.f, 10.f, 0.f, "Shape");
	}

	void onAdd() override;

	void onReset() override;

	void process(const ProcessArgs& args) override;
};

void CZReso2::onAdd() {
}

void CZReso2::onReset() {
	onAdd();
}

void CZReso2::process(const ProcessArgs& args) {
	// Get the frequency parameters
	float freqParam = params[_FREQ_PARAM].getValue() / 12.f;
	freqParam += dsp::quadraticBipolar(params[_FINE_PARAM].getValue()) * 3.f / 12.f;
	// Get the shape parameter
	float shapeParam = params[_SHAPE_PARAM].getValue();
	// Iterate over each channel
	int channels = max(inputs[_MODF_INPUT].getChannels(), 1);
	for (int c = 0; c < channels; c += 4) {
		// Get the oscillator
		auto* oscillator = &osc[c / 4];
		float_4 pitch = freqParam;
		// Set the pitch
		pitch += inputs[_MODF_INPUT].getVoltageSimd<float_4>(c);
		oscillator->setPitch(pitch);
		// Set the shape
		float_4 shape = shapeParam;
		shape += inputs[_MODS_INPUT].getVoltageSimd<float_4>(c);
		oscillator->setShape(shape);
		// Process and output
		oscillator->process(args.sampleTime);
		outputs[_WAVE_OUTPUT].setVoltageSimd(5.f * oscillator->out(), c);
	}
	outputs[_WAVE_OUTPUT].setChannels(channels);
}

struct CZReso2Widget : ModuleWidget {
	CZReso2Widget(CZReso2* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CZReso2.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 57.0)), module, CZReso2::_FREQ_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 68.0)), module, CZReso2::_FINE_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 90.0)), module, CZReso2::_SHAPE_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, CZReso2::_MODF_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, CZReso2::_MODS_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, CZReso2::_WAVE_OUTPUT));
	}
};

Model* modelCZReso2 = createModel<CZReso2, CZReso2Widget>("CZReso2");