// NUI: Dropdown (ComboBox) implementation

#include "ui/dropdown.h"
#include "core/input.h"
#include "core/application.h"
#include "renderer/font.h"
#include <algorithm>
#include <vector>

namespace nui {

Dropdown::Dropdown() {
    m_type = "dropdown";
    m_bgColor = Color(40, 40, 55, 255);
    m_borderColor = Color(80, 80, 100, 255);
}

Dropdown::~Dropdown() {
    // Release capture if we still hold it on destruction.
    Application* app = GetApp();
    if (app && app->GetCaptureWidget() == this) {
        app->SetCaptureWidget(nullptr);
    }
}

void Dropdown::AddItem(const std::string& item) {
    m_items.push_back(item);
    if (m_selectedIndex < 0) m_selectedIndex = 0;
}

void Dropdown::ClearItems() {
    m_items.clear();
    m_selectedIndex = -1;
    m_expanded = false;
}

void Dropdown::SetSelectedIndex(int index) {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        m_selectedIndex = index;
        if (m_onItemSelected) {
            m_onItemSelected(this, m_selectedIndex, m_items[m_selectedIndex]);
        }
    }
}

std::string Dropdown::GetSelectedText() const {
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size())) {
        return m_items[m_selectedIndex];
    }
    return {};
}

void Dropdown::ReleaseCapture() {
    Application* app = GetApp();
    if (app && app->GetCaptureWidget() == this) {
        app->SetCaptureWidget(nullptr);
    }
}

// Compute the expanded list panel rect. Opens below the header by default; if
// there is not enough room below and we know the screen height, opens upward.
Rect Dropdown::GetPanelRect(const Rect& abs) const {
    int visibleItems = std::min(m_maxVisibleItems, static_cast<int>(m_items.size()));
    int panelH = visibleItems * m_itemHeight;

    int screenH = m_screenH;
    if (screenH <= 0) {
        // Fall back to the application window size if available.
        Application* app = GetApp();
        if (app) screenH = app->GetHeight();
    }

    int panelY = abs.y + abs.h; // default: below
    if (screenH > 0) {
        int spaceBelow = screenH - (abs.y + abs.h);
        if (spaceBelow < panelH && abs.y >= panelH) {
            // Not enough room below, but enough above — open upward.
            panelY = abs.y - panelH;
        }
    }
    return Rect(abs.x, panelY, abs.w, panelH);
}

bool Dropdown::HandleInput(InputState& input) {
    if (!m_visible || !m_enabled) return false;

    Rect abs = GetAbsoluteRect();
    int mx = input.GetMouseX();
    int my = input.GetMouseY();

    // IMPORTANT: when expanded, the dropdown "captures" any left click so it can
    // close itself on outside clicks instead of letting the click fall through
    // to widgets beneath (which would both activate them AND leave the dropdown
    // open). Capture is registered with the Application so input reaches us
    // even when the cursor is over a sibling widget on top of the z-order.
    if (input.IsMouseClicked(MouseButton::Left)) {
        if (!m_expanded) {
            // Collapsed: only react to a click on the header itself.
            if (abs.Contains(mx, my)) {
                m_expanded = true;
                m_scrollOffset = 0;
                // Grab input capture so subsequent clicks always reach us.
                Application* app = GetApp();
                if (app) app->SetCaptureWidget(this);
                return true;
            }
            return false;
        }

        // Expanded: decide between header / panel / outside.
        Rect panelRect = GetPanelRect(abs);
        bool upward = panelRect.y < abs.y;

        // Click on the header while expanded — toggle closed.
        if (abs.Contains(mx, my)) {
            m_expanded = false;
            ReleaseCapture();
            return true;
        }
        // Click inside the panel — pick the item.
        if (panelRect.Contains(mx, my)) {
            int row = (my - panelRect.y) / m_itemHeight;
            int visibleItems = std::min(m_maxVisibleItems, static_cast<int>(m_items.size()));
            int itemIdx;
            if (upward) {
                // Opened upward: the top row of the panel is the LAST visible
                // item, mirroring Render's placement.
                itemIdx = (visibleItems - 1 - row) + m_scrollOffset;
            } else {
                itemIdx = row + m_scrollOffset;
            }
            if (itemIdx >= 0 && itemIdx < static_cast<int>(m_items.size())) {
                m_selectedIndex = itemIdx;
                if (m_onItemSelected) {
                    m_onItemSelected(this, m_selectedIndex, m_items[m_selectedIndex]);
                }
            }
            m_expanded = false;
            ReleaseCapture();
            return true;
        }

        // Click outside — close AND consume the click so nothing beneath reacts.
        m_expanded = false;
        ReleaseCapture();
        return true;
    }

    if (m_expanded) {
        Rect panelRect = GetPanelRect(abs);
        bool upward = panelRect.y < abs.y;

        // Scroll wheel when hovering the panel.
        if (panelRect.Contains(mx, my)) {
            int wheel = input.GetWheelY();
            if (wheel != 0) {
                m_scrollOffset = std::max(0, m_scrollOffset - wheel);
                int maxScroll = std::max(0, static_cast<int>(m_items.size()) - m_maxVisibleItems);
                m_scrollOffset = std::min(m_scrollOffset, maxScroll);
                return true;
            }
        }

        // Track hover for visual feedback. Clamp to the visible range so the
        // index never points past the last item.
        if (panelRect.Contains(mx, my)) {
            int row = (my - panelRect.y) / m_itemHeight;
            int visibleItems = std::min(m_maxVisibleItems, static_cast<int>(m_items.size()));
            int idx;
            if (upward) {
                idx = (visibleItems - 1 - row) + m_scrollOffset;
            } else {
                idx = row + m_scrollOffset;
            }
            m_hoveredItem = std::max(-1, std::min(idx,
                static_cast<int>(m_items.size()) - 1));
        } else {
            m_hoveredItem = -1;
        }
    }

    return false;
}

void Dropdown::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;

    Rect abs = GetAbsoluteRect();

    // Background
    canvas.FillRect(abs, m_bgColor);

    Font* font = m_font ? m_font : fonts.GetDefault(m_fontSize);
    if (!font) {
        RenderChildren(canvas, fonts);
        return;
    }

    // Selected text
    std::string displayText = GetSelectedText();
    if (!displayText.empty()) {
        int ty = abs.y + (abs.h - font->GetHeight()) / 2;
        canvas.DrawText(*font, displayText, abs.x + 8, ty, m_textColor);
    }

    // Arrow indicator
    int arrowX = abs.x + abs.w - 20;
    int arrowY = abs.y + abs.h / 2;
    Color arrowColor(150, 150, 170, 255);
    if (m_expanded) {
        // Up arrow
        canvas.DrawLine(arrowX, arrowY + 3, arrowX + 5, arrowY - 3, arrowColor);
        canvas.DrawLine(arrowX + 5, arrowY - 3, arrowX + 10, arrowY + 3, arrowColor);
    } else {
        // Down arrow
        canvas.DrawLine(arrowX, arrowY - 3, arrowX + 5, arrowY + 3, arrowColor);
        canvas.DrawLine(arrowX + 5, arrowY + 3, arrowX + 10, arrowY - 3, arrowColor);
    }

    // Border
    canvas.DrawRect(abs, m_borderColor);

    // Expanded dropdown list.
    if (m_expanded && !m_items.empty()) {
        Rect panelRect = GetPanelRect(abs);
        bool upward = panelRect.y < abs.y;
        int visibleItems = std::min(m_maxVisibleItems, static_cast<int>(m_items.size()));

        // The panel extends beyond this widget's own rect, so it would normally
        // be clipped by an ancestor ScrollView's PushClip. Temporarily pop all
        // ancestor clips, remember them, then restore the exact stack after
        // drawing the panel — otherwise subsequent widgets would lose clipping.
        std::vector<Rect> savedClips;
        while (canvas.IsClipped()) {
            savedClips.push_back(canvas.GetClipRect());
            canvas.PopClip();
        }
        canvas.PushClip(panelRect);

        // Panel background
        canvas.FillRect(panelRect, Color(35, 35, 50, 255));

        // Items
        for (int i = 0; i < visibleItems; ++i) {
            int itemIdx = i + m_scrollOffset;
            if (itemIdx >= static_cast<int>(m_items.size())) break;

            int itemY = upward
                ? panelRect.y + (visibleItems - 1 - i) * m_itemHeight
                : panelRect.y + i * m_itemHeight;
            Rect itemRect(panelRect.x, itemY, panelRect.w, m_itemHeight);

            // Hover highlight
            if (itemIdx == m_hoveredItem) {
                canvas.FillRect(itemRect, m_itemHoverColor);
            }

            // Selected highlight (left bar)
            if (itemIdx == m_selectedIndex) {
                canvas.FillRect(Rect(itemRect.x, itemRect.y, 3, itemRect.h), m_selectedColor);
            }

            // Item text
            int textY = itemY + (m_itemHeight - font->GetHeight()) / 2;
            canvas.DrawText(*font, m_items[itemIdx], itemRect.x + 10, textY, m_textColor);
        }

        canvas.PopClip();
        // Panel border (drawn after pop so it is not clipped by panelRect).
        canvas.DrawRect(panelRect, m_borderColor);

        // Restore the ancestor clip stack exactly as it was.
        for (auto it = savedClips.rbegin(); it != savedClips.rend(); ++it) {
            canvas.PushClip(*it);
        }
    }

    RenderChildren(canvas, fonts);
}

} // namespace nui
