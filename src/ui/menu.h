#pragma once
// NUI: Menu / ContextMenu widget
// A vertical list of items (actions, separators, submenus) shown as an overlay
// popup. Opens via Menu::Show(x, y) which pushes the menu onto the application
// overlay stack as a non-modal, close-on-outside-click overlay. Submenus are
// separate Menu instances pushed on top of the same stack.

#include "ui/widget.h"
#include "ui/popup_geometry.h"
#include "renderer/font.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace nui {

class Menu : public Widget {
public:
    Menu();

    struct Item {
        std::string text;
        std::function<void()> action; // fired on click (skipped for separators/disabled)
        std::unique_ptr<Menu> submenu; // non-null ⇒ this item opens a submenu
        bool enabled = true;
        bool separator = false;
        // Opaque id for XML-driven menus; the host resolves it to an action.
        std::string actionId;
    };

    // Build the menu.
    void AddItem(const std::string& text, std::function<void()> action = nullptr);
    // Variant carrying an action id instead of a closure; resolved at click
    // time through the registered ActionResolver. Used by XML-driven menus.
    void AddItemWithId(const std::string& text, const std::string& actionId);
    void AddDisabledItem(const std::string& text);
    void AddSeparator();
    // Takes ownership of `submenu` and associates it with `text`.
    void AddSubmenu(const std::string& text, std::unique_ptr<Menu> submenu);
    void ClearItems();
    int  GetItemCount() const { return static_cast<int>(m_items.size()); }

    // Open a menu at a screen-space point. Takes ownership of `menu` and pushes
    // it onto the overlay stack (non-modal, close-on-outside-click). Use this
    // static entry point so ownership is unambiguous.
    static void Open(std::unique_ptr<Menu> menu, int x, int y, int screenW, int screenH);

    // Configure the anchor/screen size used for positioning. Called by Open().
    void Configure(int x, int y, int screenW, int screenH);

    // Close this menu (remove from overlay stack). Safe to call on a menu that
    // is not currently open.
    void Close();

    // Appearance
    void SetFontSize(int size) { m_fontSize = size; }
    void SetFont(Font* font) { m_font = font; }
    void SetItemHeight(int h) { m_itemHeight = h; }
    void SetMinWidth(int w) { m_minWidth = w; }
    void SetTextColor(const Color& c) { m_textColor = c; }
    void SetHoverColor(const Color& c) { m_hoverColor = c; }
    void SetPanelColor(const Color& c) { m_panelColor = c; }
    void SetSeparatorColor(const Color& c) { m_separatorColor = c; }
    void SetDisabledColor(const Color& c) { m_disabledColor = c; }

    // Resolve an action id (set via XML `action` attribute) into a callback.
    // The host registers a resolver once; items added from XML carry only an id.
    // The resolver is a process-wide singleton — call SetActionResolver from the
    // main thread only (not synchronized for concurrent registration/use).
    using ActionResolver = std::function<std::function<void()>(const std::string& id)>;
    static void SetActionResolver(ActionResolver resolver);

    // Widget overrides
    bool HandleInput(InputState& input) override;
    void Render(Canvas& canvas, FontManager& fonts) override;

private:
    // Measure the widest item to size the panel width.
    int  MeasureWidth(Font& font) const;
    int  MeasureHeight() const;
    Rect GetPanelRect() const;

    std::vector<Item> m_items;
    int  m_hoveredItem = -1;

    int  m_fontSize = 14;
    Font* m_font = nullptr;
    int  m_itemHeight = 26;
    int  m_minWidth = 120;
    int  m_screenW = 0;
    int  m_screenH = 0;

    Color m_textColor      = Color::White();
    Color m_hoverColor     = Color(50, 50, 70, 255);
    Color m_panelColor     = Color(35, 35, 50, 255);
    Color m_separatorColor = Color(70, 70, 90, 255);
    Color m_disabledColor  = Color(120, 120, 135, 255);
};

} // namespace nui
