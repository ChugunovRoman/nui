#pragma once
// NUI: ProgressBar widget
// Visual progress indicator with customizable colors and optional text.

#include "ui/widget.h"
#include <string>

namespace nui {

class Font;

class ProgressBar : public Widget {
public:
    ProgressBar();

    // Progress value [0.0 .. 1.0]
    void SetValue(float value);
    float GetValue() const { return m_value; }

    // Colors
    void SetFillColor(const Color& c) { m_fillColor = c; }
    void SetEmptyColor(const Color& c) { m_emptyColor = c; }
    void SetTextColor(const Color& c) { m_textColor = c; }

    // Optional text (e.g., "75%" or custom label)
    void SetShowPercent(bool show) { m_showPercent = show; }
    void SetLabel(const std::string& label) { m_label = label; }

    // Font
    void SetFont(Font* font) { m_font = font; }
    void SetFontSize(int size) { m_fontSize = size; }

    // Widget overrides
    void Render(Canvas& canvas, FontManager& fonts) override;

private:
    float      m_value = 0.0f;
    Color      m_fillColor  = Color(40, 160, 40, 255);
    Color      m_emptyColor = Color(40, 40, 50, 255);
    Color      m_textColor  = Color::White();
    bool       m_showPercent = false;
    std::string m_label;
    Font*      m_font     = nullptr;
    int        m_fontSize = 14;
};

} // namespace nui
