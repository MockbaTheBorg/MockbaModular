// A Moog style filter by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct Filtah : Module {
	enum ParamIds {
		_SL1_PARAM,		// Type: LP, BP, HP
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

	Filtah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_SL1_PARAM, 0, 2, 0, "LP BP HP");
		configParam(_SL2_PARAM, 0, 100, 100, "%");
		configParam(_SL3_PARAM, 0, 0.85, 0, "");
		configParam(_SL5_PARAM, -25, 25, 0, "dB");
	}

	float cDenorm = 10 ^ -30;
	float mv = powf(2, -0.2 / 6);

	//fir restoration
	float c1 = 1;
	float c2 = -0.75;
	float c3 = 0.17;
	float fgain = 5;

	//fir bandlimit
	float bl_c1 = 0.52;
	float bl_c2 = 0.54;
	float bl_c3 = -0.02;

	float tk = 0;
	float tp = 0;
	float tr = 0;

	float ps_out1l = 0;
	float ps_out2l = 0;

	float o_in1l = 0;
	float o_in2l = 0;

	float bl1_1 = 0;
	float bl2_1 = 0;
	float bl3_1 = 0;
	float bl1_2 = 0;
	float bl2_2 = 0;
	float bl3_2 = 0;

	float bl_out1 = 0;
	float bl_out2 = 0;

	float o_out1l = 0;
	float o_out2l = 0;

	float s1l = 0;
	float s2l = 0;
	float s3l = 0;

	float src_k = 0;
	float src_p = 0;
	float src_r = 0;
	float tgt_k = 0;
	float tgt_p = 0;
	float tgt_r = 0;
	float d_k = 0;
	float d_p = 0;
	float d_r = 0;

	float x = 0;
	float y1 = 0;
	float y2 = 0;
	float y3 = 0;
	float y4 = 0;

	float oldx = 0;
	float oldy1 = 0;
	float oldy2 = 0;
	float oldy3 = 0;
	float oldy4 = 0;

	float in = 0;
	float out = 0;

	void process(const ProcessArgs& args) override;
};

void Filtah::process(const ProcessArgs& args) {
	float ftype = params[_SL1_PARAM].getValue();
	float sx = 16 + params[_SL2_PARAM].getValue() * 1.20103;
	float res = params[_SL3_PARAM].getValue();
	float outgain = powf(10, params[_SL5_PARAM].getValue() / 20);

	if (inputs[_MODC_INPUT].isConnected())
		sx *= inputs[_MODC_INPUT].getVoltage() / 10;

	if (inputs[_MODR_INPUT].isConnected())
		res *= inputs[_MODR_INPUT].getVoltage() / 10;

	float spl0 = inputs[_SP0_INPUT].getVoltage();

	float cutoff = floor(exp(sx * log(1.059)) * 8.17742);

	float f = 2 * cutoff / args.sampleRate;
	tgt_k = 3.6 * f - 1.6 * f * f - 1;
	tgt_p = (tgt_k + 1) * 0.5;
	float scale = powf(M_E, (1 - tgt_p) * 1.386249);
	tgt_r = res * scale;

	d_p = tgt_p - src_p;
	tp = src_p;
	src_p = tgt_p;
	d_k = tgt_k - src_k;
	tk = src_k;
	src_k = tgt_k;
	d_r = tgt_r - src_r;
	tr = src_r;
	src_r = tgt_r;

	tk += d_k;
	tp += d_p;
	tr += d_r;

	float in = spl0;

	//filter
	x = in - tr * y4;

	y1 = x * tp + oldx * tp - tk * y1;
	y2 = y1 * tp + oldy1 * tp - tk * y2;
	y3 = y2 * tp + oldy2 * tp - tk * y3;
	y4 = y3 * tp + oldy3 * tp - tk * y4;

	oldx = x;
	oldy1 = y1;
	oldy2 = y2;
	oldy3 = y3;

	if (ftype == 0) {
		out = y4;
	}
	if (ftype == 1) {
		out = 6 * (y3 - y4);
	}
	if (ftype == 2) {
		out = in - y4;
	}

	out *= outgain;

	spl0 = clamp(out, -5.0f, +5.0f);
	outputs[_SP0_OUTPUT].setVoltage(spl0);
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