// MockbaModular plugins for VCV Rack by Mockba the Borg

#include <cstdio>
#include <cstring>
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

std::string loadBack(int choice) {
	std::string ret;
	std::string settingsFilename = asset::user("MockbaModular.json");
	if (choice != -1) {
		switch (choice) {
		case 0:
			ret = "res/Empty_light.svg";
			break;
		case 1:
			ret = "res/Empty_dark.svg";
			break;
		default:
			ret = "res/Empty_gray.svg";
		}
	} else {
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
		}

		fclose(file);
		json_decref(settingsJ);
	}
	saveBack(ret);
	return ret;
}

float randomFloat() {
	return (float)rand() / (float)RAND_MAX;
}

float detune() {
	return (randomFloat() - 0.5f) * DETUNE_RNG;
}

float_4 detune4() {
	float_4 f;
	for (int i = 0; i < 4; ++i)
		f[i] = detune();
	return f;
}

Vec ISPos(float x, float y) {
	return mm2px(Vec(x, 128.5f - y));
}

_AlertClip loadAlertClip(const std::string& path) {
	_AlertClip out;
	FILE* f = fopen(path.c_str(), "rb");
	if (!f)
		return out;
	char id[4];
	uint32_t chunkSize32;
	if (fread(id, 1, 4, f) != 4 || memcmp(id, "RIFF", 4) != 0) { fclose(f); return out; }
	fseek(f, 4, SEEK_CUR); // riff size
	if (fread(id, 1, 4, f) != 4 || memcmp(id, "WAVE", 4) != 0) { fclose(f); return out; }

	uint16_t numChannels = 1, bitsPerSample = 16;
	uint32_t sampleRate = 44100;
	std::vector<int16_t> pcm;
	while (fread(id, 1, 4, f) == 4 && fread(&chunkSize32, 4, 1, f) == 1) {
		if (memcmp(id, "fmt ", 4) == 0) {
			uint16_t audioFormat;
			if (fread(&audioFormat, 2, 1, f) != 1) break;
			if (fread(&numChannels, 2, 1, f) != 1) break;
			if (fread(&sampleRate, 4, 1, f) != 1) break;
			fseek(f, 6, SEEK_CUR);
			if (fread(&bitsPerSample, 2, 1, f) != 1) break;
			long extra = (long)chunkSize32 - 16;
			if (extra > 0) fseek(f, extra, SEEK_CUR);
		}
		else if (memcmp(id, "data", 4) == 0 && bitsPerSample == 16) {
			size_t n = chunkSize32 / 2;
			pcm.resize(n);
			if (fread(pcm.data(), 2, n, f) != n) pcm.clear();
		}
		else {
			fseek(f, chunkSize32, SEEK_CUR);
		}
		if (chunkSize32 & 1) fseek(f, 1, SEEK_CUR);
	}
	fclose(f);
	if (pcm.empty() || numChannels < 1)
		return out;
	size_t frames = pcm.size() / numChannels;
	out.samples.resize(frames);
	for (size_t i = 0; i < frames; i++)
		out.samples[i] = pcm[i * numChannels] / 32768.f;
	out.sampleRate = (float)sampleRate;
	out.ok = true;
	return out;
}

// One-shot, native-pitch playback - a spoken message, not a note, so no V/Oct tracking.
// gateConnected=false free-runs once on the first call, matching how ungated one-shot
// transients already behave elsewhere in these plugins.
float stepAlertVoice(_AlertVoice& v, const _AlertClip& clip, bool gateConnected, bool gate, float engineSampleRate) {
	if (gateConnected) {
		if (gate && !v.gatePrev) {
			v.readPos = 0.f;
			v.playing = true;
		}
		v.gatePrev = gate;
	}
	else if (!v.started) {
		v.readPos = 0.f;
		v.playing = true;
		v.started = true;
	}
	float out = 0.f;
	if (v.playing && clip.ok && !clip.samples.empty()) {
		int len = (int)clip.samples.size();
		int idx0 = (int)v.readPos;
		if (idx0 >= len - 1) {
			v.playing = false;
		}
		else {
			float frac = v.readPos - idx0;
			out = clip.samples[idx0] * (1.f - frac) + clip.samples[idx0 + 1] * frac;
			v.readPos += clip.sampleRate / engineSampleRate;
		}
	}
	return out;
}

float_4 mix2(float_4 s1, float_4 m1, float_4 s2, float_4 m2) {
	return(s1 * m1 + s2 * m2);
}

float_4 mix3(float_4 s1, float_4 m1, float_4 s2, float_4 m2, float_4 s3, float_4 m3) {
	return(s1 * m1 + s2 * m2 + s3 * m3);
}

float_4 mix4(float_4 s1, float_4 m1, float_4 s2, float_4 m2, float_4 s3, float_4 m3, float_4 s4, float_4 m4) {
	return(s1 * m1 + s2 * m2 + s3 * m3 + s4 * m4);
}

float_4 mix5(float_4 s1, float_4 m1, float_4 s2, float_4 m2, float_4 s3, float_4 m3, float_4 s4, float_4 m4, float_4 s5, float_4 m5) {
	return(s1 * m1 + s2 * m2 + s3 * m3 + s4 * m4 + s5 * m5);
}