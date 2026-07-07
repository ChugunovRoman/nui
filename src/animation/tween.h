#pragma once
// NUI: Tween - animates a single float value over time.
// Supports easing functions, callbacks, ping-pong, and repeat.

#include "animation/easing.h"
#include <functional>
#include <string>

namespace nui {

class Tween {
public:
    using UpdateFunc = std::function<void(float value)>;
    using CompleteFunc = std::function<void()>;

    Tween() = default;

    // Create a tween that animates a float pointer
    Tween(float* target, float from, float to, float duration,
          EaseType ease = EaseType::Linear);

    // Create a tween with callback (no target pointer)
    Tween(float from, float to, float duration,
          EaseType ease = EaseType::Linear);

    // Update the tween. Returns true if still active.
    bool Update(float dt);

    // Control
    void Pause()  { m_paused = true; }
    void Resume() { m_paused = false; }
    void Restart();
    void Cancel() { m_active = false; }

    // Configuration (chainable)
    Tween& OnUpdate(UpdateFunc cb)   { m_onUpdate = std::move(cb); return *this; }
    Tween& OnComplete(CompleteFunc cb) { m_onComplete = std::move(cb); return *this; }
    Tween& SetLoop(bool loop)        { m_loop = loop; return *this; }
    Tween& SetPingPong(bool pp)      { m_pingPong = pp; return *this; }
    Tween& SetDelay(float delay)     { m_delay = delay; return *this; }
    Tween& SetTag(const std::string& tag) { m_tag = tag; return *this; }

    // State
    bool IsActive()   const { return m_active; }
    bool IsPaused()   const { return m_paused; }
    bool IsFinished() const { return !m_active && !m_paused; }
    float GetProgress() const;
    const std::string& GetTag() const { return m_tag; }

private:
    void ApplyValue(float t);

    float* m_target = nullptr;
    float  m_from   = 0.0f;
    float  m_to     = 1.0f;
    float  m_duration = 1.0f;
    float  m_elapsed  = 0.0f;
    float  m_delay    = 0.0f;
    float  m_delayElapsed = 0.0f;

    EaseType m_easeType = EaseType::Linear;
    bool     m_active   = true;
    bool     m_paused   = false;
    bool     m_loop     = false;
    bool     m_pingPong = false;
    bool     m_reverse  = false;

    std::string m_tag;
    UpdateFunc   m_onUpdate;
    CompleteFunc m_onComplete;
};

} // namespace nui
