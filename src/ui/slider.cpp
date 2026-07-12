// NUI: Slider implementation

#include "ui/slider.h"
#include "core/input.h"
#include <algorithm>

namespace nui {

Slider::Slider() {
    m_type = "slider";
    m_bgColor = Color::Transparent();
}

void Slider::SetValue(float value) {
    float clamped = std::max(0.0f, std::min(1.0f, value));
    if (clamped != m_value) {
        m_value = clamped;
        if (m_onValueChanged) m_onValueChanged(this, m_value);
    }
}

void Slider::SetRange(float minVal, float maxVal) {
    m_minValue = minVal;
    m_maxValue = maxVal;
}

float Slider::ValueFromMouseX(int mx) const {
    Rect abs = GetAbsoluteRect();
    int trackW = abs.w - m_thumbWidth;
    if (trackW <= 0) return 0.0f;
    float ratio = static_cast<float>(mx - abs.x - m_thumbWidth / 2) / trackW;
    return std::max(0.0f, std::min(1.0f, ratio));
}

bool Slider::HandleInput(InputState& input) {
    // Always release drag if the button is up, even when the slider became
    // invisible/disabled mid-drag — otherwise it stays stuck "capturing"
    // input forever (bug 4).
    if (m_dragging && !input.IsMouseDown(MouseButton::Left)) {
        m_dragging = false;
    }

    if (!m_visible || !m_enabled) return false;

    Rect abs = GetAbsoluteRect();
    int mx = input.GetMouseX();
    int my = input.GetMouseY();

    // Start drag on click anywhere on the track (click-to-jump behaviour).
    if (input.IsMouseClicked(MouseButton::Left)) {
        if (abs.Contains(mx, my)) {
            m_dragging = true;
            SetValue(ValueFromMouseX(mx));
            return true;
        }
    }

    // Continue drag — consume input while held so siblings don't react.
    if (m_dragging) {
        SetValue(ValueFromMouseX(mx));
        return true;
    }

    // Mouse wheel (nudges the value by 5%).
    int wheel = input.GetWheelY();
    if (wheel != 0 && abs.Contains(mx, my)) {
        SetValue(m_value + wheel * 0.05f);
        return true;
    }

    return false;
}

void Slider::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;

    Rect abs = GetAbsoluteRect();

    // Track (centered vertically)
    int trackY = abs.y + (abs.h - m_trackHeight) / 2;
    canvas.FillRect(Rect(abs.x, trackY, abs.w, m_trackHeight), m_trackColor);

    // Fill portion
    int trackW = abs.w - m_thumbWidth;
    int fillW = static_cast<int>(trackW * m_value + 0.5f);
    if (fillW > 0) {
        canvas.FillRect(Rect(abs.x, trackY, fillW, m_trackHeight), m_fillColor);
    }

    // Thumb
    int thumbX = abs.x + static_cast<int>(trackW * m_value);
    int thumbY = abs.y + (abs.h - m_thumbHeight) / 2;
    canvas.FillRect(Rect(thumbX, thumbY, m_thumbWidth, m_thumbHeight), m_thumbColor);
    canvas.DrawRect(Rect(thumbX, thumbY, m_thumbWidth, m_thumbHeight), Color(140, 150, 170, 255));

    // Border — the base Render() override was silently dropping this (bug 10).
    if (m_borderColor.a > 0) {
        canvas.DrawRect(abs, m_borderColor);
    }

    RenderChildren(canvas, fonts);
}

} // namespace nui
