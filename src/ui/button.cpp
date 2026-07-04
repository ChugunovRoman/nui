// NUI: Button implementation
// Handles hover, press, click states with visual feedback.

#include "ui/button.h"
#include "core/input.h"
#include "renderer/font.h"
#include "renderer/texture.h"

namespace nui {

Button::Button() {
    m_type = "button";
    m_bgColor = Color(50, 50, 70, 255);
}

void Button::SetText(const std::string& text) {
    m_text = text;
}

bool Button::HandleInput(InputState& input) {
    if (!m_visible || !m_enabled) return false;

    Rect abs = GetAbsoluteRect();
    int mx = input.GetMouseX();
    int my = input.GetMouseY();

    m_hovered = abs.Contains(mx, my);
    m_pressed = m_hovered && input.IsMouseDown(MouseButton::Left);

    if (m_hovered && input.IsMouseClicked(MouseButton::Left)) {
        if (m_onClick) {
            m_onClick(this);
            return true;
        }
    }

    // Let children handle input too
    return HandleChildrenInput(input);
}

void Button::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;

    Rect abs = GetAbsoluteRect();

    // Background with state-dependent color
    Color bg = m_bgColor;
    if (!m_enabled) {
        bg = m_disabledColor;
    } else if (m_pressed) {
        bg = m_pressedColor;
    } else if (m_hovered) {
        bg = m_hoverColor;
    }
    canvas.FillRect(abs, bg);

    // Border
    if (m_borderColor.a > 0) {
        canvas.DrawRect(abs, m_borderColor);
    } else {
        // Default subtle border
        canvas.DrawRect(abs, Color(100, 100, 120, 255));
    }

    // Icon (if set)
    int iconOffset = 0;
    if (m_icon && m_icon->IsLoaded()) {
        int iconSize = abs.h - 8;
        Rect iconRect(abs.x + 6, abs.y + 4, iconSize, iconSize);
        canvas.DrawTexture(*m_icon, iconRect);
        iconOffset = iconSize + 8;
    }

    // Text
    if (!m_text.empty()) {
        Font* font = m_font ? m_font : fonts.GetDefault(m_fontSize);
        if (font) {
            int tw = font->GetTextWidth(m_text);
            int th = font->GetHeight();
            int tx = abs.x + iconOffset + (abs.w - iconOffset - tw) / 2;
            int ty = abs.y + (abs.h - th) / 2;
            canvas.DrawText(*font, m_text, tx, ty, m_textColor);
        }
    }

    RenderChildren(canvas, fonts);
}

} // namespace nui
