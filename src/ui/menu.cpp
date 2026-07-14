// NUI: Menu / ContextMenu implementation
// Overlay popup with vertical items, hover highlight, separators and submenus.

#include "ui/menu.h"
#include "core/application.h"
#include "core/input.h"

#include <algorithm>

namespace nui {

// Static action resolver (host-registered) for XML-driven menus.
static Menu::ActionResolver s_actionResolver;
void Menu::SetActionResolver(ActionResolver resolver) {
    s_actionResolver = std::move(resolver);
}

Menu::Menu() {
    m_type = "menu";
    m_bgColor = Color(0, 0, 0, 0); // the panel draws its own background
}

void Menu::AddItem(const std::string& text, std::function<void()> action) {
    Item item;
    item.text = text;
    item.action = std::move(action);
    m_items.push_back(std::move(item));
}

void Menu::AddItemWithId(const std::string& text, const std::string& actionId) {
    Item item;
    item.text = text;
    item.actionId = actionId;
    m_items.push_back(std::move(item));
}

void Menu::AddDisabledItem(const std::string& text) {
    Item item;
    item.text = text;
    item.enabled = false;
    m_items.push_back(std::move(item));
}

void Menu::AddSeparator() {
    Item item;
    item.separator = true;
    m_items.push_back(std::move(item));
}

void Menu::AddSubmenu(const std::string& text, std::unique_ptr<Menu> submenu) {
    Item item;
    item.text = text;
    item.submenu = std::move(submenu);
    m_items.push_back(std::move(item));
}

void Menu::ClearItems() {
    m_items.clear();
    m_hoveredItem = -1;
}

int Menu::MeasureWidth(Font& font) const {
    int w = m_minPanelWidth;
    for (const auto& it : m_items) {
        if (it.separator) continue;
        int tw = font.GetTextWidth(it.text);
        // Reserve room for the submenu arrow glyph + padding.
        tw += 28;
        if (tw > w) w = tw;
    }
    return w;
}

int Menu::MeasureHeight() const {
    int h = 4; // top padding
    for (const auto& it : m_items) {
        h += it.separator ? 9 : m_itemHeight;
    }
    h += 4; // bottom padding
    return h;
}

Rect Menu::GetPanelRect() const {
    return GetAbsoluteRect();
}

void Menu::Configure(int x, int y, int screenW, int screenH) {
    m_screenW = screenW;
    m_screenH = screenH;
    // We need the font to measure width; defer final positioning to Render
    // (where FontManager is available) — but set a reasonable initial rect now.
    int w = std::max(m_minPanelWidth, 120);
    int h = MeasureHeight();
    Rect placed = PopupGeometry::ClampToScreen(Rect(x, y, w, h), screenW, screenH, 4);
    SetRect(placed.x, placed.y, placed.w, placed.h);
}

void Menu::Open(std::unique_ptr<Menu> menu, int x, int y, int screenW, int screenH) {
    if (!menu) return;
    Application* app = GetApp();
    if (!app) return;
    Menu* raw = menu.get();
    raw->Configure(x, y, screenW, screenH);
    // Non-modal, dismiss when the user clicks outside this menu.
    app->PushOverlay(std::move(menu), false, true);
}

void Menu::Close() {
    Application* app = GetApp();
    if (app) app->PopOverlay(this);
}

bool Menu::HandleInput(InputState& input) {
    if (!m_visible || !m_enabled) return false;

    Rect abs = GetAbsoluteRect();
    int mx = input.GetMouseX();
    int my = input.GetMouseY();

    // Determine hovered item (skipping separators).
    m_hoveredItem = -1;
    if (abs.Contains(mx, my)) {
        int y = abs.y + 4;
        for (int i = 0; i < static_cast<int>(m_items.size()); ++i) {
            const Item& it = m_items[i];
            int rowH = it.separator ? 9 : m_itemHeight;
            if (my >= y && my < y + rowH && !it.separator) {
                m_hoveredItem = i;
                break;
            }
            y += rowH;
        }
    }

    if (m_hoveredItem >= 0 && input.IsMouseClicked(MouseButton::Left)) {
        Item& it = m_items[m_hoveredItem];
        if (it.enabled) {
            if (it.submenu) {
                // Open submenu to the right of this item. Ownership transfers
                // to the overlay stack; the submenu is destroyed when closed.
                int yy = abs.y + 4;
                for (int i = 0; i < m_hoveredItem; ++i) {
                    yy += m_items[i].separator ? 9 : m_itemHeight;
                }
                Menu::Open(std::move(it.submenu), abs.x + abs.w, yy, m_screenW, m_screenH);
            } else {
                // Resolve an XML action id lazily, if a resolver is registered.
                std::function<void()> act = it.action;
                if (!act && !it.actionId.empty() && s_actionResolver) {
                    act = s_actionResolver(it.actionId);
                }
                if (act) act();
                // Close the whole menu chain after an action: pop every Menu
                // overlay above this one, then this one itself. Skip non-menu
                // overlays (e.g. a Tooltip) so we don't disturb unrelated UI.
                Application* app = GetApp();
                if (app) {
                    // Walk a copy of pointers since GetOverlays() returns a ref
                    // that is invalidated by PopOverlay.
                    std::vector<Widget*> snapshot;
                    for (const auto& e : app->GetOverlays()) {
                        snapshot.push_back(e.widget.get());
                    }
                    bool sawSelf = false;
                    for (auto sit = snapshot.rbegin(); sit != snapshot.rend(); ++sit) {
                        Widget* top = *sit;
                        if (top->GetType() != "menu") continue;
                        app->PopOverlay(top);
                        if (top == this) { sawSelf = true; break; }
                    }
                    // Defensive: ensure this menu is removed even if the type
                    // check skipped it for some reason.
                    if (!sawSelf) app->PopOverlay(this);
                }
            }
        }
        return true;
    }

    // Consume clicks inside the panel so they don't reach underlying widgets.
    if (abs.Contains(mx, my)) return true;
    return false;
}

void Menu::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;

    Font* font = m_font ? m_font : fonts.GetDefault(m_fontSize);
    if (!font) return;

    // Re-measure width now that we have a font, and reposition if needed.
    int measuredW = MeasureWidth(*font);
    int h = MeasureHeight();
    Rect abs = GetAbsoluteRect();
    if (abs.w != measuredW || abs.h != h) {
        Rect placed = PopupGeometry::ClampToScreen(
            Rect(abs.x, abs.y, measuredW, h), m_screenW, m_screenH, 4);
        SetRect(placed.x, placed.y, placed.w, placed.h);
        abs = GetAbsoluteRect();
    }

    // Panel background + border.
    canvas.FillRect(abs, m_panelColor);
    canvas.DrawRect(abs, Color(70, 70, 90, 255));

    // Items.
    int y = abs.y + 4;
    for (int i = 0; i < static_cast<int>(m_items.size()); ++i) {
        const Item& it = m_items[i];
        if (it.separator) {
            int sy = y + 4;
            canvas.DrawLine(abs.x + 4, sy, abs.x + abs.w - 4, sy, m_separatorColor);
            y += 9;
            continue;
        }
        Rect rowRect(abs.x, y, abs.w, m_itemHeight);
        if (i == m_hoveredItem && it.enabled) {
            canvas.FillRect(rowRect, m_hoverColor);
        }
        Color tc = it.enabled ? m_textColor : m_disabledColor;
        int ty = y + (m_itemHeight - font->GetHeight()) / 2;
        canvas.DrawText(*font, it.text, abs.x + 8, ty, tc);

        // Submenu arrow.
        if (it.submenu) {
            int ax = abs.x + abs.w - 12;
            int ay = y + m_itemHeight / 2;
            canvas.DrawLine(ax, ay - 3, ax + 4, ay, m_textColor);
            canvas.DrawLine(ax + 4, ay, ax, ay + 3, m_textColor);
        }
        y += m_itemHeight;
    }

    RenderChildren(canvas, fonts);
}

} // namespace nui
