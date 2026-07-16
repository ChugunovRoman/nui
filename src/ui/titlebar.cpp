// NUI: TitlebarWidget implementation
// Custom titlebar for borderless windows: drag-to-move, min/max/close buttons,
// double-click to maximize/restore.

#include "ui/titlebar.h"
#include "ui/button.h"
#include "core/input.h"
#include "core/application.h"
#include "renderer/font.h"
#include "renderer/texture.h"

#include <SDL.h>

namespace nui {

TitlebarWidget::TitlebarWidget() {
    m_type = "titlebar";
    m_bgColor = Color(30, 30, 50, 255);

    // Enable window drag on the titlebar itself. Buttons (children) are
    // processed first in reverse z-order; if a button consumes the click,
    // Application won't see this widget as the drag target.
    m_windowDrag = true;

    // Create window control buttons as children (invisible until layout)
    m_btnMin = new Button();
    m_btnMin->SetText("_");
    m_btnMin->SetFontSize(14);
    m_btnMin->SetBgColor(Color(50, 50, 70, 255));
    m_btnMin->SetHoverColor(Color(70, 70, 100, 255));
    m_btnMin->SetPressedColor(Color(40, 40, 60, 255));
    m_btnMin->SetOnClick([this](Widget*) {
        if (m_onMinimize) m_onMinimize();
    });

    m_btnMax = new Button();
    m_btnMax->SetText("O");
    m_btnMax->SetFontSize(14);
    m_btnMax->SetBgColor(Color(50, 50, 70, 255));
    m_btnMax->SetHoverColor(Color(70, 70, 100, 255));
    m_btnMax->SetPressedColor(Color(40, 40, 60, 255));
    m_btnMax->SetOnClick([this](Widget*) {
        if (m_onMaximize) m_onMaximize();
    });

    m_btnClose = new Button();
    m_btnClose->SetText("X");
    m_btnClose->SetFontSize(14);
    m_btnClose->SetBgColor(Color(50, 50, 70, 255));
    m_btnClose->SetHoverColor(Color(180, 40, 40, 255));
    m_btnClose->SetPressedColor(Color(140, 30, 30, 255));
    m_btnClose->SetOnClick([this](Widget*) {
        if (m_onClose) m_onClose();
    });

    // Add buttons as children (reverse z-order: last = topmost for input)
    AddChild(std::unique_ptr<Widget>(m_btnMin));
    AddChild(std::unique_ptr<Widget>(m_btnMax));
    AddChild(std::unique_ptr<Widget>(m_btnClose));
}

void TitlebarWidget::SetTitle(const std::string& title) {
    m_title = title;
    MarkDirty();
}

void TitlebarWidget::SetIcon(Texture* icon) {
    m_icon = icon;
    MarkDirty();
}

void TitlebarWidget::ShowMinimize(bool show) {
    if (m_btnMin) m_btnMin->SetVisible(show);
}

void TitlebarWidget::ShowMaximize(bool show) {
    if (m_btnMax) m_btnMax->SetVisible(show);
}

void TitlebarWidget::ShowClose(bool show) {
    if (m_btnClose) m_btnClose->SetVisible(show);
}

// ISSUE 1: resolve a per-widget override against the app-wide default.
// Returns m_buttonSize if explicitly set (>0), otherwise the value exposed by
// Application (AppDesc::titlebarButtonSize, already DPI-scaled).
int TitlebarWidget::ResolvedButtonSize() const {
    if (m_buttonSize > 0) return m_buttonSize;
    Application* app = GetApp();
    return app ? app->GetTitlebarButtonSize() : 28;
}

// ISSUE 1: resolve the double-click window the same way.
int TitlebarWidget::ResolvedDoubleClickMs() const {
    if (m_doubleClickMs > 0) return m_doubleClickMs;
    Application* app = GetApp();
    return app ? app->GetDoubleClickMs() : 500;
}

// Layout buttons on the right side of the titlebar.
// Called every frame at the start of Render — cheap (3 widgets) and ensures
// buttons stay correct after titlebar resize.
// NOTE: button rects are in LOCAL coords (relative to titlebar).
void TitlebarWidget::LayoutButtons() {
    int localW = m_rect.w;
    int localH = m_rect.h;
    int btnSize = ResolvedButtonSize();
    int btnX = localW;  // start from right edge (local)

    // Close button (rightmost)
    if (m_btnClose && m_btnClose->IsVisible()) {
        btnX -= btnSize;
        m_btnClose->SetRect(btnX, 0, btnSize, localH);
    }
    // Maximize button
    if (m_btnMax && m_btnMax->IsVisible()) {
        btnX -= btnSize;
        m_btnMax->SetRect(btnX, 0, btnSize, localH);
    }
    // Minimize button
    if (m_btnMin && m_btnMin->IsVisible()) {
        btnX -= btnSize;
        m_btnMin->SetRect(btnX, 0, btnSize, localH);
    }
}

bool TitlebarWidget::IsInDragZone(int mx, int my) const {
    Rect abs = GetAbsoluteRect();
    if (!abs.Contains(mx, my)) return false;

    // Check if the point is inside any visible button
    for (auto& child : m_children) {
        if (child->IsVisible() && child->HitTest(mx, my)) {
            return false;  // over a button, not a drag zone
        }
    }
    return true;
}

bool TitlebarWidget::HandleInput(InputState& input) {
    if (!m_visible || !m_enabled) return false;

    // BUG 6: keep button positions current before hit-testing, so that after
    // a window resize the click targets match what is drawn this frame.
    LayoutButtons();

    int mx = input.GetMouseX();
    int my = input.GetMouseY();

    // Let children (buttons) handle input first — if a button consumes the
    // click, we don't process double-click or drag.
    if (HandleChildrenInput(input)) return true;

    // Double-click on drag zone → toggle maximize
    if (input.IsMouseClicked(MouseButton::Left) && IsInDragZone(mx, my)) {
        Uint32 now = SDL_GetTicks();
        int dcMs = ResolvedDoubleClickMs();
        if (m_lastClickTime > 0 && (now - m_lastClickTime) < static_cast<Uint32>(dcMs)) {
            Application* app = GetApp();
            if (app) app->ToggleMaximize();
            m_lastClickTime = 0;
            return true;
        }
        m_lastClickTime = now;
        // Don't consume — let Application handle drag
        return false;
    }

    return false;
}

void TitlebarWidget::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;

    // Update button positions before rendering
    LayoutButtons();

    Rect abs = GetAbsoluteRect();

    // Background
    if (m_bgColor.a > 0) {
        canvas.FillRect(abs, m_bgColor);
    }

    // Icon (if set, drawn left of title)
    int contentX = abs.x + 6;
    if (m_icon && m_icon->IsLoaded()) {
        int iconSize = abs.h - 8;
        Rect iconRect(contentX, abs.y + 4, iconSize, iconSize);
        canvas.DrawTexture(*m_icon, iconRect);
        contentX += iconSize + 6;
    }

    // Title text (left-aligned in the drag zone area, before buttons)
    if (!m_title.empty()) {
        Font* font = m_font ? m_font : fonts.GetDefault(m_fontSize);
        if (font) {
            int tw = font->GetTextWidth(m_title);
            int th = font->GetHeight();

            // Calculate buttons total width
            int btnSize = ResolvedButtonSize();
            int buttonsWidth = 0;
            for (auto& child : m_children) {
                if (child->IsVisible()) buttonsWidth += btnSize;
            }

            int maxTextW = abs.w - (contentX - abs.x) - buttonsWidth - 8;
            int tx = contentX;
            int ty = abs.y + (abs.h - th) / 2;
            // Clip text if too long
            if (tw > maxTextW) tw = maxTextW;
            if (tw > 0) {
                canvas.DrawText(*font, m_title, tx, ty, m_titleColor);
            }
        }
    }

    // Bottom border line
    canvas.DrawLine(abs.x, abs.y + abs.h - 1,
                    abs.x + abs.w, abs.y + abs.h - 1,
                    Color(60, 60, 80, 255));

    // Render children (buttons)
    RenderChildren(canvas, fonts);
}

} // namespace nui
