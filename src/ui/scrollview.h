#pragma once
// NUI: ScrollView widget
// Scrollable container with scrollbar.

#include "ui/widget.h"

namespace nui {

class ScrollView : public Widget {
public:
    ScrollView();

    // Scroll position
    void SetScrollY(int y);
    int  GetScrollY() const { return m_scrollY; }
    int  GetMaxScrollY() const;

    // Scrollbar colors
    void SetScrollbarColor(const Color& c) { m_scrollbarColor = c; }
    void SetScrollbarBgColor(const Color& c) { m_scrollbarBgColor = c; }

    // Widget overrides
    bool HandleInput(InputState& input) override;
    void Render(Canvas& canvas, FontManager& fonts) override;

private:
    int   m_scrollY = 0;
    int   m_scrollSpeed = 30;
    bool  m_dragging = false;
    int   m_dragStartY = 0;
    int   m_dragStartScroll = 0;

    Color m_scrollbarColor    = Color(100, 100, 120, 200);
    Color m_scrollbarBgColor  = Color(40, 40, 50, 200);
};

} // namespace nui
