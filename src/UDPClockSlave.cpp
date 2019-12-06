// Simple UDP clock sync slave by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

#define PORT_NUM     7000             // Listening Port = PORT_NUM + _CHANNEL_PARAM

struct UDPClockSlave : Module {
	enum ParamIds {
		_RESTART_BUTTON,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		_CLOCK_OUTPUT,
		_RESET_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		_STATUS_LIGHT,
		NUM_LIGHTS
	};

#ifdef ARCH_WIN
	WORD wVersionRequested = MAKEWORD(2, 2);  // Stuff for WSA functions
	WSADATA wsaData;                          // Stuff for WSA functions
	typedef int socklen_t;
#endif
	int                  server_s;        // Server socket descriptor
	unsigned long int    noBlock;         // Non-blocking flag
	struct sockaddr_in   server_addr;     // Server Internet address
	struct sockaddr_in   client_addr;     // Client Internet address
	struct timeval       timeout;         // Server timeout
	socklen_t            addr_len;        // Internet address length
	char                 in_buf[1024];    // Input buffer for data
	int                  retcode;         // Return code
	int                  iOptVal;         // Socket option value
	int                  iOptLen;         // Socket option length

	bool                 running;         // Runs only if the socket succeeds
	bool                 was0;

	dsp::PulseGenerator clockPulse, resetPulse;

	UDPClockSlave() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void onAdd() override;

	void onRemove() override;

	void onReset() override;

	void process(const ProcessArgs& args) override;
};

void UDPClockSlave::onAdd() {
	running = true;
	was0 = true;
	lights[_STATUS_LIGHT].setBrightness(0.0);

#ifdef ARCH_WIN
	// Initalize Winsock
	WSAStartup(wVersionRequested, &wsaData);
#endif

	// Create a socket
	server_s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (server_s < 0)
	{
		running = false;
		lights[_STATUS_LIGHT].setBrightness(0.5);
	}

	// Fill-in server socket's address information
	server_addr.sin_family = AF_INET;                 // Address family to use
	server_addr.sin_port = htons(PORT_NUM);           // Port number to use
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Listen on any IP address

	// Set socket to reuse address, so multiple slavahs can be added
	iOptVal = 1;
	iOptLen = sizeof(int);
	setsockopt(server_s, SOL_SOCKET, SO_REUSEADDR, (char*)&iOptVal, iOptLen);

	retcode = bind(server_s, (struct sockaddr*) & server_addr,
		sizeof(server_addr));
	if (retcode < 0)
	{
		running = false;
		lights[_STATUS_LIGHT].setBrightness(1.0);
	}

	// Make client_s non-blocking
#ifdef ARCH_WIN
	noBlock = 1;
	ioctlsocket(server_s, FIONBIO, &noBlock);
#else
	int flags = fcntl(server_s, F_GETFL, 0);
	fcntl(server_s, F_SETFL, flags | O_NONBLOCK);
#endif
}

void UDPClockSlave::onRemove() {
#ifdef ARCH_WIN
	retcode = closesocket(server_s);
	WSACleanup();
#else
	retcode = close(server_s);
#endif
}

void UDPClockSlave::onReset() {
	if (was0) {
		was0 = false;
		onRemove();
		onAdd();
		resetPulse.trigger(.001);
	} else {
		was0 = true;
	}
}

void UDPClockSlave::process(const ProcessArgs& args) {
	if (params[_RESTART_BUTTON].getValue()) {
		onReset();
	}
	retcode = recvfrom(server_s, in_buf, sizeof(in_buf), 0,
		(struct sockaddr*) &client_addr, &addr_len);
	if (retcode > 0) {
		if(in_buf[0] == 'C') {
			clockPulse.trigger(.001);
		}
		if (in_buf[0] == 'R') {
			resetPulse.trigger(.001);
		}
	}
	bool cPulse = running && clockPulse.process(1.0 / args.sampleRate);
	outputs[_CLOCK_OUTPUT].setVoltage(cPulse ? 10.0 : 0.0);
	bool rPulse = running && resetPulse.process(1.0 / args.sampleRate);
	outputs[_RESET_OUTPUT].setVoltage(rPulse ? 10.0 : 0.0);
}

struct UDPClockSlaveWidget : ModuleWidget {
	UDPClockSlaveWidget(UDPClockSlave* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/UDPClockSlave.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Buttons
		addChild(createParam<LEDButton>(mm2px(Vec(2.0, 60.0)), module, UDPClockSlave::_RESTART_BUTTON));

		// Lights
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(5.011, 86.116)), module, UDPClockSlave::_STATUS_LIGHT));

		// Ports
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.011, 95.772)), module, UDPClockSlave::_RESET_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.011, 106.383)), module, UDPClockSlave::_CLOCK_OUTPUT));
	}
};

Model* modelUDPClockSlave = createModel<UDPClockSlave, UDPClockSlaveWidget>("UDPClockSlave");