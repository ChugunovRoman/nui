#pragma once
// NUI: Shared math constants and helpers.
// Single source of truth for constants used across renderer and animation.

#include <cmath>

namespace nui {

// PI as a float constant. Defined here so every module uses the same value
// instead of ad-hoc literals (3.14159f, M_PI, etc.).
constexpr float kPI = 3.14159265358979323846f;

// Convert degrees <-> radians.
inline float DegToRad(float deg) { return deg * kPI / 180.0f; }
inline float RadToDeg(float rad) { return rad * 180.0f / kPI; }

} // namespace nui
