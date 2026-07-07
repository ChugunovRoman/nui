// NUI: Tween implementation

#include "animation/tween.h"
#include <algorithm>
#include <cmath>

namespace nui {

Tween::Tween(float* target, float from, float to, float duration, EaseType ease)
    : m_target(target)
    , m_from(from)
    , m_to(to)
    , m_duration(std::max(0.001f, duration))
    , m_easeType(ease)
{
}

Tween::Tween(float from, float to, float duration, EaseType ease)
    : m_from(from)
    , m_to(to)
    , m_duration(std::max(0.001f, duration))
    , m_easeType(ease)
{
}

void Tween::Restart() {
    m_elapsed = 0;
    m_delayElapsed = 0;
    m_active = true;
    m_paused = false;
    m_reverse = false;
}

float Tween::GetProgress() const {
    if (m_duration <= 0) return 1.0f;
    return std::min(1.0f, m_elapsed / m_duration);
}

void Tween::ApplyValue(float t) {
    EaseFunc func = GetEaseFunction(m_easeType);
    float eased = func(t);

    float value;
    if (m_reverse) {
        value = m_to + (m_from - m_to) * eased;
    } else {
        value = m_from + (m_to - m_from) * eased;
    }

    if (m_target) {
        *m_target = value;
    }
    if (m_onUpdate) {
        m_onUpdate(value);
    }
}

bool Tween::Update(float dt) {
    if (!m_active || m_paused) return m_active;

    // Handle delay
    if (m_delay > 0 && m_delayElapsed < m_delay) {
        m_delayElapsed += dt;
        return true;
    }

    m_elapsed += dt;

    if (m_elapsed >= m_duration) {
        // Finished
        if (m_loop) {
            m_elapsed -= m_duration;
            if (m_pingPong) {
                m_reverse = !m_reverse;
            }
        } else if (m_pingPong && !m_reverse) {
            // One ping-pong cycle
            m_reverse = true;
            m_elapsed = 0;
        } else {
            // Done
            m_elapsed = m_duration;
            ApplyValue(1.0f);
            m_active = false;
            if (m_onComplete) m_onComplete();
            return false;
        }
    }

    float t = m_elapsed / m_duration;
    ApplyValue(t);
    return true;
}

} // namespace nui
