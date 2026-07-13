// NUI: Tooltip implementation
// TooltipManager drives hover timing and pushes/removes a TooltipWidget on the
// overlay stack. TooltipWidget positions itself via PopupGeometry.

#include "ui/tooltip.h"
#include "core/application.h"

namespace nui {

// ── TooltipWidget ───────────────────────────────────────────────

TooltipWidget::TooltipWidget() {
    m_type = "tooltip";
    // Transparent base: the popup draws its own filled background.
    m_bgColor = Color(0, 0, 0, 0);
}

void TooltipWidget::SetAnchor(int cursorX, int cursorY, int screenW, int screenH) {
    m_cursorX = cursorX;
    m_cursorY = cursorY;
    m_screenW = screenW;
    m_screenH = screenH;
}

void TooltipWidget::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible || m_text.empty()) return;

    Font* font = m_font ? m_font : fonts.GetDefault(m_fontSize);
    if (!font) return;

    // Measure text to size the tooltip box.
    const int padX = 8;
    const int padY = 5;
    const int margin = 8;
    int textW = font->GetTextWidth(m_text);
    int textH = font->GetHeight();
    int boxW = textW + padX * 2;
    int boxH = textH + padY * 2;

    Rect placed = PopupGeometry::PlaceAtCursor(boxW, boxH, m_cursorX, m_cursorY,
                                               m_screenW, m_screenH, margin);

    // Background + border.
    Color bgColor(40, 40, 55, 240);
    Color borderColor(90, 90, 120, 255);
    canvas.FillRect(placed, bgColor);
    canvas.DrawRect(placed, borderColor);

    // Text.
    int tx = placed.x + padX;
    int ty = placed.y + padY;
    canvas.DrawText(*font, m_text, tx, ty, Color(230, 230, 240, 255));

    RenderChildren(canvas, fonts);
}

// ── TooltipManager ──────────────────────────────────────────────

TooltipManager::TooltipManager() {}
TooltipManager& TooltipManager::Instance() {
    static TooltipManager inst;
    return inst;
}

// Recursive search for the topmost widget under (mx,my) that carries a
// non-empty tooltip. Children are checked first (they are on top), then the
// widget itself.
static Widget* FindTooltipRecursive(Widget* w, int mx, int my) {
    if (!w || !w->IsVisible()) return nullptr;

    // Children are on top of the parent, check them first (back-to-front).
    const auto& children = w->GetChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        Widget* found = FindTooltipRecursive(it->get(), mx, my);
        if (found) return found;
    }

    if (w->HitTest(mx, my) && !w->GetTooltip().empty()) {
        return w;
    }
    return nullptr;
}

Widget* TooltipManager::FindTooltipSource(Widget* root, int mx, int my) const {
    return FindTooltipRecursive(root, mx, my);
}

void TooltipManager::Show(const std::string& text, int mx, int my,
                          int screenW, int screenH) {
    Application* app = GetApp();
    if (!app) return;

    // Reuse an already-shown tooltip if possible (avoids overlay churn).
    if (!m_activeWidget) {
        auto w = std::make_unique<TooltipWidget>();
        m_activeWidget = w.get();
        w->SetFontSize(13);
        // Non-modal, do NOT close on outside click — it must follow the cursor.
        app->PushOverlay(std::move(w), false, false);
    }
    m_activeWidget->SetText(text);
    m_activeWidget->SetAnchor(mx, my, screenW, screenH);
    m_activeWidget->SetVisible(true);
}

void TooltipManager::Hide() {
    if (!m_activeWidget) return;
    Application* app = GetApp();
    if (app) app->PopOverlay(m_activeWidget);
    m_activeWidget = nullptr;
}

void TooltipManager::Update(Widget* root, int mx, int my,
                            int screenW, int screenH, float dt) {
    if (!root) {
        Hide();
        m_currentSource = nullptr;
        m_hoverTimer = 0.0f;
        return;
    }

    Widget* src = FindTooltipSource(root, mx, my);

    if (src != m_currentSource) {
        // Cursor moved to a different widget — reset timing and hide current.
        m_currentSource = src;
        m_hoverTimer = 0.0f;
        Hide();
    }

    if (!src) return; // nothing to tooltip about

    m_hoverTimer += dt;
    if (m_hoverTimer >= m_showDelay) {
        Show(src->GetTooltip(), mx, my, screenW, screenH);
    }
}

} // namespace nui
