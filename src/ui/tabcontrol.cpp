// NUI: TabControl implementation
// Header strip + single visible content page.

#include "ui/tabcontrol.h"
#include "core/input.h"

#include <algorithm>

namespace nui {

TabControl::TabControl() {
    m_type = "tabcontrol";
    m_bgColor = Color(30, 30, 42, 255);
}

void TabControl::AddTab(const std::string& title, std::unique_ptr<Widget> page) {
    Tab tab;
    tab.title = title;
    tab.page = std::move(page);
    m_tabs.push_back(std::move(tab));
    // Default-active becomes the first tab.
    if (m_activeIndex < 0) {
        m_activeIndex = 0;
    }
}

void TabControl::RemoveTab(int index) {
    if (index < 0 || index >= static_cast<int>(m_tabs.size())) return;
    m_tabs.erase(m_tabs.begin() + index);
    if (m_tabs.empty()) {
        m_activeIndex = -1;
    } else if (index <= m_activeIndex) {
        m_activeIndex = std::max(0, m_activeIndex - 1);
    }
}

void TabControl::ClearTabs() {
    m_tabs.clear();
    m_activeIndex = -1;
}

void TabControl::SetActiveTab(int index) {
    if (m_tabs.empty()) index = -1;
    if (index < 0) index = 0;
    if (index >= static_cast<int>(m_tabs.size())) index = static_cast<int>(m_tabs.size()) - 1;
    if (index == m_activeIndex) return;
    m_activeIndex = index;
    if (m_onTabChanged) m_onTabChanged(this, m_activeIndex);
}

Widget* TabControl::GetPage(int index) const {
    if (index < 0 || index >= static_cast<int>(m_tabs.size())) return nullptr;
    return m_tabs[index].page.get();
}

Rect TabControl::GetContentRect(const Rect& abs) const {
    return Rect(abs.x, abs.y + m_tabHeight, abs.w, std::max(0, abs.h - m_tabHeight));
}

Rect TabControl::GetTabRect(const Rect& abs, int index) const {
    int count = static_cast<int>(m_tabs.size());
    if (count <= 0) return Rect();
    int w = m_fixedTabWidth > 0 ? m_fixedTabWidth : (abs.w / count);
    int x = abs.x + index * w;
    return Rect(x, abs.y, w, m_tabHeight);
}

bool TabControl::HandleInput(InputState& input) {
    if (!m_visible || !m_enabled) return false;

    Rect abs = GetAbsoluteRect();
    int mx = input.GetMouseX();
    int my = input.GetMouseY();

    // Track hovered tab header.
    m_hoveredTab = -1;
    for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i) {
        if (GetTabRect(abs, i).Contains(mx, my)) {
            m_hoveredTab = i;
            break;
        }
    }

    // Click on a tab header → switch active tab.
    if (input.IsMouseClicked(MouseButton::Left) && m_hoveredTab >= 0) {
        SetActiveTab(m_hoveredTab);
        return true;
    }

    // Forward input to the active page (clipped to content area).
    if (m_activeIndex >= 0 && m_activeIndex < static_cast<int>(m_tabs.size())) {
        Widget* page = m_tabs[m_activeIndex].page.get();
        if (page && page->IsVisible() && page->IsEnabled()) {
            // Temporarily reposition the page to absolute content coords so its
            // GetAbsoluteRect hit-test lines up with where we draw it.
            Rect content = GetContentRect(abs);
            int origX = page->GetX();
            int origY = page->GetY();
            page->SetPos(content.x - abs.x, content.y - abs.y);
            bool consumed = page->HandleInput(input);
            page->SetPos(origX, origY);
            if (consumed) return true;
        }
    }

    // Click inside our bounds but not on a header/page still belongs to us.
    if (abs.Contains(mx, my) && input.IsMouseClicked(MouseButton::Left)) {
        return true;
    }
    return false;
}

void TabControl::Update(float dt) {
    // Pages are stored outside m_children (see AddTab), so the base Update()
    // would never reach them. Drive the active page here so animations /
    // timers / cursor blink on widgets inside a tab keep ticking.
    if (m_activeIndex >= 0 && m_activeIndex < static_cast<int>(m_tabs.size())) {
        Widget* page = m_tabs[m_activeIndex].page.get();
        if (page && page->IsVisible()) {
            page->Update(dt);
        }
    }
}

void TabControl::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;

    Rect abs = GetAbsoluteRect();

    // Background for the whole control.
    canvas.FillRect(abs, m_bgColor);

    Font* font = m_font ? m_font : fonts.GetDefault(m_fontSize);

    // Tab header strip.
    for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i) {
        Rect tr = GetTabRect(abs, i);
        Color bg = m_tabBgColor;
        if (i == m_activeIndex) bg = m_activeTabColor;
        else if (i == m_hoveredTab) bg = m_hoverTabColor;
        canvas.FillRect(tr, bg);
        if (font) {
            int ty = tr.y + (tr.h - font->GetHeight()) / 2;
            // Use slightly muted text for inactive tabs.
            Color tc = (i == m_activeIndex) ? m_textColor : Color(180, 180, 200, 255);
            canvas.DrawText(*font, m_tabs[i].title, tr.x + 8, ty, tc);
        }
    }

    // Content area background.
    Rect content = GetContentRect(abs);
    canvas.FillRect(content, m_contentBgColor);

    // Render the active page clipped to the content area. We position the page
    // in absolute content coords for the duration of the render, then restore.
    if (m_activeIndex >= 0 && m_activeIndex < static_cast<int>(m_tabs.size())) {
        Widget* page = m_tabs[m_activeIndex].page.get();
        if (page && page->IsVisible()) {
            int origX = page->GetX();
            int origY = page->GetY();
            page->SetPos(content.x - abs.x, content.y - abs.y);
            // Only resize (and thus MarkDirty) when the content area actually
            // changed — calling SetSize every frame would invalidate the page's
            // render cache needlessly.
            if (page->GetWidth() != content.w || page->GetHeight() != content.h) {
                page->SetSize(content.w, content.h);
            }

            canvas.PushClip(content);
            page->Render(canvas, fonts);
            canvas.PopClip();

            page->SetPos(origX, origY);
        }
    }

    // Border on top.
    if (m_borderColor.a > 0) {
        canvas.DrawRect(abs, m_borderColor);
    }
    // NOTE: RenderChildren is intentionally NOT called here. Pages are stored
    // in m_tabs, not m_children, and they don't receive input either — so any
    // widget added directly to a TabControl would be drawn but non-interactive.
    // To keep the two halves consistent we simply don't draw direct children.
}

} // namespace nui
