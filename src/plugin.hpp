// VCV Rack plugins by Mockba the Borg

#pragma once
#include <rack.hpp>

#ifdef _WIN32
	#ifndef ARCH_WIN
		#define ARCH_WIN                  // Just to make Visual Studio happy
	#endif
#endif

#ifdef ARCH_WIN
	#include <winsock2.h>                 // Needed for all Winsock stuff
	#define NOSOCKET INVALID_SOCKET
	#define NOBIND SOCKET_ERROR
#else
	#include <sys/select.h>               // Needed for sockets stuff
	#include <sys/socket.h>               // Needed for sockets stuff
	#include <arpa/inet.h>                // Needed for sockets stuff
	#include <fcntl.h>                    // Needed for sockets stuff
	#include <unistd.h>
	#define NOSOCKET -1
	#define NOBIND -1
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef M_2PI
#define M_2PI    6.28318530717958647692
#endif

template <typename T>
T expCurve(T x) {
	return (3 + x * (-13 + 5 * x)) / (3 + 2 * x);
}

using namespace rack;
using simd::float_4;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

struct _Screw : SvgScrew {
	_Screw() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/screw.svg")));
	}
};

struct _KnobTop : SvgScrew {
	_KnobTop() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob2a.svg")));
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
// extern Model* modelMyModule;
extern Model* modelBlank;
extern Model* modelFeidah;
extern Model* modelFeidahS;
extern Model* modelFiltah;
extern Model* modelMixah;
extern Model* modelDividah;
extern Model* modelCountah;
extern Model* modelSelectah;
extern Model* modelShapah;
extern Model* modelCZSaw;
extern Model* modelCZSquare;
extern Model* modelCZPulse;
extern Model* modelCZDblSine;
extern Model* modelCZSawPulse;
extern Model* modelCZReso1;
extern Model* modelCZReso2;
extern Model* modelCZReso3;
extern Model* modelComparator;
extern Model* modelDualBUFFER;
extern Model* modelDualNOT;
extern Model* modelDualOR;
extern Model* modelDualNOR;
extern Model* modelDualAND;
extern Model* modelDualNAND;
extern Model* modelDualXOR;
extern Model* modelDualXNOR;
extern Model* modelUDPClockSlave;
extern Model* modelUDPClockMaster;
