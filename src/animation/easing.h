#pragma once
// NUI: Easing functions for animations
// Based on Robert Penner's easing equations + ChezzeAnimator patterns.
// All functions take t in [0, 1] and return value in [0, 1].

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace nui {

enum class EaseType {
    Linear,
    InQuad,    OutQuad,    InOutQuad,
    InCubic,   OutCubic,   InOutCubic,
    InQuart,   OutQuart,   InOutQuart,
    InQuint,   OutQuint,   InOutQuint,
    InSine,    OutSine,    InOutSine,
    InExpo,    OutExpo,    InOutExpo,
    InCirc,    OutCirc,    InOutCirc,
    InBack,    OutBack,    InOutBack,
    InElastic, OutElastic, InOutElastic,
    InBounce,  OutBounce,  InOutBounce,
    COUNT
};

// ── Individual easing functions ─────────────────────────────────
namespace ease {

inline float Linear(float t) { return t; }

inline float InQuad(float t)    { return t * t; }
inline float OutQuad(float t)   { return t * (2 - t); }
inline float InOutQuad(float t) { return t < 0.5f ? 2*t*t : -1 + (4-2*t)*t; }

inline float InCubic(float t)    { return t * t * t; }
inline float OutCubic(float t)   { float f = t - 1; return f*f*f + 1; }
inline float InOutCubic(float t) {
    return t < 0.5f ? 4*t*t*t : (t-1)*(2*t-2)*(2*t-2) + 1;
}

inline float InQuart(float t)    { return t * t * t * t; }
inline float OutQuart(float t)   { float f = t - 1; return 1 - f*f*f*f; }
inline float InOutQuart(float t) {
    return t < 0.5f ? 8*t*t*t*t : 1 - 8*(--t)*t*t*t;
}

inline float InQuint(float t)    { return t * t * t * t * t; }
inline float OutQuint(float t)   { float f = t - 1; return f*f*f*f*f + 1; }
inline float InOutQuint(float t) {
    return t < 0.5f ? 16*t*t*t*t*t : 1 + 16*(--t)*t*t*t*t;
}

inline float InSine(float t)    { return 1 - cosf(t * (float)M_PI / 2); }
inline float OutSine(float t)   { return sinf(t * (float)M_PI / 2); }
inline float InOutSine(float t) { return -(cosf((float)M_PI * t) - 1) / 2; }

inline float InExpo(float t)    { return t == 0 ? 0 : powf(2, 10 * (t - 1)); }
inline float OutExpo(float t)   { return t == 1 ? 1 : 1 - powf(2, -10 * t); }
inline float InOutExpo(float t) {
    if (t == 0) return 0;
    if (t == 1) return 1;
    t *= 2;
    if (t < 1) return 0.5f * powf(2, 10 * (t - 1));
    return 0.5f * (2 - powf(2, -10 * (t - 1)));
}

inline float InCirc(float t)    { return 1 - sqrtf(1 - t*t); }
inline float OutCirc(float t)   { float f = t - 1; return sqrtf(1 - f*f); }
inline float InOutCirc(float t) {
    t *= 2;
    if (t < 1) return -0.5f * (sqrtf(1 - t*t) - 1);
    t -= 2;
    return 0.5f * (sqrtf(1 - t*t) + 1);
}

inline float InBack(float t) {
    const float s = 1.70158f;
    return t * t * ((s + 1) * t - s);
}
inline float OutBack(float t) {
    const float s = 1.70158f;
    t -= 1;
    return t * t * ((s + 1) * t + s) + 1;
}
inline float InOutBack(float t) {
    const float s = 1.70158f * 1.525f;
    t *= 2;
    if (t < 1) return 0.5f * (t * t * ((s + 1) * t - s));
    t -= 2;
    return 0.5f * (t * t * ((s + 1) * t + s) + 2);
}

inline float InElastic(float t) {
    if (t == 0) return 0;
    if (t == 1) return 1;
    return -powf(2, 10 * (t - 1)) * sinf((t - 1.1f) * 5 * (float)M_PI);
}
inline float OutElastic(float t) {
    if (t == 0) return 0;
    if (t == 1) return 1;
    return powf(2, -10 * t) * sinf((t - 0.1f) * 5 * (float)M_PI) + 1;
}
inline float InOutElastic(float t) {
    if (t == 0) return 0;
    if (t == 1) return 1;
    t *= 2;
    if (t < 1) return -0.5f * powf(2, 10 * (t - 1)) * sinf((t - 1.1f) * 5 * (float)M_PI);
    return powf(2, -10 * (t - 1)) * sinf((t - 1.1f) * 5 * (float)M_PI) * 0.5f + 1;
}

inline float OutBounce(float t) {
    if (t < 1.0f / 2.75f) {
        return 7.5625f * t * t;
    } else if (t < 2.0f / 2.75f) {
        t -= 1.5f / 2.75f;
        return 7.5625f * t * t + 0.75f;
    } else if (t < 2.5f / 2.75f) {
        t -= 2.25f / 2.75f;
        return 7.5625f * t * t + 0.9375f;
    } else {
        t -= 2.625f / 2.75f;
        return 7.5625f * t * t + 0.984375f;
    }
}
inline float InBounce(float t)    { return 1 - OutBounce(1 - t); }
inline float InOutBounce(float t) {
    return t < 0.5f
        ? InBounce(t * 2) * 0.5f
        : OutBounce(t * 2 - 1) * 0.5f + 0.5f;
}

} // namespace ease

// ── Lookup table for enum-based dispatch ────────────────────────
using EaseFunc = float(*)(float);

inline EaseFunc GetEaseFunction(EaseType type) {
    static const EaseFunc funcs[] = {
        ease::Linear,
        ease::InQuad,    ease::OutQuad,    ease::InOutQuad,
        ease::InCubic,   ease::OutCubic,   ease::InOutCubic,
        ease::InQuart,   ease::OutQuart,   ease::InOutQuart,
        ease::InQuint,   ease::OutQuint,   ease::InOutQuint,
        ease::InSine,    ease::OutSine,    ease::InOutSine,
        ease::InExpo,    ease::OutExpo,    ease::InOutExpo,
        ease::InCirc,    ease::OutCirc,    ease::InOutCirc,
        ease::InBack,    ease::OutBack,    ease::InOutBack,
        ease::InElastic, ease::OutElastic, ease::InOutElastic,
        ease::InBounce,  ease::OutBounce,  ease::InOutBounce,
    };
    int idx = static_cast<int>(type);
    if (idx < 0 || idx >= static_cast<int>(EaseType::COUNT))
        return ease::Linear;
    return funcs[idx];
}

} // namespace nui
