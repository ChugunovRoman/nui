#pragma once
// NUI: Animator - manages active tweens and provides convenience API.
// Integrates with Widget::Update(dt) for automatic animation.

#include "animation/tween.h"
#include <vector>
#include <memory>
#include <string>

namespace nui {

class Widget;

// ── Animator: manages a collection of tweens ────────────────────
class Animator {
public:
    // Create and start a new tween animating a float pointer
    static Tween* Animate(float* target, float from, float to,
                           float duration, EaseType ease = EaseType::Linear);

    // Animate with callback (no target pointer)
    static Tween* Animate(float from, float to, float duration,
                           EaseType ease = EaseType::Linear);

    // Animate a widget's X position
    static Tween* AnimateX(Widget* widget, float to, float duration,
                             EaseType ease = EaseType::OutCubic);

    // Animate a widget's Y position
    static Tween* AnimateY(Widget* widget, float to, float duration,
                             EaseType ease = EaseType::OutCubic);

    // Animate a widget's width
    static Tween* AnimateWidth(Widget* widget, float to, float duration,
                                 EaseType ease = EaseType::OutCubic);

    // Animate a widget's height
    static Tween* AnimateHeight(Widget* widget, float to, float duration,
                                  EaseType ease = EaseType::OutCubic);

    // Animate a widget's alpha (opacity)
    static Tween* AnimateAlpha(Widget* widget, float to, float duration,
                                 EaseType ease = EaseType::Linear);

    // Animate a widget's rotation (degrees)
    static Tween* AnimateRotation(Widget* widget, float toDeg, float duration,
                                    EaseType ease = EaseType::OutCubic);

    // Cancel all tweens with a specific tag
    static void CancelByTag(const std::string& tag);

    // Cancel all active tweens
    static void CancelAll();

    // Update all active tweens (called automatically by Application::Run)
    static void UpdateAll(float dt);

    // Get number of active tweens
    static size_t GetActiveCount();

private:
    static std::vector<std::unique_ptr<Tween>>& GetTweens();

    // Tag a tween as the owner of a widget property and cancel any previous
    // tween owning the same property on the same widget. Keeps at most one
    // active animation per (widget, property) pair.
    static void OwnPropertyTween(Tween* t, const Widget* widget, const char* property);
};

} // namespace nui
