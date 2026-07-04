// NUI: ProgressBar implementation

#include "ui/progressbar.h"
#include "renderer/font.h"

#include <algorithm>
#include <cstdio>

namespace nui {

ProgressBar::ProgressBar() {
    m_type = "progressbar";
}

void ProgressBar::SetValue(float value) {
    m_value = std::max(0.0f, std::min(1.0f, value));
}

void ProgressBar::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;

    Rect abs = GetAbsoluteRect();

    // Empty background
    canvas.FillRect(abs, m_emptyColor);

    // Filled portion (round to avoid 1px gap at 100%)
    int fillW = static_cast<int>(abs.w * m_value + 0.5f);
    fillW = std::min(fillW, abs.w);
    if (fillW > 0) {
        canvas.FillRect(Rect(abs.x, abs.y, fillW, abs.h), m_fillColor);
    }

    // Border
    if (m_borderColor.a > 0) {
        canvas.DrawRect(abs, m_borderColor);
    }

    // Text
    Font* font = m_font ? m_font : fonts.GetDefault(m_fontSize);
    if (font) {
        std::string text;
        if (!m_label.empty()) {
            text = m_label;
        } else if (m_showPercent) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%d%%", static_cast<int>(m_value * 100));
            text = buf;
        }

        if (!text.empty()) {
            int tw = font->GetTextWidth(text);
            int th = font->GetHeight();
            int tx = abs.x + (abs.w - tw) / 2;
            int ty = abs.y + (abs.h - th) / 2;
            canvas.DrawText(*font, text, tx, ty, m_textColor);
        }
    }

    RenderChildren(canvas, fonts);
}

} // namespace nui
