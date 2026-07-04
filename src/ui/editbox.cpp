// NUI: EditBox implementation
// Text input with proper key handling, focus management, and cursor blink.

#include "ui/editbox.h"
#include "core/input.h"
#include "renderer/font.h"

#include <SDL.h>
#include <algorithm>

namespace nui {

EditBox::EditBox() {
    m_type = "editbox";
    m_bgColor = Color(30, 30, 45, 255);
    m_borderColor = Color(80, 80, 100, 255);
}

void EditBox::SetText(const std::string& text) {
    m_text = text;
    m_cursorPos = static_cast<int>(m_text.size());
}

void EditBox::ResetCursorBlink() {
    m_cursorBlink = 0.0f;
    m_cursorVisible = true;
}

void EditBox::MoveCursor(int pos) {
    m_cursorPos = std::max(0, std::min(pos, static_cast<int>(m_text.size())));
    ResetCursorBlink();
}

void EditBox::InsertText(const std::string& text) {
    if (m_maxLength > 0 && static_cast<int>(m_text.size() + text.size()) > m_maxLength)
        return;
    m_text.insert(m_cursorPos, text);
    m_cursorPos += static_cast<int>(text.size());
    ResetCursorBlink();
    if (m_onTextChanged) m_onTextChanged(this, m_text);
}

void EditBox::DeleteCharAtCursor(int offset) {
    if (offset < 0) { // Backspace
        if (m_cursorPos > 0) {
            // Find start of previous UTF-8 character (skip continuation bytes 10xxxxxx)
            int pos = m_cursorPos - 1;
            while (pos > 0 && (static_cast<uint8_t>(m_text[pos]) & 0xC0) == 0x80) {
                pos--;
            }
            int len = m_cursorPos - pos;
            m_text.erase(pos, len);
            m_cursorPos = pos;
            if (m_onTextChanged) m_onTextChanged(this, m_text);
        }
    } else { // Delete
        if (m_cursorPos < static_cast<int>(m_text.size())) {
            // Find end of current UTF-8 character
            int len = 1;
            while (m_cursorPos + len < static_cast<int>(m_text.size()) &&
                   (static_cast<uint8_t>(m_text[m_cursorPos + len]) & 0xC0) == 0x80) {
                len++;
            }
            m_text.erase(m_cursorPos, len);
            if (m_onTextChanged) m_onTextChanged(this, m_text);
        }
    }
    ResetCursorBlink();
}

bool EditBox::HandleInput(InputState& input) {
    if (!m_visible || !m_enabled) return false;

    Rect abs = GetAbsoluteRect();
    int mx = input.GetMouseX();
    int my = input.GetMouseY();

    // Click to focus / click outside to unfocus
    if (input.IsMouseClicked(MouseButton::Left)) {
        if (abs.Contains(mx, my)) {
            // Clear focus from ALL widgets in the tree first
            if (GetParent()) {
                GetParent()->ClearFocus();
            }
            m_focused = true;
            MoveCursor(static_cast<int>(m_text.size()));
        } else {
            m_focused = false;
        }
    }

    if (!m_focused) return false;

    // Text input from SDL_TEXTINPUT (regular characters)
    const std::string& textInput = input.GetTextInput();
    if (!textInput.empty()) {
        InsertText(textInput);
    }

    // Special keys - use IsKeyClicked for edge detection (one action per press)
    if (input.IsKeyClicked(SDLK_BACKSPACE)) {
        DeleteCharAtCursor(-1);
    }
    if (input.IsKeyClicked(SDLK_DELETE)) {
        DeleteCharAtCursor(1);
    }
    if (input.IsKeyClicked(SDLK_LEFT)) {
        // Move left by one UTF-8 character (skip continuation bytes)
        int pos = m_cursorPos - 1;
        while (pos > 0 && (static_cast<uint8_t>(m_text[pos]) & 0xC0) == 0x80) pos--;
        MoveCursor(pos);
    }
    if (input.IsKeyClicked(SDLK_RIGHT)) {
        // Move right by one UTF-8 character (skip continuation bytes)
        int pos = m_cursorPos + 1;
        while (pos < static_cast<int>(m_text.size()) &&
               (static_cast<uint8_t>(m_text[pos]) & 0xC0) == 0x80) pos++;
        MoveCursor(pos);
    }
    if (input.IsKeyClicked(SDLK_HOME)) {
        MoveCursor(0);
    }
    if (input.IsKeyClicked(SDLK_END)) {
        MoveCursor(static_cast<int>(m_text.size()));
    }
    if (input.IsKeyClicked(SDLK_RETURN) || input.IsKeyClicked(SDLK_KP_ENTER)) {
        if (m_onEnter) m_onEnter(this, m_text);
    }

    return true;
}

void EditBox::Update(float dt) {
    // Cursor blink - framerate independent
    if (m_focused) {
        m_cursorBlink += dt * 2.0f; // 2.0 = full on/off cycles per second
        if (m_cursorBlink >= 1.0f) m_cursorBlink -= 1.0f;
        m_cursorVisible = m_cursorBlink < 0.5f;
    }
    Widget::Update(dt);
}

void EditBox::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;

    Rect abs = GetAbsoluteRect();

    // Background
    canvas.FillRect(abs, m_bgColor);

    // Border (brighter when focused)
    Color border = m_focused ? Color(120, 140, 200, 255) : m_borderColor;
    canvas.DrawRect(abs, border);

    Font* font = m_font ? m_font : fonts.GetDefault(m_fontSize);
    if (!font) {
        RenderChildren(canvas, fonts);
        return;
    }

    std::string displayText = m_text;
    if (m_password) displayText = std::string(m_text.size(), '*');

    int ty = abs.y + (abs.h - font->GetHeight()) / 2;

    if (displayText.empty() && !m_focused && !m_placeholder.empty()) {
        canvas.DrawText(*font, m_placeholder, abs.x + 4, ty, m_placeholderColor);
    } else {
        canvas.DrawText(*font, displayText, abs.x + 4, ty, m_textColor);

        // Draw cursor only when focused and visible
        if (m_focused && m_cursorVisible) {
            int cursorX = abs.x + 4 + font->GetTextWidth(displayText.substr(0, m_cursorPos));
            canvas.FillRect(Rect(cursorX, ty, 2, font->GetHeight()), m_cursorColor);
        }
    }

    RenderChildren(canvas, fonts);
}

} // namespace nui
