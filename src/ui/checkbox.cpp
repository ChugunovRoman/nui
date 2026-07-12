// NUI: CheckBox implementation

#include "ui/checkbox.h"
#include "core/input.h"
#include "renderer/font.h"

namespace nui {

CheckBox::CheckBox() {
    m_type = "checkbox";
    m_bgColor = Color::Transparent();
}

void CheckBox::SetChecked(bool checked) {
    if (checked != m_checked) {
        m_checked = checked;
        if (m_onCheckedChanged) m_onCheckedChanged(this, m_checked);
    }
}

bool CheckBox::HandleInput(InputState& input) {
    if (!m_visible || !m_enabled) return false;

    Rect abs = GetAbsoluteRect();
    int mx = input.GetMouseX();
    int my = input.GetMouseY();

    if (abs.Contains(mx, my) && input.IsMouseClicked(MouseButton::Left)) {
        m_checked = !m_checked;
        if (m_onCheckedChanged) m_onCheckedChanged(this, m_checked);
        return true;
    }

    return false;
}

void CheckBox::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;

    Rect abs = GetAbsoluteRect();

    // Background
    if (m_bgColor.a > 0) {
        canvas.FillRect(abs, m_bgColor);
    }

    // Checkbox square (vertically centered)
    int boxY = abs.y + (abs.h - m_boxSize) / 2;
    Rect boxRect(abs.x, boxY, m_boxSize, m_boxSize);

    // Box outline
    canvas.FillRect(boxRect, Color(50, 50, 65, 255));
    canvas.DrawRect(boxRect, Color(90, 90, 110, 255));

    // Checked state: fill + checkmark
    if (m_checked) {
        // Inner fill
        canvas.FillRect(Rect(boxRect.x + 2, boxRect.y + 2,
                              boxRect.w - 4, boxRect.h - 4), m_checkColor);

        // Checkmark (two lines forming a check)
        int x1 = boxRect.x + 4;
        int y1 = boxRect.y + boxRect.h / 2;
        int x2 = boxRect.x + boxRect.w / 3;
        int y2 = boxRect.y + boxRect.h - 5;
        int x3 = boxRect.x + boxRect.w - 4;
        int y3 = boxRect.y + 5;
        canvas.DrawLine(x1, y1, x2, y2, Color::White());
        canvas.DrawLine(x2, y2, x3, y3, Color::White());
    }

    // Text (right of checkbox)
    if (!m_text.empty()) {
        Font* font = m_font ? m_font : fonts.GetDefault(m_fontSize);
        if (font) {
            int textX = abs.x + m_boxSize + 8;
            int textY = abs.y + (abs.h - font->GetHeight()) / 2;
            canvas.DrawText(*font, m_text, textX, textY, m_textColor);
        }
    }

    // Border — the base Render() override was silently dropping this (bug 10).
    if (m_borderColor.a > 0) {
        canvas.DrawRect(abs, m_borderColor);
    }

    RenderChildren(canvas, fonts);
}

} // namespace nui
