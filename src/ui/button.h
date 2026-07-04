#pragma once
// NUI: Button widget
// Clickable button with hover/press states, text, and optional icon.

#include "ui/widget.h"
#include <string>

namespace nui {

class Font;
class Texture;

class Button : public Widget {
public:
    Button();

    // Text
    void SetText(const std::string& text);
    const std::string& GetText() const { return m_text; }

    // Font
    void SetFont(Font* font) { m_font = font; }
    void SetFontSize(int size) { m_fontSize = size; }

    // Colors for different states
    void SetTextColor(const Color& c) { m_textColor = c; }
    void SetHoverColor(const Color& c) { m_hoverColor = c; }
    void SetPressedColor(const Color& c) { m_pressedColor = c; }
    void SetDisabledColor(const Color& c) { m_disabledColor = c; }

    // Icon (optional, drawn to the left of text)
    void SetIcon(Texture* tex) { m_icon = tex; }

    // State
    bool IsHovered() const { return m_hovered; }
    bool IsPressed() const { return m_pressed; }

    // Widget overrides
    bool HandleInput(InputState& input) override;
    void Render(Canvas& canvas, FontManager& fonts) override;

private:
    std::string m_text;
    Font*       m_font     = nullptr;
    int         m_fontSize = 16;
    Texture*    m_icon     = nullptr;

    Color m_textColor     = Color::White();
    Color m_hoverColor    = Color(60, 60, 80, 255);
    Color m_pressedColor  = Color(40, 40, 60, 255);
    Color m_disabledColor = Color(80, 80, 80, 255);

    bool m_hovered = false;
    bool m_pressed = false;
};

} // namespace nui
