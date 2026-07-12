#pragma once
// NUI: CheckBox widget
// Toggle checkbox with text label.

#include "ui/widget.h"
#include <string>
#include <functional>

namespace nui {

class Font;

class CheckBox : public Widget {
public:
    CheckBox();

    // State
    void SetChecked(bool checked);
    bool IsChecked() const { return m_checked; }

    // Appearance
    void SetText(const std::string& text) { m_text = text; }
    const std::string& GetText() const { return m_text; }
    void SetFontSize(int size) { m_fontSize = size; }
    void SetFont(Font* font) { m_font = font; }
    void SetTextColor(const Color& c) { m_textColor = c; }
    void SetCheckColor(const Color& c) { m_checkColor = c; }
    void SetBoxSize(int size) { m_boxSize = size; }

    // Callback
    using CheckedCallback = std::function<void(Widget*, bool)>;
    void SetOnCheckedChanged(CheckedCallback cb) { m_onCheckedChanged = std::move(cb); }

    // Widget overrides
    bool HandleInput(InputState& input) override;
    void Render(Canvas& canvas, FontManager& fonts) override;

private:
    bool m_checked = false;
    std::string m_text;
    int m_fontSize = 14;
    Font* m_font = nullptr;
    int m_boxSize = 18;

    Color m_textColor  = Color::White();
    Color m_checkColor = Color(80, 200, 120, 255);

    CheckedCallback m_onCheckedChanged;
};

} // namespace nui
