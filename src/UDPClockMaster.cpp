// Simple UDP clock sync master by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

#define PORT_NUM     7000             // Listening Port = PORT_NUM + _CHANNEL_PARAM
#define IP_ADDR      "192.168.1.255"  // IP address to broadcast to (must be added to setup later)

void saveIPAddress(const char* IPAddress) {
	json_t* settingsJ = json_object();
	json_object_set_new(settingsJ, "IPAddress", json_string(IPAddress));
	json_object_set_new(settingsJ, "Background", json_string(BGCOLOR.c_str()));
	std::string settingsFilename = asset::user("MockbaModular.json");
	FILE* file = fopen(settingsFilename.c_str(), "w");
	if (file) {
		json_dumpf(settingsJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
		fclose(file);
	}
	json_decref(settingsJ);
}

const char* loadIPAddress() {
	const char* ret = IP_ADDR;
	std::string settingsFilename = asset::user("MockbaModular.json");
	FILE* file = fopen(settingsFilename.c_str(), "r");
	if (!file) {
		saveIPAddress(IP_ADDR);
		return IP_ADDR;
	}
	json_error_t error;
	json_t* settingsJ = json_loadf(file, 0, &error);
	if (!settingsJ) {
		// invalid setting json file
		fclose(file);
		saveIPAddress(IP_ADDR);
		return IP_ADDR;
	}
	json_t* IPAddressJ = json_object_get(settingsJ, "IPAddress");
	if (IPAddressJ) {
		ret = json_string_value(IPAddressJ);
	} else {
		ret = IP_ADDR;
		saveIPAddress(IP_ADDR);
	}

	fclose(file);
	json_decref(settingsJ);
	return ret;
}

struct UDPClockMaster : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		_CLOCK_INPUT,
		_RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		_STATUS_LIGHT,
		NUM_LIGHTS
	};

#ifdef ARCH_WIN
	WORD wVersionRequested = MAKEWORD(2, 2);  // Stuff for WSA functions
	WSADATA wsaData;                          // Stuff for WSA functions
#endif
	int                  client_s;        // Client socket descriptor
	struct sockaddr_in   server_addr;     // Server Internet address
	char                 out_buf[64];     // Output buffer for data
	int                  retcode;         // Return code
	int                  iOptVal;         // Socket option value
	int                  iOptLen;         // Socket option length

	bool                 running;         // Runs only if the socket succeeds
	bool                 clock_was0;      // Used to control when clock pulse is sent
	bool                 reset_was0;      // Used to control when clock pulse is sent

	UDPClockMaster() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void onAdd() override;

	void onRemove() override;

	void process(const ProcessArgs& args) override;
};

void UDPClockMaster::onAdd() {
	clock_was0 = true;
	running = true;
	lights[_STATUS_LIGHT].setBrightness(0.0);

#ifdef ARCH_WIN
	// Initalize Winsock
	WSAStartup(wVersionRequested, &wsaData);
#endif

	// Create a socket
	client_s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (client_s < 0) {
		running = false;
		lights[_STATUS_LIGHT].setBrightness(1.0);
	}

	// Fill-in server socket's address information
	server_addr.sin_family = AF_INET;                         // Address family to use
	server_addr.sin_port = htons(PORT_NUM);                   // Port num to use
	server_addr.sin_addr.s_addr = inet_addr(loadIPAddress()); // Need this for Broadcast

	// Set socket to use MAC-level broadcast
	iOptVal = 1;
	iOptLen = sizeof(int);
	setsockopt(client_s, SOL_SOCKET, SO_BROADCAST, (char*)&iOptVal, iOptLen);
}

void UDPClockMaster::onRemove() {
#ifdef ARCH_WIN
	retcode = closesocket(client_s);
	WSACleanup();
#else
	retcode = close(client_s);
#endif
}

void UDPClockMaster::process(const ProcessArgs& args) {
	// Process RESET signal
	if (inputs[_RESET_INPUT].getVoltage() > 0.0) {
		if (reset_was0) {
			reset_was0 = false;
			// Assign a message to buffer out_buf
			strcpy(out_buf, "RESET");
			retcode = sendto(client_s, out_buf, (strlen(out_buf) + 1), 0,
				(struct sockaddr*) & server_addr, sizeof(server_addr));
			if (retcode <= 0) {
				lights[_STATUS_LIGHT].setBrightness(1.0);
			}
		}
	} else {
		reset_was0 = true;
	}

	// Process CLOCK signal
	if (inputs[_CLOCK_INPUT].getVoltage() > 0.0) {
		if (clock_was0) {
			clock_was0 = false;
			// Assign a message to buffer out_buf
			strcpy(out_buf, "CLOCK");
			retcode = sendto(client_s, out_buf, (strlen(out_buf) + 1), 0,
				(struct sockaddr*) & server_addr, sizeof(server_addr));
			if (retcode <= 0) {
				lights[_STATUS_LIGHT].setBrightness(1.0);
			}
		}
	} else {
		clock_was0 = true;
	}
}

struct UDPClockMasterWidget : ModuleWidget {
	UDPClockMasterWidget(UDPClockMaster* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/UDPClockMaster.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Lights
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(5.070, 86.116)), module, UDPClockMaster::_STATUS_LIGHT));

		// Ports
		addInput(createInputCentered<_Port>(mm2px(Vec(5.011, 95.772)), module, UDPClockMaster::_RESET_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.011, 106.383)), module, UDPClockMaster::_CLOCK_INPUT));
	}
};

Model* modelUDPClockMaster = createModel<UDPClockMaster, UDPClockMasterWidget>("UDPClockMaster");