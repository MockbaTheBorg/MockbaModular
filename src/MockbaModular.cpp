#include "plugin.hpp"
#include "MockbaModular.hpp"

#define BACK "res/Empty_light.svg"

void saveBack(const char* Back) {
	json_t* settingsJ = json_object();
	json_object_set_new(settingsJ, "Background", json_string(Back));
	std::string settingsFilename = asset::user("MockbaModular.json");
	FILE* file = fopen(settingsFilename.c_str(), "w");
	if (file) {
		json_dumpf(settingsJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
		fclose(file);
	}
	json_decref(settingsJ);
}

const char* loadBack() {
	const char* ret = BACK;
	std::string settingsFilename = asset::user("MockbaModular.json");
	FILE* file = fopen(settingsFilename.c_str(), "r");
	if (!file) {
		saveBack(BACK);
		return BACK;
	}
	json_error_t error;
	json_t* settingsJ = json_loadf(file, 0, &error);
	if (!settingsJ) {
		// invalid setting json file
		fclose(file);
		saveBack(BACK);
		return BACK;
	}
	json_t* BackJ = json_object_get(settingsJ, "Background");
	if (BackJ) {
		ret = json_string_value(BackJ);
	} else {
		ret = BACK;
		saveBack(BACK);
	}

	fclose(file);
	json_decref(settingsJ);
	return ret;
}