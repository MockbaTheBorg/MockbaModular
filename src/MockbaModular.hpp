// MockbaModular plugins for VCV Rack by Mockba the Borg

#ifndef MOCKBAMODULAR_HPP
#define MOCKBAMODULAR_HPP

using namespace simd;

#define DETUNE_RNG 0.002f	// Osc spread range
#define EXP_FACTOR -4.f		// Bulge factor for the Exponential Curve
#define GATE_VOLTAGE 10.0f	// Gate voltage level
#define GATE_THRESHOLD 1.0f	// Gate threshold level

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

#ifndef M_PI
	#define M_PI	3.14159265358979323846f
#endif
#ifndef M_PI_2
	#define M_PI_2	1.57079632679489661923f
#endif
#ifndef M_2_PI
	#define M_2_PI	0.63661977236758134307f
#endif
#ifndef M_2PI
	#define M_2PI	6.28318530717958647692f
#endif
#ifndef M_E
	#define M_E		2.71828182845904523536f
#endif

#define BGCOLOR loadBack(-1)

// Base Functions
void saveBack(const std::string& Back);
std::string loadBack(int bgSel);
float randomFloat();
float detune();
float_4 detune4();
Vec ISPos(float x, float y);

// Shared "no content loaded" spoken alert clip (res/noContent.wav) - plugin-shipped, mono
// 16-bit only, so no need for a fuller multi-bit-depth WAV loader. Used by any sample-player
// module (JDOscah, LAOscah, ...) that needs to tell the user its folder/card came up empty.
struct _AlertClip {
	std::vector<float> samples;
	float sampleRate = 44100.f;
	bool ok = false;
};

struct _AlertVoice {
	float readPos = 0.f;
	bool playing = false;
	bool started = false;
	bool gatePrev = false;
};

_AlertClip loadAlertClip(const std::string& path);
float stepAlertVoice(_AlertVoice& v, const _AlertClip& clip, bool gateConnected, bool gate, float engineSampleRate);

// simd mixing functions
float_4 mix2(float_4 s1, float_4 m1, float_4 s2, float_4 m2);
float_4 mix3(float_4 s1, float_4 m1, float_4 s2, float_4 m2, float_4 s3, float_4 m3);
float_4 mix4(float_4 s1, float_4 m1, float_4 s2, float_4 m2, float_4 s3, float_4 m3, float_4 s4, float_4 m4);
float_4 mix5(float_4 s1, float_4 m1, float_4 s2, float_4 m2, float_4 s3, float_4 m3, float_4 s4, float_4 m4, float_4 s5, float_4 m5);

// Exponential curve (Maug version)
template <typename T>
T expCurve(T x, float f) {
	return (3 + x * ((-13 - f) + 5 * x)) / (3 + (2 + f) * x);
}

// Cosine/Sine Approximation
template <typename T>
inline T mmCos(T x) noexcept {
	x *= 0.159154943092;
	x -= T(.25) + simd::floor(x + T(.25));
	x *= T(16.) * (simd::fabs(x) - T(.5));
#if EXTRA_PRECISION
	x += T(.225) * x * (simd::fabs(x) - T(1.));
#endif
	return x;
}
template <typename T>
inline T mmSin(T x) noexcept {
	x -= M_PI_2;
	x *= 0.159154943092;
	x -= T(.25) + simd::floor(x + T(.25));
	x *= T(16.) * (simd::fabs(x) - T(.5));
#if EXTRA_PRECISION
	x += T(.225) * x * (simd::fabs(x) - T(1.));
#endif
	return x;
}

// Linear to Log pot
// 2.0 = 0.25 at half turn
// 3.0 = 0.2 at half turn
// 6.0 = 0.125 at half turn
// 8.0 = 0.1 at half turn
#define LOGPOT_PARAM 6.f
template <typename T>
inline T logPot(T lin) {
	return(lin / (1 + (1 - lin) * LOGPOT_PARAM));
}

// Minimal MM Oscillator (No Antialias)
struct _MMOsc {
	int wave = 0;
	float_4 shape = 0.f;
	float_4 freq = 0.f;
	float_4 detuneMap = 0.f;
	float_4 phase = 0.f;
	float_4 outValue = 0.f;

	void setWave(float waveV) {
		wave = waveV;
	}

	void setShape(float_4 shapeV) {
		shape = simd::clamp(shapeV, 0.01f, 0.99f);
	}

	void setPitch(float_4 pitchV, float_4 spreadFlag) {
		freq = dsp::FREQ_C4 * dsp::approxExp2_taylor5(pitchV + 30) / 1073741824 * (1 + detuneMap * spreadFlag);
	}

	void init() {
		for (int i = 0; i < 4; i++)
			detuneMap[i] = detune();
	}

	virtual void process(float_4 delta) {
		// Calculate phase
		float_4 deltaPhase = simd::clamp(freq * delta, 1e-6f, 0.35f);
		phase += deltaPhase;
		phase -= simd::floor(phase);
		// Process oscillator step
		outValue = oscStep(phase, shape, wave);
	}

	// Override this to define the oscillator waveform - sinewave by default
	virtual float_4 oscStep(float_4 phase, float_4 shape, int wave) {
		return mmSin(phase * M_2PI);
	}

	float_4 _Out() {
		return outValue;
	}
};

// Maug Oscillator
struct _MaugOsc {
	// For optimizing in serial code
	int channels = 0;

	int wave = 0;

	float_4 phase = 0.f;
	float_4 freq;
	float_4 detuneMap = 0.f;
	float_4 pulseWidth = 0.5f;

	dsp::TRCFilter<float_4> sqrFilter;

	dsp::MinBlepGenerator<16, 16, float_4> triMinBlep;
	dsp::MinBlepGenerator<16, 16, float_4> sharkMinBlep;
	dsp::MinBlepGenerator<16, 16, float_4> sawMinBlep;
	dsp::MinBlepGenerator<16, 16, float_4> sqrMinBlep;

	float_4 triValue = 0.f;
	float_4 sharkValue = 0.f;
	float_4 sawValue = 0.f;
	float_4 sqrValue = 0.f;
	float_4 invsawValue = 0.f;

	void init() {
		for (int i = 0; i < 4; i++)
			detuneMap[i] = detune();
	}

	void setPitch(float_4 pitchV, float_4 spreadFlag) {
		freq = dsp::FREQ_C4 * dsp::approxExp2_taylor5(pitchV + 30) / 1073741824 * (1 + detuneMap * spreadFlag);
	}

	void setPulseWidth(float_4 pulseWidth) {
		const float pwMin = 0.01f;
		this->pulseWidth = simd::clamp(pulseWidth, pwMin, 1.f - pwMin);
	}

	void setWave(float waveV) {
		wave = waveV;
		switch (wave) {
		case 3:
			pulseWidth = 0.48f;
			break;
		case 4:
			pulseWidth = 0.29f;
			break;
		case 5:
			pulseWidth = 0.17f;
			break;
		default:
			pulseWidth = 0.5f; // was missing '=', value got discarded
		}
	}

	void process(float deltaTime) {
		// Advance phase
		float_4 deltaPhase = simd::clamp(freq * deltaTime, 1e-6f, 0.35f);
		phase += deltaPhase;
		phase -= simd::floor(phase);

		// Only the active wave needs its discontinuity check and generator run, saves work for single-wave modules
		bool needSqr = (wave == 3 || wave == 4 || wave == 5);
		bool needSaw = (wave == 2 || wave == 6);
		bool needShark = (wave == 1);

		if (needSqr) {
			// Jump sqr when crossing 0, or 1 if backwards
			float_4 wrapCrossing = -(phase - deltaPhase) / deltaPhase;
			int wrapMask = simd::movemask((0 < wrapCrossing) & (wrapCrossing <= 1.f));
			if (wrapMask) {
				for (int i = 0; i < channels; i++) {
					if (wrapMask & (1 << i)) {
						float_4 mask = simd::movemaskInverse<float_4>(1 << i);
						float p = wrapCrossing[i] - 1.f;
						float_4 x = mask & 2.f;
						sqrMinBlep.insertDiscontinuity(p, x);
					}
				}
			}

			// Jump sqr when crossing `pulseWidth`
			float_4 pulseCrossing = (pulseWidth - (phase - deltaPhase)) / deltaPhase;
			int pulseMask = simd::movemask((0 < pulseCrossing) & (pulseCrossing <= 1.f));
			if (pulseMask) {
				for (int i = 0; i < channels; i++) {
					if (pulseMask & (1 << i)) {
						float_4 mask = simd::movemaskInverse<float_4>(1 << i);
						float p = pulseCrossing[i] - 1.f;
						float_4 x = mask & -2.f;
						sqrMinBlep.insertDiscontinuity(p, x);
					}
				}
			}
		}

		if (needSaw) {
			// Jump saw when crossing 0.5
			float_4 halfCrossing = (0.5f - (phase - deltaPhase)) / deltaPhase;
			int halfMask = simd::movemask((0 < halfCrossing) & (halfCrossing <= 1.f));
			if (halfMask) {
				for (int i = 0; i < channels; i++) {
					if (halfMask & (1 << i)) {
						float_4 mask = simd::movemaskInverse<float_4>(1 << i);
						float p = halfCrossing[i] - 1.f;
						float_4 x = mask & -2.f;
						sawMinBlep.insertDiscontinuity(p, x);
					}
				}
			}
		}

		if (needShark) {
			// Shark jumps by +0.5 when crossing phase 0.5, uncorrected until now
			float_4 sharkCrossing = (0.5f - (phase - deltaPhase)) / deltaPhase;
			int sharkMask = simd::movemask((0 < sharkCrossing) & (sharkCrossing <= 1.f));
			if (sharkMask) {
				for (int i = 0; i < channels; i++) {
					if (sharkMask & (1 << i)) {
						float_4 mask = simd::movemaskInverse<float_4>(1 << i);
						float p = sharkCrossing[i] - 1.f;
						float_4 x = mask & 0.5f;
						sharkMinBlep.insertDiscontinuity(p, x);
					}
				}
			}
		}

		switch (wave) {
		case 0:
			triValue = tri(phase);
			triValue += triMinBlep.process();
			break;
		case 1:
			sharkValue = shark(phase);
			sharkValue += sharkMinBlep.process();
			break;
		case 2:
			sawValue = saw(phase);
			sawValue += sawMinBlep.process();
			break;
		case 3:
		case 4:
		case 5:
			sqrValue = sqr(phase);
			sqrValue += sqrMinBlep.process();
			sqrFilter.setCutoffFreq(20.f * deltaTime);
			sqrFilter.process(sqrValue);
			sqrValue = sqrFilter.highpass() * 0.95f;
			break;
		case 6:
			sawValue = saw(phase);
			sawValue += sawMinBlep.process();
			invsawValue = -sawValue;
			break;
		default:
			break;
		}
	}

	float_4 tri(float_4 phase) {
		float_4 v;
		float_4 x = phase + 0.25f;
		x -= simd::trunc(x);
		float_4 halfX = (x >= 0.5f);
		x *= 2;
		x -= simd::trunc(x);
		v = expCurve(x, EXP_FACTOR) * simd::ifelse(halfX, 1.f, -1.f);
		return v;
	}
	float_4 tri() {
		return triValue;
	}

	float_4 shark(float_4 phase) {
		float_4 a = simd::fmod(3.f * phase, 1.5f) - 1.f;
		float_4 b = 1.f - simd::fmod(4.f * phase, 2.f);
		float_4 c = (b * b - 1.f) / 7.f;
		float_4 v = simd::ifelse(phase < 0.5f, a - c, b + c);
		return v;
	}
	float_4 shark() {
		return sharkValue;
	}

	float_4 saw(float_4 phase) {
		float_4 v;
		float_4 x = phase + 0.5f;
		x -= simd::trunc(x);
		v = -expCurve(x, EXP_FACTOR);
		return v;
	}
	float_4 saw() {
		return sawValue;
	}

	float_4 sqr(float_4 phase) {
		float_4 v = simd::ifelse(phase < pulseWidth, 1.f, -1.f);
		return v;
	}
	float_4 sqr() {
		return sqrValue;
	}

	float_4 invsaw() {
		return invsawValue;
	}

	float_4 _Out() {
		float_4 result = 0.f;
		switch (wave) {
		case 0:
			result = triValue;
			break;
		case 1:
			result = sharkValue;
			break;
		case 2:
			result = sawValue;
			break;
		case 3:
			result = sqrValue;
			break;
		case 4:
			result = sqrValue - 0.4f;
			break;
		case 5:
			result = sqrValue - 0.6f;
			break;
		case 6:
			result = invsawValue;
			break;
		default:
			result = 0.f;
		}
		return result;
	}
};

// Maug LFO
struct _LFO {
    float phase = 0.f;
    float outTriangle = 0.f;
    float outSquare = 0.f;
    float freq = 0.f;

    void process(float delta, float speed) {
        freq = 0.05f + 199.95f * speed;
        float deltaPhase = simd::clamp(freq * delta, 1e-6f, 0.35f);
        phase += deltaPhase;
        
        // More efficient phase wrapping
        if (phase >= 1.f) phase -= 1.f;
        
        // Optimized triangle wave generation
        float scaledPhase = phase * 4.f;
        if (scaledPhase <= 2.f) {
            outTriangle = scaledPhase - 1.f;  // Rising edge: -1 to 1
        } else {
            outTriangle = 3.f - scaledPhase;  // Falling edge: 1 to -1
        }
        
        // Square wave from triangle sign
        outSquare = (outTriangle >= 0.f) ? 1.f : -1.f;
    }

    float _Triangle() {
        return outTriangle;
    }

    float _Square() {
        return outSquare;
    }
};

// Maug Filter
template <typename T>
struct _Filter {
	T tk = 0.f;
	T tp = 0.f;
	T tr = 0.f;

	T x = 0.f;
	T y1 = 0.f;
	T y2 = 0.f;
	T y3 = 0.f;
	T y4 = 0.f;

	T oldx = 0.f;
	T oldy1 = 0.f;
	T oldy2 = 0.f;
	T oldy3 = 0.f;

	int ftype = 0;
	T cutoff = 0.f;
	T res = 0.f;
	T outgain = 0.f;

	T outValue = 0.f;

	void init() {
		outgain = 1.f;
	}

	void setType(int ftypeV) {
		ftype = ftypeV;
	}

	void setCutoff(T cutoffV) {
		cutoff = cutoffV;
	}

	void setRes(T resV) {
		res = simd::clamp(resV, 0.f, 1.f); // headroom above the knob's max for CV-driven self-osc, but bounded
	}

	void setGain(T outgainV) {
		outgain = outgainV;
	}

	void process(T in, T rate) {
		// Recompute target coefficients every sample so CV modulation isn't quantized to a block
		T f = simd::clamp(2.f * cutoff / rate, 0.f, 0.99f); // keep in range the tgt_k/tgt_p approximation is valid for
		T tgt_k = 3.6f * f - 1.6f * f * f - 1.f;
		T tgt_p = (tgt_k + 1.f) * 0.5f;
		T scale = simd::exp((1.f - tgt_p) * 1.386249f);  // More efficient than pow
		T tgt_r = res * scale;

		// One-pole smoothing towards the target avoids zipper noise on knob changes
		const float smoothTime = 0.001f; // 1 ms
		T smooth = 1.f - simd::exp(-1.f / (smoothTime * rate));
		tk += (tgt_k - tk) * smooth;
		tp += (tgt_p - tp) * smooth;
		tr += (tgt_r - tr) * smooth;

		// Filter processing
		T x = in - tr * y4 + 1e-20f; // tiny DC bias keeps y1..y4 out of denormal range on silence
		y1 = x * tp + oldx * tp - tk * y1;
		y2 = y1 * tp + oldy1 * tp - tk * y2;
		y3 = y2 * tp + oldy2 * tp - tk * y3;
		y4 = y3 * tp + oldy3 * tp - tk * y4;

		oldx = x;
		oldy1 = y1;
		oldy2 = y2;
		oldy3 = y3;

		switch (ftype) {
		case 0:
			outValue = y4 * outgain; break;
		case 1:
			outValue = (in - y4) * outgain; break;
		default:
			outValue = 6.f * (y3 - y4) * outgain;
		}
	}

	T _Out() {
		return outValue;
	}
};

struct _DCBlock {
	float_4 xm1 = 0.f;
	float_4 ym1 = 0.f;

	float_4 process(float_4 x) {
		float_4 y = x - xm1 + 0.995f * ym1; xm1 = x; ym1 = y;
		return y;
	}
};

struct _Glider {
	float_4 freq = 0.f;

	float_4 process(float_4 newFreq, float glide, float sr) {
		float_4 diff = newFreq - freq;
		float_4 step = diff / (1.f + glide * (sr - 1.f));
		freq += step;
		return freq;
	}
};

struct _ModelVFilter {
	int type = 0;

	float_4 cutoff = 1.f;
	float_4 resonance = 0.f;

	float_4 stage[4];
	float_4 delay[4];
	float_4 p;
	float_4 k;
	float_4 t1;
	float_4 t2;

	void init() {
		for (int i = 0; i < 4; i++) {
			stage[i] = 0.f;
			delay[i] = 0.f;
		}
		setType(0.f);
		setCutoff(1.f);
		setResonance(0.f);
	}

	virtual float_4 process(float_4 input) {
		float_4 x = input - resonance * stage[3];

		// Four cascaded one-pole filters (bilinear transform)
		stage[0] = x * p + delay[0] * p - k * stage[0];
		stage[1] = stage[0] * p + delay[1] * p - k * stage[1];
		stage[2] = stage[1] * p + delay[2] * p - k * stage[2];
		stage[3] = stage[2] * p + delay[3] * p - k * stage[3];

		// Clipping band-limited sigmoid, only valid/stable up to |x|~1.4; hard-clamp beyond that
		// so self-oscillation stays bounded instead of the cubic term making it worse
		float_4 s3 = simd::clamp(stage[3], -1.4f, 1.4f);
		stage[3] = simd::clamp(stage[3] - (s3 * s3 * s3) / 6.0f, -4.f, 4.f);

		delay[0] = x;
		delay[1] = stage[0];
		delay[2] = stage[1];
		delay[3] = stage[2];

		switch (type) {
		case 0:
			input = stage[3]; break;
		case 1:
			input -= stage[3]; break;
		default:
			input = 3.f * (stage[2] - stage[3]);
		}

		return input;
	}

	virtual void setType(int t) {
		type = t;
	}

	virtual void setResonance(float r) {
		// 1.4x headroom above the raw formula so max resonance can cross the self-oscillation threshold
		resonance = 1.4f * r * (t2 + 6.0f * t1) / (t2 - 6.0f * t1);
	}

	virtual void setCutoff(float_4 c) {
		cutoff = c;

		p = cutoff * (1.8f - 0.8f * cutoff);
		k = 2.0f * mmSin(cutoff * M_PI_2) - 1.0f;
		t1 = (1.0f - p) * 1.386249f;
		t2 = 12.0f + t1 * t1 * t1;
	}
};

struct _MaugLP_A {
	float type;
	float_4 cutoff;
	float resonance;
	float_4 output = { 0, 0, 0, 0 };

	float_4 in1 = { 0, 0, 0, 0 };
	float_4 in2 = { 0, 0, 0, 0 };
	float_4 in3 = { 0, 0, 0, 0 };
	float_4 in4 = { 0, 0, 0, 0 };

	float_4 out1 = { 0, 0, 0, 0 };
	float_4 out2 = { 0, 0, 0, 0 };
	float_4 out3 = { 0, 0, 0, 0 };
	float_4 out4 = { 0, 0, 0, 0 };

	void init() {
		type = 0.f;
		resonance = 0.f;
		cutoff = 1.f;
		output = 0;
	}

	void setType(float t) {
		type = t;
	}

	void setCutoff(float_4 c) {
		cutoff = c * .99f + .01f;
	}

	void setResonance(float r) {
		resonance = r * 4.f;
	}

	float_4 process(float_4 input) {
		float_4 f = cutoff * 1.16;
		float_4 fb = resonance * (1.0 - 0.15 * f * f);
		input -= out4 * fb;
		input *= 0.35013 * (f * f) * (f * f);
		out1 = input + 0.3 * in1 + (1 - f) * out1; // Pole 1
		in1 = input;
		out2 = out1 + 0.3 * in2 + (1 - f) * out2;  // Pole 2
		in2 = out1;
		out3 = out2 + 0.3 * in3 + (1 - f) * out3;  // Pole 3
		in3 = out2;
		out4 = out3 + 0.3 * in4 + (1 - f) * out4;  // Pole 4
		in4 = out3;

		if (type == 0.f) {
			input = out4;
		} else if (type == 1.f) {
			input -= out4;
		} else {
			input = 3.f * (out3 - out4);
		}
		return input;
	}
};

// Maug Oscah Oscillator waves
template <typename T>
struct _MaugWave {
	T Triangle(T phase) {
		T a = 2.f * phase - 1.f;
		T b = simd::sgn(a);
		T c = simd::fmod(4.f * phase, 2.f) - 1.f;
		T d = (c * c - 1.f) / 7.f;
		T v = b * d - b * c;
		return v;
	}

	T Shark(T phase) {
		T a = simd::fmod(3.f * phase, 1.5f) - 1.f;
		T b = 1.f - simd::fmod(4.f * phase, 2.f);
		T c = (b * b - 1.f) / 7.f;
		T v = simd::ifelse(phase < 0.5f, a - c, b + c);
		return v;
	}

	T Saw(T phase) {
		T a = 2.f * phase - 1.f;
		T b = (a * a - 1.f) / 7.f;
		T v = a - b;
		return v;
	}

	T Square(T phase, T size) {
		T a = phase - size;
		T b = simd::sgn(a);
		T c = mmCos(phase - a / 2.f);
		T v = b * c;
		return v;
	}

	T InvSaw(T phase) {
		T a = 1.f - 2.f * phase;
		T b = (a * a - 1.f) / 7.f;
		T v = a + b;
		return v;
	}
};

// CZ Oscah Oscillator Waves
template <typename T>
struct _CZWave {
	T Saw(T phase, T shape) {
		T a = 0.5f - shape / 2.f;
		T b = phase * ((0.5f - a) / a);
		T c = (-phase + 1.f) * ((0.5f - a) / (1.f - a));
		T d = phase + simd::fmin(b, c);
		T v = mmCos(d * M_2PI);
		return v;
	}

	T Square(T phase, T shape) {
		T a = simd::sgn(0.5f - phase);
		T b = simd::fmod(2.f * phase, 1.f);
		T c = (1.f - b) * (shape / (1.f - shape));
		T d = 0.5f * (b - simd::fmin(b, c));
		T v = mmCos(d * M_2PI) * a;
		return v;
	}

	T Pulse(T phase, T shape) {
		T a = (1.f - phase) / (1.f - shape);
		T b = 1.f - a;
		T c = simd::fmax(b, 0.f);
		T d = simd::fmin(a, c);
		T v = mmCos(d * M_2PI);
		return v;
	}

	T DblSine(T phase, T shape) {
		T a = 0.5f - (shape * 0.5f);
		T b = phase * ((0.5f - a) / a);
		T c = (1.f - phase) * ((0.5f - a) / (1.f - a));
		T d = phase + simd::fmin(b, c);
		T v = mmCos(2.f * d * M_2PI);
		return v;
	}

	T SawPulse(T phase, T shape) {
		T a = -2.f * phase + 2.f;
		T b = (1.f - a) * (shape / (1.f - shape));
		T c = 0.5f * (a - simd::fmin(a, b));
		T d = simd::fmin(c, phase);
		T v = mmCos(d * M_2PI);
		return v;
	}

	T Reso1(T phase, T shape) {
		T a = 1.f - phase;
		T b = phase * (.0625 + shape) * 16.f;
		T v = mmCos(b * M_2PI) * a + phase;
		return v;
	}

	T Reso2(T phase, T shape) {
		T a = 2.f * phase;
		T b = 2.f - a;
		T c = simd::fmin(a, b);
		T d = simd::fmod(phase * (.0625 + shape) * 16.f, 1.f);
		T v = mmCos(d * M_2PI) * c + (1.f - c);
		return v;
	}

	T Reso3(T phase, T shape) {
		T a = 2.f - 2.f * phase;
		T b = simd::fmin(a, 1.f);
		T c = phase * (.0625 + shape) * 16.f;
		T v = mmCos(c * M_2PI) * b + (1 - b);
		return v;
	}
};

// Pink Noise
struct _PinkNoise {
	int frame = -1;
	float_4 values[8] = {};
	
	float_4 _Out() {
		int lastFrame = frame;
		frame++;
		if (frame >= 256)  // More readable than (1 << 8)
			frame = 0;
		int diff = lastFrame ^ frame;
		
		float_4 sum = values[0] + values[1] + values[2] + values[3] + 
					  values[4] + values[5] + values[6] + values[7];
		
		// Only update changed generators and adjust sum
		for (int i = 0; i < 8; i++) {
			if (diff & (1 << i)) {
				float_4 oldValue = values[i];
				values[i] = random::uniform() - 0.5f;
				sum += values[i] - oldValue;  // Update sum incrementally
			}
		}
		return sum * 0.125f;  // Division by 8, more precise scaling
	}
};

// White Noise
struct _WhiteNoise {
	float_4 _Out() {
		float_4 white = random::normal();
		return white / 2.f;
	}
};

// Simple ADSR
struct _ADSR {
	int32_4 state = 0;
	float_4 output = 0.f;
	float_4 attackRate = 0.f;
	float_4 decayRate = 0.f;
	float_4 releaseRate = 0.f;
	float_4 attackCoef = 0.f;
	float_4 decayCoef = 0.f;
	float_4 releaseCoef = 0.f;
	float_4 sustainLevel = 1.f;
	float_4 targetRatioA = 0.f;
	float_4 targetRatioDR = 0.f;
	float_4 attackBase = 0.f;
	float_4 decayBase = 0.f;
	float_4 releaseBase = 0.f;

	enum envState {
		env_idle = 0,
		env_attack,
		env_decay,
		env_sustain,
		env_release
	};

	void process() {
		for (int i = 0; i < 4; i++) {
			switch (state[i]) {
			case env_idle:		// 0
				break;
			case env_attack:	// 1
				output[i] = attackBase[i] + output[i] * attackCoef[i];
				if (output[i] >= 1.f) {
					output[i] = 1.f;
					state[i] = env_decay;
				}
				break;
			case env_decay:		// 2
				output[i] = decayBase[i] + output[i] * decayCoef[i];
				if (output[i] <= sustainLevel[i]) {
					output[i] = sustainLevel[i];
					state[i] = env_sustain;
				}
				break;
			case env_sustain:	// 3
				break;
			case env_release:	// 4
				output[i] = releaseBase[i] + output[i] * releaseCoef[i];
				if (output[i] <= 0.f) {
					output[i] = 0.f;
					state[i] = env_idle;
				}
			}
		}
	}

	void gate(float_4 gate) {
		for (int i = 0; i < 4; i++) {
			if (gate[i] <= 0.f) {
				if (state[i] != env_idle)
					state[i] = env_release;
			} else {
				if (state[i] == env_idle || state[i] == env_release)
					state[i] = env_attack;
			}
		}
	}

	void init() {
		setState(0.f);
		setAttackRate(0.f);
		setDecayRate(0.f);
		setReleaseRate(0.f);
		setSustainLevel(0.5f);
		setTargetRatioA(0.3f);
		setTargetRatioDR(0.0001f);
	}

	float_4 _Out() {
		return output;
	}

	float_4 calcCoef(float_4 rate, float_4 targetRatio) {
		float_4 result = simd::ifelse(rate <= 0.f, 0.f, exp(-log((1.f + targetRatio) / targetRatio) / rate));
		return result;
	}

	void setState(int32_4 stateV) {
		state = stateV;
	}

	int32_4 getState() {
		return state;
	}

	void setAttackRate(float_4 rate) {
		if (simd::movemask(rate == attackRate) == 0xF) return; // exp/log below only needed when the rate actually changes
		attackRate = rate;
		attackCoef = calcCoef(rate, targetRatioA);
		attackBase = (1.f + targetRatioA) * (1.f - attackCoef);
	}

	void setDecayRate(float_4 rate) {
		if (simd::movemask(rate == decayRate) == 0xF) return;
		decayRate = rate;
		decayCoef = calcCoef(rate, targetRatioDR);
		decayBase = (sustainLevel - targetRatioDR) * (1.f - decayCoef);
	}

	void setReleaseRate(float_4 rate) {
		if (simd::movemask(rate == releaseRate) == 0xF) return;
		releaseRate = rate;
		releaseCoef = calcCoef(rate, targetRatioDR);
		releaseBase = -targetRatioDR * (1.f - releaseCoef);
	}

	void setSustainLevel(float_4 level) {
		if (simd::movemask(level == sustainLevel) == 0xF) return;
		sustainLevel = level;
		decayBase = (sustainLevel - targetRatioDR) * (1.f - decayCoef);
	}

	void setTargetRatioA(float_4 targetRatio) {
		for (int i = 0; i < 4; i++) {
			if (targetRatio[i] < 0.000000001f)
				targetRatio[i] = 0.000000001f;  // -180 dB
		}
		if (simd::movemask(targetRatio == targetRatioA) == 0xF) return;
		targetRatioA = targetRatio;
		attackCoef = calcCoef(attackRate, targetRatioA);
		attackBase = (1.f + targetRatioA) * (1.f - attackCoef);
	}

	void setTargetRatioDR(float_4 targetRatio) {
		for (int i = 0; i < 4; i++) {
			if (targetRatio[i] < 0.000000001f)
				targetRatio[i] = 0.000000001f;  // -180 dB
		}
		if (simd::movemask(targetRatio == targetRatioDR) == 0xF) return;
		targetRatioDR = targetRatio;
		decayCoef = calcCoef(decayRate, targetRatioDR);
		releaseCoef = calcCoef(releaseRate, targetRatioDR);
		decayBase = (sustainLevel - targetRatioDR) * (1.f - decayCoef);
		releaseBase = -targetRatioDR * (1.f - releaseCoef);
	}
};

// Simple DAHDSR
struct _DAHDSR {
	int32_4 state = 0;
	float_4 output = 0.f;
	int32_4 delayCtr = 0;
	int32_4 delayTime = 0;
	float_4 attackRate = 0.f;
	int32_4 holdCtr = 0;
	int32_4 holdTime = 0;
	float_4 decayRate = 0.f;
	float_4 releaseRate = 0.f;
	float_4 attackCoef = 0.f;
	float_4 decayCoef = 0.f;
	float_4 releaseCoef = 0.f;
	float_4 sustainLevel = 1.f;
	float_4 targetRatioA = 0.f;
	float_4 targetRatioDR = 0.f;
	float_4 attackBase = 0.f;
	float_4 decayBase = 0.f;
	float_4 releaseBase = 0.f;

	enum envState {
		env_idle = 0,
		env_delay,
		env_attack,
		env_hold,
		env_decay,
		env_sustain,
		env_release
	};

	void process() {
		for (int i = 0; i < 4; i++) {
			switch (state[i]) {
			case env_idle:		// 0
				break;
			case env_delay:		// 1
				if (delayCtr[i] < delayTime[i]) {
					delayCtr[i]++;
				} else {
					delayCtr[i] = 0;
					state[i] = env_attack;
				}
				break;
			case env_attack:	// 2
				output[i] = attackBase[i] + output[i] * attackCoef[i];
				if (output[i] >= 1.f) {
					output[i] = 1.f;
					state[i] = env_hold;
				}
				break;
			case env_hold:		// 3
				if (holdCtr[i] < holdTime[i]) {
					holdCtr[i]++;
				} else {
					holdCtr[i] = 0;
					state[i] = env_decay;
				}
				break;
			case env_decay:		// 4
				output[i] = decayBase[i] + output[i] * decayCoef[i];
				if (output[i] <= sustainLevel[i]) {
					output[i] = sustainLevel[i];
					state[i] = env_sustain;
				}
				break;
			case env_sustain:	// 5
				break;
			case env_release:	// 6
				output[i] = releaseBase[i] + output[i] * releaseCoef[i];
				if (output[i] <= 0.f) {
					output[i] = 0.f;
					state[i] = env_idle;
				}
			}
		}
	}

	void gate(float_4 gate) {
		for (int i = 0; i < 4; i++) {
			if (gate[i] <= 0.f) {
				if (state[i] != env_idle)
					state[i] = env_release;
			} else {
				if (state[i] == env_idle || state[i] == env_release)
					state[i] = env_delay;
			}
		}
	}

	void init() {
		setState(0.f);
		setDelayTime(0);
		setAttackRate(0.f);
		setHoldTime(0);
		setDecayRate(0.f);
		setReleaseRate(0.f);
		setSustainLevel(0.5f);
		setTargetRatioA(0.3f);
		setTargetRatioDR(0.0001f);
	}

	float_4 _Out() {
		return output;
	}

	float_4 calcCoef(float_4 rate, float_4 targetRatio) {
		float_4 result = simd::ifelse(rate <= 0.f, 0.f, exp(-log((1.f + targetRatio) / targetRatio) / rate));
		return result;
	}

	void setState(int32_4 stateV) {
		state = stateV;
	}

	int32_4 getState() {
		return state;
	}

	void setDelayTime(float_4 time) {
		delayTime = time;
	}

	void setAttackRate(float_4 rate) {
		attackRate = rate;
		attackCoef = calcCoef(rate, targetRatioA);
		attackBase = (1.f + targetRatioA) * (1.f - attackCoef);
	}

	void setHoldTime(float_4 time) {
		holdTime = time;
	}

	void setDecayRate(float_4 rate) {
		decayRate = rate;
		decayCoef = calcCoef(rate, targetRatioDR);
		decayBase = (sustainLevel - targetRatioDR) * (1.f - decayCoef);
	}

	void setReleaseRate(float_4 rate) {
		releaseRate = rate;
		releaseCoef = calcCoef(rate, targetRatioDR);
		releaseBase = -targetRatioDR * (1.f - releaseCoef);
	}

	void setSustainLevel(float_4 level) {
		sustainLevel = level;
		decayBase = (sustainLevel - targetRatioDR) * (1.f - decayCoef);
	}

	void setTargetRatioA(float_4 targetRatio) {
		for (int i = 0; i < 4; i++) {
			if (targetRatio[i] < 0.000000001f)
				targetRatio[i] = 0.000000001f;  // -180 dB
		}
		targetRatioA = targetRatio;
		attackCoef = calcCoef(attackRate, targetRatioA);
		attackBase = (1.f + targetRatioA) * (1.f - attackCoef);
	}

	void setTargetRatioDR(float_4 targetRatio) {
		for (int i = 0; i < 4; i++) {
			if (targetRatio[i] < 0.000000001f)
				targetRatio[i] = 0.000000001f;  // -180 dB
		}
		targetRatioDR = targetRatio;
		decayCoef = calcCoef(decayRate, targetRatioDR);
		releaseCoef = calcCoef(releaseRate, targetRatioDR);
		decayBase = (sustainLevel - targetRatioDR) * (1.f - decayCoef);
		releaseBase = -targetRatioDR * (1.f - releaseCoef);
	}
};

// Custom buttons/tooltips

template<int NUM_MODES>
struct MMButtonQuantity : ParamQuantity {

	std::string modes[NUM_MODES];

	virtual int getModeEnumeration(void) { return 0; }
	virtual void setMode(int mode) {}

	float getDisplayValue() override {
		if (!module)
			return Quantity::getDisplayValue();
		return getModeEnumeration();
	}

	std::string getDisplayValueString() override {
		if (!module)
			return Quantity::getDisplayValueString();
		int mode = getDisplayValue();
		return modes[mode];
	}

	void setDisplayValueString(std::string s) override {
		if (!module)
			return;
		for (int i = 0; i < NUM_MODES; i++) {
			if (s == modes[i] || s == std::to_string(i + 1)) {
				setMode(i);
			}
		}

	}
};

struct _CZWaveNames : ParamQuantity {
	std::string getDisplayValueString() override {
		int v = getValue();
		std::string result;
		switch (v) {
		case 0:
			result = "Saw";
			break;
		case 1:
			result = "Square";
			break;
		case 2:
			result = "Pulse";
			break;
		case 3:
			result = "DblSine";
			break;
		case 4:
			result = "SawPulse";
			break;
		case 5:
			result = "Reso1";
			break;
		case 6:
			result = "Reso2";
			break;
		case 7:
			result = "Reso3";
			break;
		default:
			result = "???";
		}
		return result;
	}
};

struct _MaugWaveNames : ParamQuantity {
	std::string getDisplayValueString() override {
		int v = getValue();
		std::string result;
		switch (v) {
		case 0:
			result = "Triangle";
			break;
		case 1:
			result = "Shark";
			break;
		case 2:
			result = "Saw";
			break;
		case 3:
			result = "Square 48%";
			break;
		case 4:
			result = "Square 29%";
			break;
		case 5:
			result = "Square 17%";
			break;
		case 6:
			result = "InvSaw";
			break;
		default:
			result = "???";
		}
		return result;
	}
};

struct _MaugOsc3WaveNames : ParamQuantity {
	std::string getDisplayValueString() override {
		int v = getValue();
		std::string result;
		switch (v) {
		case 0:
			result = "Triangle";
			break;
		case 1:
			result = "InvSaw";
			break;
		case 2:
			result = "Saw";
			break;
		case 3:
			result = "Square 48%";
			break;
		case 4:
			result = "Square 29%";
			break;
		case 5:
			result = "Square 17%";
			break;
		default:
			result = "???";
		}
		return result;
	}
};

struct _ProtonWaveNames : ParamQuantity {
	std::string getDisplayValueString() override {
		int v = getValue();
		std::string result;
		switch (v) {
		case 0:
			result = "Sine";
			break;
		case 1:
			result = "Triangle";
			break;
		case 2:
			result = "Saw";
			break;
		case 3:
			result = "InvSaw";
			break;
		case 4:
			result = "Square";
			break;
		case 5:
			result = "PWM";
			break;
		default:
			result = "???";
		}
		return result;
	}
};

struct _OscRange : ParamQuantity {
	std::string getDisplayValueString() override {
		int v = getValue();
		std::string result;
		switch (v) {
		case -3:
			result = "Low";
			break;
		case -2:
			result = "32'";
			break;
		case -1:
			result = "16'";
			break;
		case 0:
			result = "8'";
			break;
		case 1:
			result = "4'";
			break;
		case 2:
			result = "2'";
			break;
		default:
			result = "???";
		}
		return result;
	}
};

struct _OnOff : ParamQuantity {
	std::string getDisplayValueString() override {
		int v = getValue();
		std::string result;
		switch (v) {
		case 0:
			result = "Off";
			break;
		case 1:
			result = "On";
			break;
		default:
			result = "???";
		}
		return result;
	}
};

struct _NoiseType : ParamQuantity {
	std::string getDisplayValueString() override {
		int v = getValue();
		std::string result;
		switch (v) {
		case 0:
			result = "Pink";
			break;
		case 1:
			result = "White";
			break;
		default:
			result = "???";
		}
		return result;
	}
};

struct _ModMixLeft : ParamQuantity {
	std::string getDisplayValueString() override {
		int v = getValue();
		std::string result;
		switch (v) {
		case 0:
			result = "Osc 3";
			break;
		case 1:
			result = "Filter";
			break;
		default:
			result = "???";
		}
		return result;
	}
};

struct _ModMixRight : ParamQuantity {
	std::string getDisplayValueString() override {
		int v = getValue();
		std::string result;
		switch (v) {
		case 0:
			result = "Noise";
			break;
		case 1:
			result = "LFO";
			break;
		default:
			result = "???";
		}
		return result;
	}
};

struct _LfoWave : ParamQuantity {
	std::string getDisplayValueString() override {
		int v = getValue();
		std::string result;
		switch (v) {
		case 0:
			result = "Triangle";
			break;
		case 1:
			result = "Square";
			break;
		default:
			result = "???";
		}
		return result;
	}
};

struct _FilterMode : ParamQuantity {
	std::string getDisplayValueString() override {
		int v = getValue();
		std::string result;
		switch (v) {
		case 0:
			result = "Low Pass";
			break;
		case 1:
			result = "High Pass";
			break;
		case 2:
			result = "Band Pass";
			break;
		default:
			result = "???";
		}
		return result;
	}
};

// Shared scaffolding for the 2-input Dual gate modules (AND/OR/NAND/NOR/XOR/XNOR)
// Only the boolean op and panel SVG differ between them.
enum class _GateOp2 { AND, OR, NAND, NOR, XOR, XNOR };

inline const char* _dualGate2Svg(_GateOp2 op) {
	switch (op) {
	case _GateOp2::AND:  return "res/DualAND.svg";
	case _GateOp2::OR:   return "res/DualOR.svg";
	case _GateOp2::NAND: return "res/DualNAND.svg";
	case _GateOp2::NOR:  return "res/DualNOR.svg";
	case _GateOp2::XOR:  return "res/DualXOR.svg";
	case _GateOp2::XNOR: return "res/DualXNOR.svg";
	}
	return "";
}

template <_GateOp2 Op>
struct _DualGate2 : Module {
	enum InputIds {
		_A1_INPUT,
		_B1_INPUT,
		_A2_INPUT,
		_B2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_Q1_OUTPUT,
		_Q2_OUTPUT,
		NUM_OUTPUTS
	};
	enum ParamIds {
		NUM_PARAMS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	_DualGate2() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configInput(_A1_INPUT, "A1");
		configInput(_B1_INPUT, "B1");
		configInput(_A2_INPUT, "A2");
		configInput(_B2_INPUT, "B2");
		configOutput(_Q1_OUTPUT, "Q1");
		configOutput(_Q2_OUTPUT, "Q2");
	}

	static bool eval(bool a, bool b) {
		switch (Op) {
		case _GateOp2::AND:  return a && b;
		case _GateOp2::OR:   return a || b;
		case _GateOp2::NAND: return !(a && b);
		case _GateOp2::NOR:  return !(a || b);
		case _GateOp2::XOR:  return a != b;
		case _GateOp2::XNOR: return a == b;
		}
		return false;
	}

	void process(const ProcessArgs& args) override {
		// Get inputs, treating disconnected inputs as false (0V)
		bool a1 = inputs[_A1_INPUT].isConnected() ?
			(inputs[_A1_INPUT].getVoltage() > GATE_THRESHOLD) : false;
		bool b1 = inputs[_B1_INPUT].isConnected() ?
			(inputs[_B1_INPUT].getVoltage() > GATE_THRESHOLD) : false;
		bool a2 = inputs[_A2_INPUT].isConnected() ?
			(inputs[_A2_INPUT].getVoltage() > GATE_THRESHOLD) : false;
		bool b2 = inputs[_B2_INPUT].isConnected() ?
			(inputs[_B2_INPUT].getVoltage() > GATE_THRESHOLD) : false;

		bool q1 = eval(a1, b1);
		bool q2 = eval(a2, b2);

		outputs[_Q1_OUTPUT].setVoltage(q1 ? GATE_VOLTAGE : 0.0f);
		outputs[_Q2_OUTPUT].setVoltage(q2 ? GATE_VOLTAGE : 0.0f);
	}
};

template <_GateOp2 Op>
struct _DualGate2Widget : ModuleWidget {
	_DualGate2Widget(_DualGate2<Op>* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, _dualGate2Svg(Op))));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 57.0)), module, _DualGate2<Op>::_A1_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, _DualGate2<Op>::_B1_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, _DualGate2<Op>::_A2_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, _DualGate2<Op>::_B2_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, _DualGate2<Op>::_Q1_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, _DualGate2<Op>::_Q2_OUTPUT));
	}
};

// Shared scaffolding for the single-wave Casio CZ oscillator modules.
// Only the waveform, panel SVG, and a couple of optional features (LFO mode, DC block) differ between them.
enum class _CZWaveOp { Saw, Square, Pulse, DblSine, SawPulse, Reso1, Reso2, Reso3 };

inline float_4 _czWaveEval(_CZWaveOp op, _CZWave<float_4>& osc, float_4 phase, float_4 shape) {
	switch (op) {
	case _CZWaveOp::Saw:      return osc.Saw(phase, shape);
	case _CZWaveOp::Square:   return osc.Square(phase, shape);
	case _CZWaveOp::Pulse:    return osc.Pulse(phase, shape);
	case _CZWaveOp::DblSine:  return osc.DblSine(phase, shape);
	case _CZWaveOp::SawPulse: return osc.SawPulse(phase, shape);
	case _CZWaveOp::Reso1:    return osc.Reso1(phase, shape);
	case _CZWaveOp::Reso2:    return osc.Reso2(phase, shape);
	case _CZWaveOp::Reso3:    return osc.Reso3(phase, shape);
	}
	return 0.f;
}

inline const char* _czWaveSvg(_CZWaveOp op) {
	switch (op) {
	case _CZWaveOp::Saw:      return "res/CZSaw.svg";
	case _CZWaveOp::Square:   return "res/CZSquare.svg";
	case _CZWaveOp::Pulse:    return "res/CZPulse.svg";
	case _CZWaveOp::DblSine:  return "res/CZDblSine.svg";
	case _CZWaveOp::SawPulse: return "res/CZSawPulse.svg";
	case _CZWaveOp::Reso1:    return "res/CZReso1.svg";
	case _CZWaveOp::Reso2:    return "res/CZReso2.svg";
	case _CZWaveOp::Reso3:    return "res/CZReso3.svg";
	}
	return "";
}

template <_CZWaveOp Op>
struct _CZOscVoice : _MMOsc {
	_CZWave<float_4> osc;

	float_4 oscStep(float_4 phase, float_4 shape, int wave) override {
		return _czWaveEval(Op, osc, phase, shape);
	}
};

// LFO-capable group: CZSaw, CZSquare, CZPulse (LFO on/off switch, no DC block)
template <_CZWaveOp Op>
struct _CZOscLFO : Module {
	enum ParamIds {
		_LFO_PARAM,
		_FREQ_PARAM,
		_FINE_PARAM,
		_SHAPE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_MODF_INPUT,
		_MODS_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_WAVE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	_CZOscVoice<Op> osc[4];

	_CZOscLFO() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam<_OnOff>(_LFO_PARAM, 0, 1, 0, "LFO Mode");
		configParam(_FREQ_PARAM, -54.f, 54.f, 0.f, "Frequency", " Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);
		configParam(_FINE_PARAM, -1.f, 1.f, 0.f, "Fine frequency / LFO Offset");
		configParam(_SHAPE_PARAM, 0.f, 1.f, 0.f, "Shape");
		configInput(_MODF_INPUT, "Frequency modulation");
		configInput(_MODS_INPUT, "Shape modulation");
		configOutput(_WAVE_OUTPUT, "Waveform");
	}

	void onAdd() override {
		for (int i = 0; i < 4; i++) osc[i].init(); // init all 4 banks, not just bank 0
	}

	void onReset() override {
		onAdd();
	}

	void process(const ProcessArgs& args) override {
		// Get the frequency parameters
		float freqParam = params[_FREQ_PARAM].getValue() / 12.f;
		// LFO mode
		if (params[_LFO_PARAM].getValue())
			freqParam = (freqParam * 2) - 5;
		freqParam += dsp::quadraticBipolar(params[_FINE_PARAM].getValue()) * 3.f / 12.f;
		// Get the shape parameter
		float shapeParam = params[_SHAPE_PARAM].getValue();
		// Iterate over each channel
		int channels = max(inputs[_MODF_INPUT].getChannels(), 1);
		for (int c = 0; c < channels; c += 4) {
			// Get the oscillator
			auto* oscillator = &osc[c / 4];
			// Set the pitch
			float_4 pitch = freqParam;
			pitch += inputs[_MODF_INPUT].template getVoltageSimd<float_4>(c);
			oscillator->setPitch(pitch, 1.f);
			// Set the shape
			float_4 shape = shapeParam;
			if (inputs[_MODS_INPUT].isConnected())
				shape *= inputs[_MODS_INPUT].template getVoltageSimd<float_4>(c) / 10.f;
			oscillator->setShape(shape);
			// Process and output
			oscillator->process(args.sampleTime);
			float_4 off = params[_LFO_PARAM].getValue() * params[_FINE_PARAM].getValue() * 5.f;
			outputs[_WAVE_OUTPUT].setVoltageSimd(5.f * oscillator->_Out() + off, c);
		}
		outputs[_WAVE_OUTPUT].setChannels(channels);
	}
};

template <_CZWaveOp Op>
struct _CZOscLFOWidget : ModuleWidget {
	_CZOscLFOWidget(_CZOscLFO<Op>* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, _czWaveSvg(Op))));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Hsw>(mm2px(Vec(5.1, 46.0)), module, _CZOscLFO<Op>::_LFO_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 57.0)), module, _CZOscLFO<Op>::_FREQ_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 68.0)), module, _CZOscLFO<Op>::_FINE_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 90.0)), module, _CZOscLFO<Op>::_SHAPE_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, _CZOscLFO<Op>::_MODF_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, _CZOscLFO<Op>::_MODS_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, _CZOscLFO<Op>::_WAVE_OUTPUT));
	}
};

// Plain group: CZDblSine, CZSawPulse (no LFO switch, no DC block)
template <_CZWaveOp Op>
struct _CZOscPlain : Module {
	enum ParamIds {
		_FREQ_PARAM,
		_FINE_PARAM,
		_SHAPE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_MODF_INPUT,
		_MODS_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_WAVE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	_CZOscVoice<Op> osc[4];

	_CZOscPlain() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_FREQ_PARAM, -54.f, 54.f, 0.f, "Frequency", " Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);
		configParam(_FINE_PARAM, -1.f, 1.f, 0.f, "Fine frequency");
		configParam(_SHAPE_PARAM, 0.f, 1.f, 0.f, "Shape");
		configInput(_MODF_INPUT, "Frequency modulation");
		configInput(_MODS_INPUT, "Shape modulation");
		configOutput(_WAVE_OUTPUT, "Waveform");
	}

	void onAdd() override {
		for (int i = 0; i < 4; i++) osc[i].init(); // init all 4 banks, not just bank 0
	}

	void onReset() override {
		onAdd();
	}

	void process(const ProcessArgs& args) override {
		// Get the frequency parameters
		float freqParam = params[_FREQ_PARAM].getValue() / 12.f;
		freqParam += dsp::quadraticBipolar(params[_FINE_PARAM].getValue()) * 3.f / 12.f;
		// Get the shape parameter
		float shapeParam = params[_SHAPE_PARAM].getValue();
		// Iterate over each channel
		int channels = max(inputs[_MODF_INPUT].getChannels(), 1);
		for (int c = 0; c < channels; c += 4) {
			// Get the oscillator
			auto* oscillator = &osc[c / 4];
			float_4 pitch = freqParam;
			// Set the pitch
			pitch += inputs[_MODF_INPUT].template getVoltageSimd<float_4>(c);
			oscillator->setPitch(pitch, 1.f);
			// Set the shape
			float_4 shape = shapeParam;
			if (inputs[_MODS_INPUT].isConnected())
				shape *= inputs[_MODS_INPUT].template getVoltageSimd<float_4>(c) / 10.f;
			oscillator->setShape(shape);
			// Process and output
			oscillator->process(args.sampleTime);
			outputs[_WAVE_OUTPUT].setVoltageSimd(5.f * oscillator->_Out(), c);
		}
		outputs[_WAVE_OUTPUT].setChannels(channels);
	}
};

template <_CZWaveOp Op>
struct _CZOscPlainWidget : ModuleWidget {
	_CZOscPlainWidget(_CZOscPlain<Op>* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, _czWaveSvg(Op))));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 57.0)), module, _CZOscPlain<Op>::_FREQ_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 68.0)), module, _CZOscPlain<Op>::_FINE_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 90.0)), module, _CZOscPlain<Op>::_SHAPE_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, _CZOscPlain<Op>::_MODF_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, _CZOscPlain<Op>::_MODS_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, _CZOscPlain<Op>::_WAVE_OUTPUT));
	}
};

// Reso group: CZReso1, CZReso2, CZReso3 (no LFO switch, has DC block + context menu)
template <_CZWaveOp Op>
struct _CZOscReso : Module {
	enum ParamIds {
		_FREQ_PARAM,
		_FINE_PARAM,
		_SHAPE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_MODF_INPUT,
		_MODS_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_WAVE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	_CZOscVoice<Op> osc[4];
	_DCBlock dcb[4];

	bool dcBlock = false;

	_CZOscReso() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_FREQ_PARAM, -54.f, 54.f, 0.f, "Frequency", " Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);
		configParam(_FINE_PARAM, -1.f, 1.f, 0.f, "Fine frequency");
		configParam(_SHAPE_PARAM, 0.f, 1.f, 0.f, "Shape");
		configInput(_MODF_INPUT, "Frequency modulation");
		configInput(_MODS_INPUT, "Shape modulation");
		configOutput(_WAVE_OUTPUT, "Waveform");
	}

	void onAdd() override {
		for (int i = 0; i < 4; i++) osc[i].init(); // init all 4 banks, not just bank 0
	}

	void onReset() override {
		onAdd();
	}

	void process(const ProcessArgs& args) override {
		// Get the frequency parameters
		float freqParam = params[_FREQ_PARAM].getValue() / 12.f;
		freqParam += dsp::quadraticBipolar(params[_FINE_PARAM].getValue()) * 3.f / 12.f;
		// Get the shape parameter
		float shapeParam = params[_SHAPE_PARAM].getValue();
		// Iterate over each channel
		int channels = max(inputs[_MODF_INPUT].getChannels(), 1);
		for (int c = 0; c < channels; c += 4) {
			// Get the oscillator
			auto* oscillator = &osc[c / 4];
			float_4 pitch = freqParam;
			// Set the pitch
			pitch += inputs[_MODF_INPUT].template getVoltageSimd<float_4>(c);
			oscillator->setPitch(pitch, 1.f);
			// Set the shape
			float_4 shape = shapeParam;
			if (inputs[_MODS_INPUT].isConnected())
				shape *= inputs[_MODS_INPUT].template getVoltageSimd<float_4>(c) / 10.f;
			oscillator->setShape(shape);
			// Process and output
			oscillator->process(args.sampleTime);
			float_4 out;
			if (dcBlock) {
				// DC Blocker
				auto* dcblock = &dcb[c / 4];
				out = dcblock->process(oscillator->_Out());
			} else {
				out = oscillator->_Out();
			}
			outputs[_WAVE_OUTPUT].setVoltageSimd(5.f * out, c);
		}
		outputs[_WAVE_OUTPUT].setChannels(channels);
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "dcBlock", json_integer(dcBlock));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* dcBlockJ = json_object_get(rootJ, "dcBlock");
		if (dcBlockJ)
			dcBlock = json_integer_value(dcBlockJ);
	}
};

template <_CZWaveOp Op>
struct _CZOscResoWidget : ModuleWidget {
	_CZOscResoWidget(_CZOscReso<Op>* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, _czWaveSvg(Op))));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 57.0)), module, _CZOscReso<Op>::_FREQ_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 68.0)), module, _CZOscReso<Op>::_FINE_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 90.0)), module, _CZOscReso<Op>::_SHAPE_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, _CZOscReso<Op>::_MODF_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, _CZOscReso<Op>::_MODS_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, _CZOscReso<Op>::_WAVE_OUTPUT));
	}

	struct DCBlock : MenuItem {
		_CZOscReso<Op>* module;
		void onAction(const event::Action& e) override {
			module->dcBlock ^= 0x1;
		}
	};

	void appendContextMenu(Menu* menu) override {
		_CZOscReso<Op>* module = dynamic_cast<_CZOscReso<Op>*>(this->module);
		assert(module);

		menu->addChild(new MenuLabel());

		DCBlock* htItem = createMenuItem<DCBlock>("DC Block", CHECKMARK(module->dcBlock != 0));
		htItem->module = module;
		menu->addChild(htItem);
	}
};

// Shared scaffolding for the single-wave Moog style oscillator modules (MaugSaw, MaugSaw2, MaugShark,
// MaugSquare, MaugSquare2, MaugSquare3, MaugTriangle). Only the wave index and panel SVG differ.
enum class _MaugWaveOp { Triangle = 0, Shark = 1, Saw = 2, Square = 3, Square2 = 4, Square3 = 5, InvSaw = 6 };

inline const char* _maugWaveSvg(_MaugWaveOp op) {
	switch (op) {
	case _MaugWaveOp::Triangle: return "res/MaugTriangle.svg";
	case _MaugWaveOp::Shark:    return "res/MaugShark.svg";
	case _MaugWaveOp::Saw:      return "res/MaugSaw.svg";
	case _MaugWaveOp::Square:   return "res/MaugSquare.svg";
	case _MaugWaveOp::Square2:  return "res/MaugSquare2.svg";
	case _MaugWaveOp::Square3:  return "res/MaugSquare3.svg";
	case _MaugWaveOp::InvSaw:   return "res/MaugSaw2.svg";
	}
	return "";
}

template <_MaugWaveOp Op>
struct _MaugOscSingle : Module {
	enum ParamIds {
		_LFO_PARAM,
		_FREQ_PARAM,
		_FINE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_MODF_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_WAVE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	_MaugOsc osc[4];

	_MaugOscSingle() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam<_OnOff>(_LFO_PARAM, 0, 1, 0, "LFO Mode");
		configParam(_FREQ_PARAM, -54.f, 54.f, 0.f, "Frequency", " Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);
		configParam(_FINE_PARAM, -1.f, 1.f, 0.f, "Fine frequency / LFO Offset");
		configInput(_MODF_INPUT, "Frequency modulation");
		configOutput(_WAVE_OUTPUT, "Waveform");
	}

	void onAdd() override {
		for (int i = 0; i < 4; i++) osc[i].init(); // init all 4 banks, not just bank 0
	}

	void onReset() override {
		onAdd();
	}

	void process(const ProcessArgs& args) override {
		// Get the frequency parameters
		float freqParam = params[_FREQ_PARAM].getValue() / 12.f;
		// LFO mode
		if (params[_LFO_PARAM].getValue())
			freqParam = (freqParam * 2) - 5;
		freqParam += dsp::quadraticBipolar(params[_FINE_PARAM].getValue()) * 3.f / 12.f;
		// Iterate over each channel
		int channels = max(inputs[_MODF_INPUT].getChannels(), 1);
		for (int c = 0; c < channels; c += 4) {
			// Get the oscillator
			auto* oscillator = &osc[c / 4];
			oscillator->channels = min(channels - c, 4);
			float_4 pitch = freqParam;
			// Set the pitch
			pitch += inputs[_MODF_INPUT].template getVoltageSimd<float_4>(c);
			oscillator->setWave((float)static_cast<int>(Op));
			oscillator->setPitch(pitch, 1.f);
			// Process and output
			oscillator->process(args.sampleTime);
			float_4 off = params[_LFO_PARAM].getValue() * params[_FINE_PARAM].getValue() * 5.f;
			outputs[_WAVE_OUTPUT].setVoltageSimd(5.f * oscillator->_Out() + off, c);
		}
		outputs[_WAVE_OUTPUT].setChannels(channels);
	}
};

template <_MaugWaveOp Op>
struct _MaugOscSingleWidget : ModuleWidget {
	_MaugOscSingleWidget(_MaugOscSingle<Op>* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, _maugWaveSvg(Op))));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Hsw>(mm2px(Vec(5.1, 68.0)), module, _MaugOscSingle<Op>::_LFO_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 79.0)), module, _MaugOscSingle<Op>::_FREQ_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 90.0)), module, _MaugOscSingle<Op>::_FINE_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, _MaugOscSingle<Op>::_MODF_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, _MaugOscSingle<Op>::_WAVE_OUTPUT));
	}
};

// Shared scaffolding for the binary clock Countah/Dividah modules. Only count direction and panel SVG differ.
template <bool CountUp>
struct _ClockDivCounter : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		_CLOCK_INPUT,
		_RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_BY2_OUTPUT,
		_BY4_OUTPUT,
		_BY8_OUTPUT,
		_BY16_OUTPUT,
		_BY32_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	_ClockDivCounter() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configInput(_CLOCK_INPUT, "Clock");
		configInput(_RESET_INPUT, "Reset");
		configOutput(_BY2_OUTPUT, "By 2");
		configOutput(_BY4_OUTPUT, "By 4");
		configOutput(_BY8_OUTPUT, "By 8");
		configOutput(_BY16_OUTPUT, "By 16");
		configOutput(_BY32_OUTPUT, "By 32");
	}

	uint8_t value = 0;
	bool clock_was0 = true;
	bool reset_was0 = true;

	void updateOutputs() {
		outputs[_BY2_OUTPUT].setVoltage(value & 1 ? GATE_VOLTAGE : 0.0f);
		outputs[_BY4_OUTPUT].setVoltage(value & 2 ? GATE_VOLTAGE : 0.0f);
		outputs[_BY8_OUTPUT].setVoltage(value & 4 ? GATE_VOLTAGE : 0.0f);
		outputs[_BY16_OUTPUT].setVoltage(value & 8 ? GATE_VOLTAGE : 0.0f);
		outputs[_BY32_OUTPUT].setVoltage(value & 16 ? GATE_VOLTAGE : 0.0f);
	}

	void onAdd() override {
		clock_was0 = true;
		reset_was0 = true;
		value = 0;
		updateOutputs();
	}

	void onReset() override {
		onAdd();
	}

	void process(const ProcessArgs& args) override {
		// Process RESET signal with proper threshold
		if (inputs[_RESET_INPUT].getVoltage() > GATE_THRESHOLD) {
			if (reset_was0) {
				reset_was0 = false;
				onReset();
				// Update outputs immediately after reset
				updateOutputs();
			}
		} else {
			reset_was0 = true;
		}

		// Process CLOCK signal
		if (inputs[_CLOCK_INPUT].getVoltage() > GATE_THRESHOLD) {
			if (clock_was0) {
				clock_was0 = false;
				if (CountUp)
					++value;
				else
					--value;
				// Prevent overflow/underflow by masking to 5 bits (since we only use bits 0-4)
				value &= 0x1F;
				updateOutputs();
			}
		} else {
			clock_was0 = true;
		}
	}
};

template <bool CountUp>
struct _ClockDivCounterWidget : ModuleWidget {
	_ClockDivCounterWidget(_ClockDivCounter<CountUp>* module, const char* svg) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, svg)));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 46.0)), module, _ClockDivCounter<CountUp>::_CLOCK_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 57.0)), module, _ClockDivCounter<CountUp>::_RESET_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 68.0)), module, _ClockDivCounter<CountUp>::_BY2_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, _ClockDivCounter<CountUp>::_BY4_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 90.0)), module, _ClockDivCounter<CountUp>::_BY8_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, _ClockDivCounter<CountUp>::_BY16_OUTPUT));
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, _ClockDivCounter<CountUp>::_BY32_OUTPUT));
	}
};

#endif