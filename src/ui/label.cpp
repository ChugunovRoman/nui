// NUI: Label implementation

#include "ui/label.h"
#include "renderer/font.h"

namespace nui {

Label::Label() {
    m_type = "label";
}

void Label::SetText(const std::string& text) {
    m_text = text;
}

void Label::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;

    DrawBackground(canvas);

    if (!m_text.empty()) {
        Font* font = m_font ? m_font : fonts.GetDefault(m_fontSize);
        if (font) {
            Rect abs = GetAbsoluteRect();

            // Calculate text position based on alignment
            int tw = font->GetTextWidth(m_text);
            int th = font->GetHeight();
            int tx = abs.x;
            int ty = abs.y;

            if (m_alignH == AlignH::Center) {
                tx = abs.x + (abs.w - tw) / 2;
            } else if (m_alignH == AlignH::Right) {
                tx = abs.x + abs.w - tw;
            }

            if (m_alignV == AlignV::Center) {
                ty = abs.y + (abs.h - th) / 2;
            } else if (m_alignV == AlignV::Bottom) {
                ty = abs.y + abs.h - th;
            }

            if (m_wordWrap) {
                // Word-wrap always uses abs.x as origin, alignment is ignored for wrapped text
                canvas.DrawTextWrapped(*font, m_text,
                    Rect(abs.x, ty, abs.w, abs.h - (ty - abs.y)), m_textColor);
            } else {
                canvas.DrawText(*font, m_text, tx, ty, m_textColor);
            }
        }
    }

    RenderChildren(canvas, fonts);
}

} // namespace nui
