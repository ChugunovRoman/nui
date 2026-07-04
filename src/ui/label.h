#pragma once
// NUI: Label widget
// Displays static text with configurable font, color, and alignment.

#include "ui/widget.h"
#include <string>

namespace nui {

class Font;

class Label : public Widget {
public:
    Label();

    // Text content
    void SetText(const std::string& text);
    const std::string& GetText() const { return m_text; }

    // Font (nullptr = use default)
    void SetFont(Font* font) { m_font = font; }
    Font* GetFont() const { return m_font; }
    void SetFontSize(int size) { m_fontSize = size; }

    // Text color
    void SetTextColor(const Color& c) { m_textColor = c; }

    // Word wrap
    void SetWordWrap(bool wrap) { m_wordWrap = wrap; }

    // Widget overrides
    void Render(Canvas& canvas, FontManager& fonts) override;

private:
    std::string m_text;
    Font*       m_font     = nullptr;
    int         m_fontSize = 16;
    Color       m_textColor = Color::White();
    bool        m_wordWrap  = false;
};

} // namespace nui
