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

struct _SnapKnob : _Knob {
	_SnapKnob() {
		snap = true;
	}
};

struct _Selector : _SnapKnob {
	_Selector() {
		minAngle = -0.40 * M_PI;
		maxAngle = 0.40 * M_PI;
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
extern Model* modelMixah;
extern Model* modelDividah;
extern Model* modelCountah;
extern Model* modelSelectah;
extern Model* modelUDPClockSlave;
extern Model* modelUDPClockMaster;
