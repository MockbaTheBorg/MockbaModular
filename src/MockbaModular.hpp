#ifndef MOCKBAMODULAR_HPP
#define MOCKBAMODULAR_HPP

#include <time.h>

using simd::float_4;
using simd::int32_4;

#define DETUNE_RNG 0.001f	// Osc spread range
#define EXP_FACTOR -3.f		// Bulge factor for the Exponential Curve

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
	#define M_PI	3.14159265358979323846
#endif
#ifndef M_2PI
	#define M_2PI	6.28318530717958647692
#endif
#ifndef M_E
	#define M_E		2.71828182845904523536
#endif

#define BGCOLOR loadBack()

// Base Functions
void saveBack(const std::string& Back);
std::string loadBack();
float randomFloat();
float detune();

// simd functions
float_4 xFade(float_4 s1, float_4 s2, float mix);
float_4 mix2(float_4 s1, float_4 m1, float_4 s2, float_4 m2);
float_4 mix3(float_4 s1, float_4 m1, float_4 s2, float_4 m2, float_4 s3, float_4 m3);
float_4 mix4(float_4 s1, float_4 m1, float_4 s2, float_4 m2, float_4 s3, float_4 m3, float_4 s4, float_4 m4);
float_4 mix5(float_4 s1, float_4 m1, float_4 s2, float_4 m2, float_4 s3, float_4 m3, float_4 s4, float_4 m4, float_4 s5, float_4 m5);

// Exponential curve (Maug version)
template <typename T>
T expCurve(T x, float f) {
	return (3 + x * ((-13 - f) + 5 * x)) / (3 + (2 + f) * x);
}

// Cosine Approximation
template<typename T>
inline T mmCos(T x) noexcept {
	//	constexpr T tp = 1. / (2. * M_PI);
	x *= 0.159154943092;
	x -= T(.25) + simd::floor(x + T(.25));
	x *= T(16.) * (simd::fabs(x) - T(.5));
#if EXTRA_PRECISION
	x += T(.225) * x * (simd::fabs(x) - T(1.));
#endif
	return x;
}

// Minimal MM Oscillator (No Antialias)
struct _MMOsc {
	int wave = 0;
	float_4 shape = 0.f;
	float_4 freq = 0.f;
	float_4 detuneMap = 0.f;
	float_4 phase = 0.f;
	float_4 outValue = 0.f;

	dsp::MinBlepGenerator<16, 16, float> oscMinBlep[4];

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

	// Override this to define the oscillator waveform - (co)sinewave by default
	virtual float_4 oscStep(float_4 phase, float_4 shape, int wave) {
		return mmCos(phase * M_2PI);
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
	dsp::MinBlepGenerator<16, 16, float_4> invsawMinBlep;

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
			pulseWidth - 0.5f;
		}
	}

	void process(float deltaTime) {
		// Advance phase
		float_4 deltaPhase = simd::clamp(freq * deltaTime, 1e-6f, 0.35f);
		phase += deltaPhase;
		phase -= simd::floor(phase);

		// Jump sqr when crossing 0, or 1 if backwards
		float_4 wrapPhase = 0.f;
		float_4 wrapCrossing = (wrapPhase - (phase - deltaPhase)) / deltaPhase;
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

		// Tri
		triValue = tri(phase);
		triValue += triMinBlep.process();

		// Shark
		sharkValue = shark(phase);
		sharkValue += sharkMinBlep.process();

		// Saw
		sawValue = saw(phase);
		sawValue += sawMinBlep.process();

		// Square
		sqrValue = sqr(phase);
		sqrValue += sqrMinBlep.process();

		sqrFilter.setCutoffFreq(20.f * deltaTime);
		sqrFilter.process(sqrValue);
		sqrValue = sqrFilter.highpass() * 0.95f;

		// InvSaw
		invsawValue = invsaw(phase);
		invsawValue += invsawMinBlep.process();
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

	float_4 invsaw(float_4 phase) {
		float_4 v;
		float_4 x = phase + 0.5f;
		x -= simd::trunc(x);
		v = -expCurve(x, EXP_FACTOR);
		return -v;
	}
	float_4 invsaw() {
		return sawValue;
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
		freq = .05f + 200.f * speed;
		float deltaPhase = simd::clamp(freq * delta, 1e-6f, 0.35f);
		phase += deltaPhase;
		phase -= simd::floor(phase);

		float a = phase * 4.f;
		float b = 4.f - a;
		outTriangle = min(a, b) - 1;
		outSquare = sgn(outTriangle);
	}

	float _Triangle() {
		return outTriangle;
	}

	float _Square() {
		return outSquare;
	}
};

// Maug Filter
struct _Filter {
	float_4 tk = 0;
	float_4 tp = 0;
	float_4 tr = 0;

	float_4 src_k = 0;
	float_4 src_p = 0;
	float_4 src_r = 0;
	float_4 tgt_k = 0;
	float_4 tgt_p = 0;
	float_4 tgt_r = 0;
	float_4 d_k = 0;
	float_4 d_p = 0;
	float_4 d_r = 0;

	float_4 x = 0;
	float_4 y1 = 0;
	float_4 y2 = 0;
	float_4 y3 = 0;
	float_4 y4 = 0;

	float_4 oldx = 0;
	float_4 oldy1 = 0;
	float_4 oldy2 = 0;
	float_4 oldy3 = 0;
	float_4 oldy4 = 0;

	float_4 ftype = 0;
	float_4 cutoff = 0;
	float_4 res = 0;
	float_4 outgain = 0;

	float_4 outValue = 0;

	void setType(float_4 ftypeV) {
		ftype = ftypeV;
	}

	void setCutoff(float_4 cutoffV) {
		cutoff = cutoffV;
	}

	void setRes(float_4 resV) {
		res = resV;
	}

	void setGain(float_4 outgainV) {
		outgain = outgainV;
	}

	void process(float_4 in, float rate) {
		float_4 out;
		float_4 f = 2 * cutoff / rate;
		tgt_k = 3.6 * f - 1.6 * f * f - 1;
		tgt_p = (tgt_k + 1) * 0.5;
		float_4 scale = simd::pow(M_E, (1 - tgt_p) * 1.386249);
		tgt_r = res * scale;

		d_p = tgt_p - src_p;
		tp = src_p;
		src_p = tgt_p;
		d_k = tgt_k - src_k;
		tk = src_k;
		src_k = tgt_k;
		d_r = tgt_r - src_r;
		tr = src_r;
		src_r = tgt_r;

		tk += d_k;
		tp += d_p;
		tr += d_r;

		//filter
		x = in - tr * y4;

		y1 = x * tp + oldx * tp - tk * y1;
		y2 = y1 * tp + oldy1 * tp - tk * y2;
		y3 = y2 * tp + oldy2 * tp - tk * y3;
		y4 = y3 * tp + oldy3 * tp - tk * y4;

		oldx = x;
		oldy1 = y1;
		oldy2 = y2;
		oldy3 = y3;

		if (ftype[0] == 0) {
			out = y4;
		}
		if (ftype[0] == 1) {
			out = 6 * (y3 - y4);
		}
		if (ftype[0] == 2) {
			out = in - y4;
		}

		out *= outgain;

		outValue = simd::clamp(out, -5.f, +5.f);
	}

	float_4 _Out() {
		return outValue;
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
	float type = 0.f;

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

	virtual float_4 process(float_4 sample) {
		float_4 x = sample - resonance * stage[3];

		// Four cascaded one-pole filters (bilinear transform)
		stage[0] = x * p + delay[0] * p - k * stage[0];
		stage[1] = stage[0] * p + delay[1] * p - k * stage[1];
		stage[2] = stage[1] * p + delay[2] * p - k * stage[2];
		stage[3] = stage[2] * p + delay[3] * p - k * stage[3];

		// Clipping band-limited sigmoid
		stage[3] -= (stage[3] * stage[3] * stage[3]) / 6.0f;

		delay[0] = x;
		delay[1] = stage[0];
		delay[2] = stage[1];
		delay[3] = stage[2];

		if (type == 0.f) {
			sample = stage[3];
		} else {
			sample -= stage[3];
		}

		return sample;
	}

	virtual void setType(float t) {
		type = t;
	}

	virtual void setResonance(float r) {
		resonance = r * (t2 + 6.0f * t1) / (t2 - 6.0f * t1);
	}

	virtual void setCutoff(float_4 c) {
		cutoff = c;

		p = cutoff * (1.8f - 0.8f * cutoff);
		k = 2.0f * simd::sin(cutoff * M_PI * 0.5f) - 1.0f;
		t1 = (1.0f - p) * 1.386249f;
		t2 = 12.0f + t1 * t1;
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
		if (frame >= (1 << 8))
			frame = 0;
		int diff = lastFrame ^ frame;

		float_4 sum = 0.f;
		for (int i = 0; i < 8; i++) {
			if (diff & (1 << i)) {
				values[i] = random::uniform() - 0.5f;
			}
			sum += values[i];
		}
		return sum;
	}
};

// White Noise
struct _WhiteNoise {
	float_4 _Out() {
		float_4 white = random::normal();
		return white;
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
		float_4 result;
		for (int i = 0; i < 4; i++) {
			result[i] = (rate[i] <= 0.f) ? 0.f : exp(-log((1.f + targetRatio[i]) / targetRatio[i]) / rate[i]);
		}
		return result;
	}

	void setState(int32_4 stateV) {
		state = stateV;
	}

	void setAttackRate(float_4 rate) {
		attackRate = rate;
		attackCoef = calcCoef(rate, targetRatioA);
		attackBase = (1.f + targetRatioA) * (1.f - attackCoef);
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


#endif