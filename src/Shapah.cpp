// Simple ADSR shaper based on VCV fundamental

#include "plugin.hpp"
#include "MockbaModular.hpp"

const float MIN_TIME = 1e-3f;
const float MAX_TIME = 10.f;
const float LAMBDA_BASE = MAX_TIME / MIN_TIME;

struct Shapah : Module {
	enum ParamIds {
		_ATTACK_PARAM,
		_DECAY_PARAM,
		_SUSTAIN_PARAM,
		_RELEASE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_GATE_INPUT,
		_RTRG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_ADSR_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	float_4 attacking[4] = {float_4::zero()};
	float_4 env[4] = {0.f};
	dsp::TSchmittTrigger<float_4> trigger[4];
	dsp::ClockDivider cvDivider;
	float_4 attackLambda[4] = {0.f};
	float_4 decayLambda[4] = {0.f};
	float_4 releaseLambda[4] = {0.f};
	float_4 sustain[4] = {0.f};

	Shapah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_ATTACK_PARAM, 0.f, 1.f, 0.5f, "Attack", " ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(_DECAY_PARAM, 0.f, 1.f, 0.5f, "Decay", " ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(_SUSTAIN_PARAM, 0.f, 1.f, 0.5f, "Sustain", "%", 0, 100);
		configParam(_RELEASE_PARAM, 0.f, 1.f, 0.5f, "Release", " ms", LAMBDA_BASE, MIN_TIME * 1000);
		cvDivider.setDivision(16);
	}

	void process(const ProcessArgs& args) override;
};

void Shapah::process(const ProcessArgs& args) {
	int channels = inputs[_GATE_INPUT].getChannels();
	int nc;

	// Compute lambdas
	if (cvDivider.process()) {
		float attackParam = params[_ATTACK_PARAM].getValue();
		float decayParam = params[_DECAY_PARAM].getValue();
		float sustainParam = params[_SUSTAIN_PARAM].getValue();
		float releaseParam = params[_RELEASE_PARAM].getValue();

		for (int c = 0; c < channels; c += 4) {
			nc = c / 4;
			// CV
			float_4 attack = attackParam;
			float_4 decay = decayParam;
			float_4 sustain = sustainParam;
			float_4 release = releaseParam;

			attack = simd::clamp(attack, 0.f, 1.f);
			decay = simd::clamp(decay, 0.f, 1.f);
			sustain = simd::clamp(sustain, 0.f, 1.f);
			release = simd::clamp(release, 0.f, 1.f);

			attackLambda[nc] = simd::pow(LAMBDA_BASE, -attack) / MIN_TIME;
			decayLambda[nc] = simd::pow(LAMBDA_BASE, -decay) / MIN_TIME;
			releaseLambda[nc] = simd::pow(LAMBDA_BASE, -release) / MIN_TIME;
			this->sustain[nc] = sustain;
		}
	}

	float_4 gate[4];

	for (int c = 0; c < channels; c += 4) {
		nc = c / 4;
		gate[nc] = inputs[_GATE_INPUT].getVoltageSimd<float_4>(c) >= 1.f;
		float_4 triggered = trigger[nc].process(inputs[_RTRG_INPUT].getPolyVoltageSimd<float_4>(c));
		attacking[nc] = simd::ifelse(triggered, float_4::mask(), attacking[nc]);
		const float attackTarget = 1.2f;
		float_4 target = simd::ifelse(gate[nc], simd::ifelse(attacking[nc], attackTarget, sustain[nc]), 0.f);
		float_4 lambda = simd::ifelse(gate[nc], simd::ifelse(attacking[nc], attackLambda[nc], decayLambda[nc]), releaseLambda[nc]);
		env[nc] += (target - env[nc]) * lambda * args.sampleTime;
		attacking[nc] = simd::ifelse(env[nc] >= 1.f, float_4::zero(), attacking[nc]);
		attacking[nc] = simd::ifelse(gate[nc], attacking[nc], float_4::mask());
		outputs[_ADSR_OUTPUT].setVoltageSimd(10.f * env[nc], c);
	}
	outputs[_ADSR_OUTPUT].setChannels(channels);
}

struct ShapahWidget : ModuleWidget {
	ShapahWidget(Shapah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Shapah.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 46.0)), module, Shapah::_ATTACK_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 57.0)), module, Shapah::_DECAY_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 68.0)), module, Shapah::_SUSTAIN_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 79.0)), module, Shapah::_RELEASE_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, Shapah::_GATE_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Shapah::_RTRG_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, Shapah::_ADSR_OUTPUT));
	}
};


Model* modelShapah = createModel<Shapah, ShapahWidget>("Shapah");