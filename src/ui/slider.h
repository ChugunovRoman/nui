#pragma once
// NUI: Slider widget
// Draggable slider for selecting a value within a range.

#include "ui/widget.h"
#include <string>
#include <functional>

namespace nui {

class Font;

class Slider : public Widget {
public:
    Slider();

    // Value (normalized 0.0–1.0)
    void SetValue(float value);
    float GetValue() const { return m_value; }

    // Range (for display purposes, value is always 0–1 internally)
    void SetRange(float minVal, float maxVal);
    float GetMinValue() const { return m_minValue; }
    float GetMaxValue() const { return m_maxValue; }
    float GetRangedValue() const { return m_minValue + m_value * (m_maxValue - m_minValue); }

    // Appearance
    void SetTrackHeight(int h) { m_trackHeight = h; }
    void SetThumbWidth(int w) { m_thumbWidth = w; }
    void SetThumbHeight(int h) { m_thumbHeight = h; }
    void SetTrackColor(const Color& c) { m_trackColor = c; }
    void SetFillColor(const Color& c) { m_fillColor = c; }
    void SetThumbColor(const Color& c) { m_thumbColor = c; }

    // Callback
    using SliderCallback = std::function<void(Widget*, float)>;
    void SetOnValueChanged(SliderCallback cb) { m_onValueChanged = std::move(cb); }

    // Widget overrides
    bool HandleInput(InputState& input) override;
    void Render(Canvas& canvas, FontManager& fonts) override;

private:
    float ValueFromMouseX(int mx) const;

    float m_value = 0.5f;
    float m_minValue = 0.0f;
    float m_maxValue = 1.0f;

    int m_trackHeight = 6;
    int m_thumbWidth  = 12;
    int m_thumbHeight = 20;

    Color m_trackColor = Color(60, 60, 75, 255);
    Color m_fillColor  = Color(60, 140, 220, 255);
    Color m_thumbColor = Color(200, 210, 230, 255);

    bool m_dragging = false;
    SliderCallback m_onValueChanged;
};

} // namespace nui
