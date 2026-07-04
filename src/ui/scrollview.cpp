// NUI: ScrollView implementation
// Renders children with vertical offset, draws scrollbar.

#include "ui/scrollview.h"
#include "core/input.h"

#include <algorithm>

namespace nui {

ScrollView::ScrollView() {
    m_type = "scrollview";
    m_bgColor = Color(25, 25, 35, 255);
}

void ScrollView::SetScrollY(int y) {
    int maxY = GetMaxScrollY();
    m_scrollY = std::max(0, std::min(y, maxY));
}

int ScrollView::GetMaxScrollY() const {
    int totalH = 0;
    for (auto& child : m_children) {
        int bottom = child->GetY() + child->GetHeight();
        if (bottom > totalH) totalH = bottom;
    }
    return std::max(0, totalH - m_rect.h);
}

bool ScrollView::HandleInput(InputState& input) {
    if (!m_visible || !m_enabled) return false;

    Rect abs = GetAbsoluteRect();
    int mx = input.GetMouseX();
    int my = input.GetMouseY();

    // Scroll with mouse wheel when hovering
    // (SDL_MOUSEWHEEL is translated to scroll in Application)
    // For now, use up/down keys as a fallback

    // Mouse wheel scrolling
    int wheel = input.GetWheelY();
    if (wheel != 0 && abs.Contains(mx, my)) {
        m_scrollY -= wheel * m_scrollSpeed;
        int maxScroll = GetMaxScrollY();
        m_scrollY = std::max(0, std::min(m_scrollY, maxScroll));
        return true;
    }

    // Drag scrollbar
    if (input.IsMouseClicked(MouseButton::Left) && abs.Contains(mx, my)) {
        // Check if click is on scrollbar area (right edge)
        int scrollbarX = abs.x + abs.w - 10;
        if (mx >= scrollbarX) {
            m_dragging = true;
            m_dragStartY = my;
            m_dragStartScroll = m_scrollY;
        }
    }

    if (m_dragging) {
        if (input.IsMouseDown(MouseButton::Left)) {
            int dy = my - m_dragStartY;
            int maxScroll = GetMaxScrollY();
            if (abs.h > 0 && maxScroll > 0) {
                float scrollRatio = static_cast<float>(dy) / abs.h;
                m_scrollY = m_dragStartScroll + static_cast<int>(scrollRatio * maxScroll);
                m_scrollY = std::max(0, std::min(m_scrollY, maxScroll));
            }
        } else {
            m_dragging = false;
        }
        return true;
    }

    // Pass input to children (with offset)
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        // Temporarily offset child for hit testing
        int origY = (*it)->GetY();
        (*it)->SetPos((*it)->GetX(), origY - m_scrollY);
        bool consumed = (*it)->HandleInput(input);
        (*it)->SetPos((*it)->GetX(), origY);
        if (consumed) return true;
    }

    return false;
}

void ScrollView::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;

    Rect abs = GetAbsoluteRect();

    // Background
    canvas.FillRect(abs, m_bgColor);

    // Clip to scrollview bounds
    canvas.PushClip(abs);

    // Render children with scroll offset
    for (auto& child : m_children) {
        int origY = child->GetY();
        child->SetPos(child->GetX(), origY - m_scrollY);
        child->Render(canvas, fonts);
        child->SetPos(child->GetX(), origY);
    }

    canvas.PopClip();

    // Draw scrollbar
    int maxScroll = GetMaxScrollY();
    if (maxScroll > 0) {
        int scrollbarW = 10;
        int scrollbarX = abs.x + abs.w - scrollbarW;

        // Scrollbar background
        canvas.FillRect(Rect(scrollbarX, abs.y, scrollbarW, abs.h), m_scrollbarBgColor);

        // Scrollbar thumb (clamp to scrollbar area)
        float viewRatio = static_cast<float>(abs.h) / (abs.h + maxScroll);
        int thumbH = std::min(abs.h, std::max(20, static_cast<int>(abs.h * viewRatio)));
        float scrollRatio = static_cast<float>(m_scrollY) / maxScroll;
        int thumbY = abs.y + static_cast<int>((abs.h - thumbH) * scrollRatio);

        canvas.FillRect(Rect(scrollbarX, thumbY, scrollbarW, thumbH), m_scrollbarColor);
    }

    // Border
    if (m_borderColor.a > 0) {
        canvas.DrawRect(abs, m_borderColor);
    }
}

} // namespace nui
