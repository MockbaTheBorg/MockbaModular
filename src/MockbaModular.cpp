#include "plugin.hpp"
#include "MockbaModular.hpp"

const std::string BACK("res/Empty_light.svg");

void saveBack(const std::string& Back) {
	json_t* settingsJ = json_object();
	json_object_set_new(settingsJ, "Background", json_string(Back.c_str()));
	std::string settingsFilename = asset::user("MockbaModular.json");
	FILE* file = fopen(settingsFilename.c_str(), "w");
	if (file) {
		json_dumpf(settingsJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
		fclose(file);
	}
	json_decref(settingsJ);
}

std::string loadBack() {
	std::string ret;
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

float randomFloat() {
	return (float)rand() / (float)RAND_MAX;
}

float detune() {
	return (randomFloat() - 0.5f) * DETUNE_RNG;
}

float_4 xFade(float_4 s1, float_4 s2, float mix) {
	return(s1 * (1.f - mix) + s2 * mix);
}

float_4 mix2(float_4 s1, float_4 m1, float_4 s2, float_4 m2) {
	return((s1 * m1 + s2 * m2) / simd::fmax(1, m1 + m2));
}

float_4 mix3(float_4 s1, float_4 m1, float_4 s2, float_4 m2, float_4 s3, float_4 m3) {
	return((s1 * m1 + s2 * m2 + s3 * m3) / simd::fmax(1, m1 + m2 + m3));
}

float_4 mix4(float_4 s1, float_4 m1, float_4 s2, float_4 m2, float_4 s3, float_4 m3, float_4 s4, float_4 m4) {
	return((s1 * m1 + s2 * m2 + s3 * m3 + s4 * m4) / simd::fmax(1, m1 + m2 + m3 + m4));
}

float_4 mix5(float_4 s1, float_4 m1, float_4 s2, float_4 m2, float_4 s3, float_4 m3, float_4 s4, float_4 m4, float_4 s5, float_4 m5) {
	return((s1 * m1 + s2 * m2 + s3 * m3 + s4 * m4 + s5 * m5) / simd::fmax(1, m1 + m2 + m3 + m4 + m5));
}
