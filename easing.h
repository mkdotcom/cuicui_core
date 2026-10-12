// cuicui - easing functions
// SPDX-License-Identifier: MIT
//
// The 30 classic easing curves (plots: https://easings.net).
// Each function maps a normalized time t in [0,1] to a progress value:
// 0 at t=0 and 1 at t=1 ("Back" and "Elastic" curves overshoot in between).

#pragma once

namespace cui {
enum easing_functions
{
	EaseInSine,
	EaseOutSine,
	EaseInOutSine,
	EaseInQuad,
	EaseOutQuad,
	EaseInOutQuad,
	EaseInCubic,
	EaseOutCubic,
	EaseInOutCubic,
	EaseInQuart,
	EaseOutQuart,
	EaseInOutQuart,
	EaseInQuint,
	EaseOutQuint,
	EaseInOutQuint,
	EaseInExpo,
	EaseOutExpo,
	EaseInOutExpo,
	EaseInCirc,
	EaseOutCirc,
	EaseInOutCirc,
	EaseInBack,
	EaseOutBack,
	EaseInOutBack,
	EaseInElastic,
	EaseOutElastic,
	EaseInOutElastic,
	EaseInBounce,
	EaseOutBounce,
	EaseInOutBounce
};

typedef double (*easingFunction)(double);

// Returns the function implementing the given curve (nullptr for an invalid value).
easingFunction getEasingFunction(easing_functions function);

} // namespace cui