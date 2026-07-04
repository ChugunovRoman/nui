#pragma once
// NUI: EditBox widget
// Single-line text input with cursor and focus management.

#include "ui/widget.h"
#include <string>

namespace nui {

class Font;

class EditBox : public Widget {
public:
    EditBox();

    void SetText(const std::string& text);
    const std::string& GetText() const { return m_text; }
    void SetPlaceholder(const std::string& text) { m_placeholder = text; }
    void SetFont(Font* font) { m_font = font; }
    void SetFontSize(int size) { m_fontSize = size; }
    void SetTextColor(const Color& c) { m_textColor = c; }
    void SetPlaceholderColor(const Color& c) { m_placeholderColor = c; }
    void SetCursorColor(const Color& c) { m_cursorColor = c; }
    void SetPasswordMode(bool mask) { m_password = mask; }
    void SetMaxLength(int len) { m_maxLength = len; }

    using TextCallback = std::function<void(EditBox*, const std::string&)>;
    void SetOnTextChanged(TextCallback cb) { m_onTextChanged = std::move(cb); }
    void SetOnEnter(TextCallback cb) { m_onEnter = std::move(cb); }

    bool HandleInput(InputState& input) override;
    void Update(float dt) override;
    void Render(Canvas& canvas, FontManager& fonts) override;

private:
    void InsertText(const std::string& text);
    void DeleteCharAtCursor(int offset);
    void MoveCursor(int pos);
    void ResetCursorBlink();

    std::string m_text;
    std::string m_placeholder;
    Font*       m_font     = nullptr;
    int         m_fontSize = 16;
    int         m_cursorPos = 0;

    Color m_textColor        = Color::White();
    Color m_placeholderColor = Color(120, 120, 140, 255);
    Color m_cursorColor      = Color(200, 200, 255, 255);

    bool  m_password  = false;
    int   m_maxLength = 0;
    float m_cursorBlink = 0.0f;
    bool  m_cursorVisible = true;

    TextCallback m_onTextChanged;
    TextCallback m_onEnter;
};

} // namespace nui
