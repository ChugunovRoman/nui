#pragma once
// NUI: TabControl widget
// A container with a strip of clickable tab headers and a content area that
// shows the page of the currently active tab. Only one page is visible at a
// time. Modeled after ScrollView (container with PushClip) but without
// scrolling: it switches between discrete child pages.

#include "ui/widget.h"
#include "renderer/font.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace nui {

class TabControl : public Widget {
public:
    TabControl();

    // A tab = header title + page widget (the page is rendered inside the
    // content area when this tab is active). The TabControl takes ownership
    // of the page widget.
    void AddTab(const std::string& title, std::unique_ptr<Widget> page);
    void RemoveTab(int index);
    void ClearTabs();
    int  GetTabCount() const { return static_cast<int>(m_tabs.size()); }

    // Active tab selection. SetActiveTab clamps the index. When the active tab
    // changes, OnTabChanged fires (after the state is committed).
    int  GetActiveTab() const { return m_activeIndex; }
    void SetActiveTab(int index);

    // Access the page widget of a tab (nullptr if index is out of range).
    Widget* GetPage(int index) const;

    // Appearance
    void SetFontSize(int size) { m_fontSize = size; }
    void SetFont(Font* font) { m_font = font; }
    void SetTextColor(const Color& c) { m_textColor = c; }
    void SetTabHeight(int h) { m_tabHeight = h; }
    void SetTabBgColor(const Color& c) { m_tabBgColor = c; }
    void SetActiveTabColor(const Color& c) { m_activeTabColor = c; }
    void SetHoverTabColor(const Color& c) { m_hoverTabColor = c; }
    void SetContentBgColor(const Color& c) { m_contentBgColor = c; }
    // Fixed width for every tab header. When > 0, headers are laid out with
    // this exact width; otherwise they share the header strip evenly.
    void SetFixedTabWidth(int w) { m_fixedTabWidth = w; }

    // Callback fires whenever the active tab changes (index of the new tab).
    using TabCallback = std::function<void(Widget*, int)>;
    void SetOnTabChanged(TabCallback cb) { m_onTabChanged = std::move(cb); }

    // Widget overrides
    bool HandleInput(InputState& input) override;
    void Update(float dt) override;
    void Render(Canvas& canvas, FontManager& fonts) override;

private:
    struct Tab {
        std::string title;
        std::unique_ptr<Widget> page;
    };

    // Rect of the content area (below the header strip), in absolute coords.
    Rect GetContentRect(const Rect& abs) const;
    // Rect of the i-th tab header in absolute coords. Tabs are laid out
    // left-to-right with equal width if fixedWidth <= 0, otherwise each tab is
    // fixedWidth wide.
    Rect GetTabRect(const Rect& abs, int index) const;

    std::vector<Tab> m_tabs;
    int  m_activeIndex = -1;
    int  m_hoveredTab = -1;

    int  m_fontSize = 14;
    Font* m_font = nullptr;
    int  m_tabHeight = 28;
    int  m_fixedTabWidth = 0; // 0 = distribute evenly across the header strip

    Color m_textColor       = Color::White();
    Color m_tabBgColor      = Color(35, 35, 50, 255);
    Color m_activeTabColor  = Color(60, 90, 150, 255);
    Color m_hoverTabColor   = Color(50, 50, 70, 255);
    Color m_contentBgColor  = Color(25, 25, 35, 255);

    TabCallback m_onTabChanged;
};

} // namespace nui
