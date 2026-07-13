// NUI: Dialog / MessageBox implementation
// Modal overlay: title bar, message text, content widgets, button row.

#include "ui/dialog.h"
#include "core/application.h"
#include "core/input.h"

#include <SDL.h> // SDLK_RETURN, SDLK_ESCAPE
#include <algorithm>

namespace nui {

Dialog::Dialog() {
    m_type = "dialog";
    m_bgColor = Color(0, 0, 0, 0); // panel draws its own background
}

void Dialog::SetButtons(DialogButtons buttons) {
    m_buttons = buttons;
    m_buttonInfos.clear();
    auto add = [&](const char* label, DialogResult r) {
        m_buttonInfos.push_back({label, r, Rect()});
    };
    switch (buttons) {
        case DialogButtons::Ok:
            add("OK", DialogResult::Ok);
            break;
        case DialogButtons::OkCancel:
            add("OK", DialogResult::Ok);
            add("Cancel", DialogResult::Cancel);
            break;
        case DialogButtons::YesNo:
            add("Yes", DialogResult::Yes);
            add("No", DialogResult::No);
            break;
        case DialogButtons::YesNoCancel:
            add("Yes", DialogResult::Yes);
            add("No", DialogResult::No);
            add("Cancel", DialogResult::Cancel);
            break;
        case DialogButtons::RetryCancel:
            add("Retry", DialogResult::Retry);
            add("Cancel", DialogResult::Cancel);
            break;
    }
}

void Dialog::AddContent(std::unique_ptr<Widget> w) {
    if (w) AddChild(std::move(w));
}

void Dialog::Open(std::unique_ptr<Dialog> dlg, int screenW, int screenH) {
    if (!dlg) return;
    Application* app = GetApp();
    if (!app) return;

    Dialog* raw = dlg.get();
    // Default button set if the host forgot to set one.
    if (raw->m_buttonInfos.empty()) raw->SetButtons(DialogButtons::Ok);

    // Auto-size: choose a reasonable dialog width/height.
    int dw = std::min(440, screenW - 40);
    int dh = std::min(220, screenH - 40);
    int dx = (screenW - dw) / 2;
    int dy = (screenH - dh) / 2;
    raw->SetRect(dx, dy, dw, dh);

    // Modal overlay: dims the background, blocks input to root.
    app->PushOverlay(std::move(dlg), true, false);
}

void Dialog::ShowMessage(const std::string& title, const std::string& message,
                         DialogButtons buttons, ResultCallback cb,
                         int screenW, int screenH) {
    auto dlg = std::make_unique<Dialog>();
    dlg->SetTitle(title);
    dlg->SetMessage(message);
    dlg->SetButtons(buttons);
    dlg->SetOnResult(std::move(cb));
    Open(std::move(dlg), screenW, screenH);
}

void Dialog::Close(DialogResult result) {
    if (m_closed) return;
    m_closed = true;
    if (m_onResult) m_onResult(result);
    Application* app = GetApp();
    if (app) app->PopOverlay(this);
}

Rect Dialog::GetButtonRect(const Rect& abs, int index) const {
    const int btnW = 90;
    const int btnH = 30;
    const int gap = 10;
    int count = static_cast<int>(m_buttonInfos.size());
    int totalW = count * btnW + (count - 1) * gap;
    int startX = abs.x + abs.w - totalW - 14;
    int y = abs.y + abs.h - btnH - 14;
    return Rect(startX + index * (btnW + gap), y, btnW, btnH);
}

void Dialog::LayoutButtons(const Rect& abs) {
    for (int i = 0; i < static_cast<int>(m_buttonInfos.size()); ++i) {
        m_buttonInfos[i].rect = GetButtonRect(abs, i);
    }
}

bool Dialog::HandleInput(InputState& input) {
    if (!m_visible || !m_enabled) return false;

    Rect abs = GetAbsoluteRect();
    int mx = input.GetMouseX();
    int my = input.GetMouseY();

    LayoutButtons(abs);

    // Hovered button.
    m_hoveredButton = -1;
    for (int i = 0; i < static_cast<int>(m_buttonInfos.size()); ++i) {
        if (m_buttonInfos[i].rect.Contains(mx, my)) {
            m_hoveredButton = i;
            break;
        }
    }

    // Button click.
    if (m_hoveredButton >= 0 && input.IsMouseClicked(MouseButton::Left)) {
        Close(m_buttonInfos[m_hoveredButton].result);
        return true;
    }

    // Keyboard shortcuts: Enter = default (first/OK), Esc = cancel/close.
    if (input.IsKeyClicked(SDLK_RETURN) && !m_buttonInfos.empty()) {
        // Default to the primary button (first).
        Close(m_buttonInfos.front().result);
        return true;
    }
    if (input.IsKeyClicked(SDLK_ESCAPE)) {
        // Map Esc to Cancel when present, else close with None.
        DialogResult r = DialogResult::None;
        for (const auto& b : m_buttonInfos) {
            if (b.result == DialogResult::Cancel) { r = DialogResult::Cancel; break; }
        }
        Close(r);
        return true;
    }

    // Forward input to content children (inside the content area).
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->HandleInput(input)) return true;
    }

    // As a modal overlay we always consume clicks inside our panel.
    if (abs.Contains(mx, my)) return true;
    return false;
}

void Dialog::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;

    Rect abs = GetAbsoluteRect();
    Font* font = m_font ? m_font : fonts.GetDefault(m_fontSize);
    LayoutButtons(abs);

    // Panel background + border.
    canvas.FillRect(abs, m_panelColor);
    canvas.DrawRect(abs, Color(80, 80, 100, 255));

    // Title bar.
    const int titleH = 32;
    Rect titleRect(abs.x, abs.y, abs.w, titleH);
    canvas.FillRect(titleRect, m_titleColor);
    if (font) {
        int ty = abs.y + (titleH - font->GetHeight()) / 2;
        canvas.DrawText(*font, m_title, abs.x + 12, ty, Color::White());
    }

    // Message text (wrapped).
    if (font && !m_message.empty()) {
        Rect msgRect(abs.x + 14, abs.y + titleH + 12,
                     abs.w - 28, abs.h - titleH - 12 - 50);
        canvas.DrawTextWrapped(*font, m_message, msgRect, m_textColor);
    }

    // Render content children (positioned by the host / common props), clipped
    // to the area below the message.
    Rect contentClip(abs.x + 1, abs.y + titleH + 1, abs.w - 2, abs.h - titleH - 2);
    canvas.PushClip(contentClip);
    RenderChildren(canvas, fonts);
    canvas.PopClip();

    // Buttons (outside the content clip so they are never clipped).
    for (int i = 0; i < static_cast<int>(m_buttonInfos.size()); ++i) {
        const auto& b = m_buttonInfos[i];
        Color bg = (i == m_hoveredButton) ? m_buttonHoverColor : m_buttonColor;
        canvas.FillRect(b.rect, bg);
        canvas.DrawRect(b.rect, Color(90, 90, 110, 255));
        if (font) {
            int tw = font->GetTextWidth(b.label);
            int tx = b.rect.x + (b.rect.w - tw) / 2;
            int ty = b.rect.y + (b.rect.h - font->GetHeight()) / 2;
            canvas.DrawText(*font, b.label, tx, ty, m_textColor);
        }
    }
}

} // namespace nui
