// MockbaModular plugins for VCV Rack by Mockba the Borg

#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

struct _Screw : SvgScrew {
	_Screw() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/screw.svg")));
	}
};

struct _Knob : RoundKnob {
	_Knob() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob.svg")));
		minAngle = -0.68 * M_PI;
		maxAngle = 0.68 * M_PI;
	}
};

struct _Selector : RoundKnob {
	_Selector() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/selector.svg")));
		minAngle = -0.40 * M_PI;
		maxAngle = 0.40 * M_PI;
		snap = true;
	}
};

struct _Hsw : SvgSwitch {
	_Hsw() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HSW_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HSW_2.svg")));
	}
};

struct _Hsw3 : SvgSwitch {
	_Hsw3() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HSW_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HSW_1.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HSW_2.svg")));
	}
};

struct _Port : SVGPort {
	_Port() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/port.svg")));
	}
};

// Declare each Model, defined in each module source file
extern Model* modelBlank;
extern Model* modelFeidah;
extern Model* modelFeidahS;
extern Model* modelFiltah;
extern Model* modelMixah;
extern Model* modelMixah3;
extern Model* modelDividah;
extern Model* modelCountah;
extern Model* modelSelectah;
extern Model* modelShapah;
extern Model* modelHoldah;
extern Model* modelPannah;
extern Model* modelReVoltah;
extern Model* modelCZSaw;
extern Model* modelCZSquare;
extern Model* modelCZPulse;
extern Model* modelCZDblSine;
extern Model* modelCZSawPulse;
extern Model* modelCZReso1;
extern Model* modelCZReso2;
extern Model* modelCZReso3;
extern Model* modelCZOsc;
extern Model* modelMaugTriangle;
extern Model* modelMaugShark;
extern Model* modelMaugSaw;
extern Model* modelMaugSaw2;
extern Model* modelMaugSquare;
extern Model* modelMaugSquare2;
extern Model* modelMaugSquare3;
extern Model* modelMaugOsc;
extern Model* modelComparator;
extern Model* modelDualBUFFER;
extern Model* modelDualNOT;
extern Model* modelDualOR;
extern Model* modelDualNOR;
extern Model* modelDualAND;
extern Model* modelDualNAND;
extern Model* modelDualXOR;
extern Model* modelDualXNOR;
extern Model* modelPSelectah;
extern Model* modelUDPClockSlave;
extern Model* modelUDPClockMaster;
