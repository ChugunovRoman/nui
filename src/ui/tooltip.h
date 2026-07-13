#pragma once
// NUI: Tooltip widget + TooltipManager
// A TooltipManager singleton tracks the widget under the cursor, accumulates a
// hover delay, and — when it elapses — pushes a TooltipWidget onto the
// application's overlay stack (non-modal, no outside-click dismissal). The
// existing Widget::m_tooltip field (set via SetTooltip / XML `tooltip` attr)
// is the source of the text — this just makes it visible.

#include "ui/widget.h"
#include "ui/popup_geometry.h"
#include "renderer/font.h"
#include <string>

namespace nui {

// Visual tooltip popup. Rendered inside the overlay pass so it may appear in
// any screen coordinate. Positions itself near the cursor via PopupGeometry.
class TooltipWidget : public Widget {
public:
    TooltipWidget();

    void SetText(const std::string& text) { m_text = text; }
    const std::string& GetText() const { return m_text; }

    void SetFontSize(int size) { m_fontSize = size; }
    void SetFont(Font* font) { m_font = font; }

    // Screen-space anchor (cursor position). The widget measures its text,
    // computes a clamped rect via PopupGeometry::PlaceAtCursor and positions
    // itself accordingly during Render.
    void SetAnchor(int cursorX, int cursorY, int screenW, int screenH);

    // Widget overrides
    void Render(Canvas& canvas, FontManager& fonts) override;

private:
    std::string m_text;
    int  m_fontSize = 13;
    Font* m_font = nullptr;
    int  m_cursorX = 0;
    int  m_cursorY = 0;
    int  m_screenW = 0;
    int  m_screenH = 0;
};

// Singleton that drives tooltip show/hide. Call TooltipManager::Instance() and
// feed it input every frame (typically from Application::SetOnTick). Modeled
// after radiobutton.cpp's RadioGroupRegistry: Meyers singleton, no external
// registration needed; the overlay-stack ownership of the tooltip widget is
// managed through Application::PushOverlay/PopOverlay.
class TooltipManager {
public:
    static TooltipManager& Instance();

    // Per-frame update.
    //  - root: the widget tree to hit-test against (finds the hovered widget).
    //  - mx, my: current mouse position.
    //  - screenW/H: screen size for clamping the tooltip rect.
    //  - dt: frame delta (seconds).
    void Update(Widget* root, int mx, int my, int screenW, int screenH, float dt);

    // How long the cursor must hover a widget before the tooltip shows.
    void SetShowDelay(float seconds) { m_showDelay = seconds; }
    float GetShowDelay() const { return m_showDelay; }

    // Drop the reference to the active tooltip widget WITHOUT touching the
    // overlay stack. Called by Application when it clears the overlay stack
    // (SetRoot/ClearOverlays), because the tooltip widget owned by the stack
    // is about to be destroyed and m_activeWidget would otherwise dangle.
    void OnOverlaysCleared() { m_activeWidget = nullptr; }

private:
    TooltipManager();

    // Find the topmost visible widget under (mx, my) that has a non-empty
    // tooltip string. Returns nullptr if none.
    Widget* FindTooltipSource(Widget* root, int mx, int my) const;

    void Show(const std::string& text, int mx, int my, int screenW, int screenH);
    void Hide();

    float m_hoverTimer = 0.0f;        // accumulated hover time on the current source
    Widget* m_currentSource = nullptr; // widget currently being hovered
    TooltipWidget* m_activeWidget = nullptr; // tooltip currently on the overlay stack
    float m_showDelay = 0.5f;
};

} // namespace nui
