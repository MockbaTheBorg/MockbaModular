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
#else
	#include <sys/select.h>               // Needed for sockets stuff
	#include <sys/socket.h>               // Needed for sockets stuff
	#include <arpa/inet.h>                // Needed for sockets stuff
	#include <fcntl.h>                    // Needed for sockets stuff
	#include <unistd.h>
#endif

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

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

struct _Port : SVGPort {
	_Port() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/port.svg")));
	}
};

// Declare each Model, defined in each module source file
// extern Model* modelMyModule;
extern Model* modelFeidah;
extern Model* modelDividah;
extern Model* modelCountah;
extern Model* modelUDPClockSlave;
extern Model* modelUDPClockMaster;
