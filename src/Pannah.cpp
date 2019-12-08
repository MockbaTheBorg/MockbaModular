// Simple stereo panner by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct Pannah : Module {
	enum ParamIds {
		_KNOB_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_MOD_INPUT,
		_VOLTAGE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_VOLTAGEL_OUTPUT,
		_VOLTAGER_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Pannah() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_KNOB_PARAM, 0.f, 1.f, 0.5f, "pan");
	}

	void process(const ProcessArgs& args) override;
};

void Pannah::process(const ProcessArgs& args) {
	float pan = params[_KNOB_PARAM].getValue();
	float in;
	float outl = 0.f;
	float outr = 0.f;
	// Iterate over each channel
	int channels = max(inputs[_VOLTAGE_INPUT].getChannels(), 1);
	for (int c = 0; c < channels; ++c) {
		in = inputs[_VOLTAGE_INPUT].getVoltage(c);
		if (inputs[_MOD_INPUT].isConnected())
			pan = clamp(inputs[_MOD_INPUT].getVoltage(), -5.f, 5.f) / 10.f + .5f;
		outl = in * (1.f - pan);
		outr = -in * pan;
		outputs[_VOLTAGEL_OUTPUT].setVoltage(outl, c);
		outputs[_VOLTAGER_OUTPUT].setVoltage(outr, c);
	}
	outputs[_VOLTAGEL_OUTPUT].setChannels(channels);
	outputs[_VOLTAGER_OUTPUT].setChannels(channels);
}

struct PannahWidget : ModuleWidget {
	PannahWidget(Pannah* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Pannah.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 57.0)), module, Pannah::_KNOB_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, Pannah::_MOD_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, Pannah::_VOLTAGE_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, Pannah::_VOLTAGEL_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, Pannah::_VOLTAGER_OUTPUT));
	}
};

Model* modelPannah = createModel<Pannah, PannahWidget>("Pannah");