// cuicui - easing functions
// SPDX-License-Identifier: MIT
//
// Standard easing equations, written from the formulas published on https://easings.net.

#include "easing.h"
namespace cui {} using namespace cui; // for code outside the namespace (ObjC blocks, main)

#include <cmath>

namespace cui {
namespace
{
	const double PI = 3.14159265358979323846;

	double p2(double x) { return x * x; }
	double p3(double x) { return x * x * x; }
	double p4(double x) { return p2(p2(x)); }
	double p5(double x) { return p4(x) * x; }

	// Sine
	double easeInSine(double t) { return 1 - std::cos(t * PI / 2); }
	double easeOutSine(double t) { return std::sin(t * PI / 2); }
	double easeInOutSine(double t) { return -(std::cos(PI * t) - 1) / 2; }

	// Power curves
	double easeInQuad(double t) { return p2(t); }
	double easeOutQuad(double t) { return 1 - p2(1 - t); }
	double easeInOutQuad(double t) { return t < 0.5 ? 2 * p2(t) : 1 - p2(-2 * t + 2) / 2; }

	double easeInCubic(double t) { return p3(t); }
	double easeOutCubic(double t) { return 1 - p3(1 - t); }
	double easeInOutCubic(double t) { return t < 0.5 ? 4 * p3(t) : 1 - p3(-2 * t + 2) / 2; }

	double easeInQuart(double t) { return p4(t); }
	double easeOutQuart(double t) { return 1 - p4(1 - t); }
	double easeInOutQuart(double t) { return t < 0.5 ? 8 * p4(t) : 1 - p4(-2 * t + 2) / 2; }

	double easeInQuint(double t) { return p5(t); }
	double easeOutQuint(double t) { return 1 - p5(1 - t); }
	double easeInOutQuint(double t) { return t < 0.5 ? 16 * p5(t) : 1 - p5(-2 * t + 2) / 2; }

	// Exponential
	double easeInExpo(double t) { return t <= 0 ? 0 : std::pow(2, 10 * t - 10); }
	double easeOutExpo(double t) { return t >= 1 ? 1 : 1 - std::pow(2, -10 * t); }
	double easeInOutExpo(double t)
	{
		if (t <= 0) return 0;
		if (t >= 1) return 1;
		return t < 0.5 ? std::pow(2, 20 * t - 10) / 2 : (2 - std::pow(2, -20 * t + 10)) / 2;
	}

	// Circular
	double easeInCirc(double t) { return 1 - std::sqrt(1 - p2(t)); }
	double easeOutCirc(double t) { return std::sqrt(1 - p2(t - 1)); }
	double easeInOutCirc(double t)
	{
		return t < 0.5 ? (1 - std::sqrt(1 - p2(2 * t))) / 2 : (std::sqrt(1 - p2(-2 * t + 2)) + 1) / 2;
	}

	// Back (overshoots)
	const double backC1 = 1.70158;
	const double backC2 = backC1 * 1.525;
	const double backC3 = backC1 + 1;
	double easeInBack(double t) { return backC3 * p3(t) - backC1 * p2(t); }
	double easeOutBack(double t) { return 1 + backC3 * p3(t - 1) + backC1 * p2(t - 1); }
	double easeInOutBack(double t)
	{
		return t < 0.5
			? (p2(2 * t) * ((backC2 + 1) * 2 * t - backC2)) / 2
			: (p2(2 * t - 2) * ((backC2 + 1) * (t * 2 - 2) + backC2) + 2) / 2;
	}

	// Elastic (overshoots)
	double easeInElastic(double t)
	{
		if (t <= 0) return 0;
		if (t >= 1) return 1;
		return -std::pow(2, 10 * t - 10) * std::sin((t * 10 - 10.75) * (2 * PI / 3));
	}
	double easeOutElastic(double t)
	{
		if (t <= 0) return 0;
		if (t >= 1) return 1;
		return std::pow(2, -10 * t) * std::sin((t * 10 - 0.75) * (2 * PI / 3)) + 1;
	}
	double easeInOutElastic(double t)
	{
		if (t <= 0) return 0;
		if (t >= 1) return 1;
		const double c5 = 2 * PI / 4.5;
		return t < 0.5
			? -(std::pow(2, 20 * t - 10) * std::sin((20 * t - 11.125) * c5)) / 2
			: (std::pow(2, -20 * t + 10) * std::sin((20 * t - 11.125) * c5)) / 2 + 1;
	}

	// Bounce
	double easeOutBounce(double t)
	{
		const double n1 = 7.5625;
		const double d1 = 2.75;
		if (t < 1 / d1) return n1 * t * t;
		if (t < 2 / d1) { t -= 1.5 / d1; return n1 * t * t + 0.75; }
		if (t < 2.5 / d1) { t -= 2.25 / d1; return n1 * t * t + 0.9375; }
		t -= 2.625 / d1;
		return n1 * t * t + 0.984375;
	}
	double easeInBounce(double t) { return 1 - easeOutBounce(1 - t); }
	double easeInOutBounce(double t)
	{
		return t < 0.5 ? (1 - easeOutBounce(1 - 2 * t)) / 2 : (1 + easeOutBounce(2 * t - 1)) / 2;
	}
}

easingFunction getEasingFunction(easing_functions function)
{
	switch (function)
	{
	case EaseInSine:       return easeInSine;
	case EaseOutSine:      return easeOutSine;
	case EaseInOutSine:    return easeInOutSine;
	case EaseInQuad:       return easeInQuad;
	case EaseOutQuad:      return easeOutQuad;
	case EaseInOutQuad:    return easeInOutQuad;
	case EaseInCubic:      return easeInCubic;
	case EaseOutCubic:     return easeOutCubic;
	case EaseInOutCubic:   return easeInOutCubic;
	case EaseInQuart:      return easeInQuart;
	case EaseOutQuart:     return easeOutQuart;
	case EaseInOutQuart:   return easeInOutQuart;
	case EaseInQuint:      return easeInQuint;
	case EaseOutQuint:     return easeOutQuint;
	case EaseInOutQuint:   return easeInOutQuint;
	case EaseInExpo:       return easeInExpo;
	case EaseOutExpo:      return easeOutExpo;
	case EaseInOutExpo:    return easeInOutExpo;
	case EaseInCirc:       return easeInCirc;
	case EaseOutCirc:      return easeOutCirc;
	case EaseInOutCirc:    return easeInOutCirc;
	case EaseInBack:       return easeInBack;
	case EaseOutBack:      return easeOutBack;
	case EaseInOutBack:    return easeInOutBack;
	case EaseInElastic:    return easeInElastic;
	case EaseOutElastic:   return easeOutElastic;
	case EaseInOutElastic: return easeInOutElastic;
	case EaseInBounce:     return easeInBounce;
	case EaseOutBounce:    return easeOutBounce;
	case EaseInOutBounce:  return easeInOutBounce;
	}
	return nullptr;
}

} // namespace cui