#pragma once
// NUI: TitlebarWidget
// Custom titlebar for borderless windows with drag-to-move, minimize,
// maximize/restore, and close buttons. The drag zone is the area outside
// the buttons — clicking there initiates window drag via Application.
// Double-click on the drag zone toggles maximize/restore.

#include "ui/widget.h"
#include <string>
#include <functional>

namespace nui {

class Font;
class Texture;
class Button;

class TitlebarWidget : public Widget {
public:
    TitlebarWidget();

    // Title text
    void SetTitle(const std::string& title);
    const std::string& GetTitle() const { return m_title; }

    // Icon (optional, drawn to the left of title)
    void SetIcon(Texture* icon);
    Texture* GetIcon() const { return m_icon; }

    // Button visibility
    void ShowMinimize(bool show);
    void ShowMaximize(bool show);
    void ShowClose(bool show);

    // Callbacks
    using ActionCallback = std::function<void()>;
    void SetOnClose(ActionCallback cb)    { m_onClose = std::move(cb); }
    void SetOnMinimize(ActionCallback cb) { m_onMinimize = std::move(cb); }
    void SetOnMaximize(ActionCallback cb) { m_onMaximize = std::move(cb); }

    // Style
    void SetTitleColor(const Color& c) { m_titleColor = c; }
    const Color& GetTitleColor() const { return m_titleColor; }
    void SetFontSize(int size) { m_fontSize = size; }
    int GetFontSize() const { return m_fontSize; }
    void SetFont(Font* font) { m_font = font; }

    // ISSUE 1: configurable button size and double-click window. When unset,
    // the titlebar reads the app-wide values from Application (see AppDesc).
    void SetButtonSize(int px) { m_buttonSize = px; }
    int  GetButtonSize() const { return m_buttonSize; }
    void SetDoubleClickMs(int ms) { m_doubleClickMs = ms; }
    int  GetDoubleClickMs() const { return m_doubleClickMs; }

    // Widget overrides
    bool HandleInput(InputState& input) override;
    void Render(Canvas& canvas, FontManager& fonts) override;

private:
    // Position buttons on the right side of the titlebar
    void LayoutButtons();
    // Check if mouse is in the drag zone (titlebar area outside buttons)
    bool IsInDragZone(int mx, int my) const;
    // ISSUE 1: resolve per-widget overrides against app-wide defaults
    int ResolvedButtonSize() const;
    int ResolvedDoubleClickMs() const;

    std::string m_title;
    Texture*    m_icon    = nullptr;
    Font*       m_font    = nullptr;
    int         m_fontSize = 14;
    Color       m_titleColor = Color::White();

    // Buttons (owned children, created in constructor)
    Button* m_btnMin   = nullptr;
    Button* m_btnMax   = nullptr;
    Button* m_btnClose = nullptr;

    // Button size (width and height of the square buttons). 0 = read from
    // Application (AppDesc::titlebarButtonSize). ISSUE 1: configurable.
    int m_buttonSize = 0;

    // Double-click window. 0 = read from Application. ISSUE 1: configurable.
    int m_doubleClickMs = 0;

    // Double-click detection
    Uint32 m_lastClickTime = 0;

    ActionCallback m_onClose;
    ActionCallback m_onMinimize;
    ActionCallback m_onMaximize;
};

} // namespace nui
