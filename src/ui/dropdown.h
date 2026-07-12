#pragma once
// NUI: Dropdown (ComboBox) widget
// Expandable list with single selection.

#include "ui/widget.h"
#include <string>
#include <vector>
#include <functional>

namespace nui {

class Font;

class Dropdown : public Widget {
public:
    Dropdown();
    ~Dropdown();

    // Items
    void AddItem(const std::string& item);
    void ClearItems();
    const std::vector<std::string>& GetItems() const { return m_items; }

    // Selection
    void SetSelectedIndex(int index);
    int GetSelectedIndex() const { return m_selectedIndex; }
    std::string GetSelectedText() const;

    // Appearance
    void SetFontSize(int size) { m_fontSize = size; }
    void SetFont(Font* font) { m_font = font; }
    void SetTextColor(const Color& c) { m_textColor = c; }
    void SetItemHeight(int h) { m_itemHeight = h; }
    void SetMaxVisibleItems(int n) { m_maxVisibleItems = n; }
    void SetItemHoverColor(const Color& c) { m_itemHoverColor = c; }
    void SetSelectedColor(const Color& c) { m_selectedColor = c; }

    // Screen size — used to decide whether to open the list upward when there
    // is not enough space below the dropdown (avoids drawing past the window).
    void SetScreenSize(int w, int h) { m_screenW = w; m_screenH = h; }

    // Callback
    using DropdownCallback = std::function<void(Widget*, int, const std::string&)>;
    void SetOnItemSelected(DropdownCallback cb) { m_onItemSelected = std::move(cb); }

    // Widget overrides
    bool HandleInput(InputState& input) override;
    void Render(Canvas& canvas, FontManager& fonts) override;

private:
    // Rect of the expanded list panel. Opens below the header by default; if
    // there is not enough vertical space below (and we know the screen size),
    // opens upward instead. Width matches the header width.
    Rect GetPanelRect(const Rect& abs) const;

    // Release input capture from the Application if we currently hold it.
    void ReleaseCapture();

    std::vector<std::string> m_items;
    int m_selectedIndex = -1;
    bool m_expanded = false;
    int m_hoveredItem = -1;
    int m_scrollOffset = 0;

    int m_fontSize = 14;
    Font* m_font = nullptr;
    int m_itemHeight = 28;
    int m_maxVisibleItems = 5;

    // Screen size hint for upward-open logic. <=0 means "unknown" (always open
    // downward, legacy behaviour).
    int m_screenW = 0;
    int m_screenH = 0;

    Color m_textColor       = Color::White();
    Color m_itemHoverColor  = Color(50, 50, 70, 255);
    Color m_selectedColor   = Color(40, 80, 140, 255);

    DropdownCallback m_onItemSelected;
};

} // namespace nui
