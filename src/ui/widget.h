#pragma once
// NUI: Widget base class
// Inspired by xrUICore's CUIWindow - parent/child tree, event routing, layout.
// All UI elements inherit from Widget.

#include "renderer/canvas.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace nui {

class InputState;
class FontManager;
class Canvas;

// ── Alignment modes (like xrUICore) ─────────────────────────────
enum class AlignH { Left, Center, Right };
enum class AlignV { Top,  Center, Bottom };

// ── Widget base class ───────────────────────────────────────────
class Widget {
public:
    Widget();
    virtual ~Widget();

    // ── Identity ────────────────────────────────────────────────
    void SetName(const std::string& name) { m_name = name; }
    const std::string& GetName() const { return m_name; }
    void SetType(const std::string& type) { m_type = type; }
    const std::string& GetType() const { return m_type; }

    // ── Geometry ────────────────────────────────────────────────
    void SetRect(int x, int y, int w, int h);
    void SetPos(int x, int y);
    void SetSize(int w, int h);
    const Rect& GetRect() const { return m_rect; }
    int GetX() const { return m_rect.x; }
    int GetY() const { return m_rect.y; }
    int GetWidth()  const { return m_rect.w; }
    int GetHeight() const { return m_rect.h; }

    // Absolute position (including parent offsets)
    Rect GetAbsoluteRect() const;

    // ── Visibility ──────────────────────────────────────────────
    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }

    // ── Alignment ───────────────────────────────────────────────
    void SetAlignH(AlignH a) { m_alignH = a; }
    void SetAlignV(AlignV a) { m_alignV = a; }

    // ── Colors ──────────────────────────────────────────────────
    void SetColor(const Color& c) { m_color = c; }
    const Color& GetColor() const { return m_color; }
    void SetBgColor(const Color& c) { m_bgColor = c; }
    const Color& GetBgColor() const { return m_bgColor; }
    void SetBorderColor(const Color& c) { m_borderColor = c; }

    // ── Parent / Child tree ─────────────────────────────────────
    void AddChild(std::unique_ptr<Widget> child);
    Widget* GetChild(const std::string& name) const;
    const std::vector<std::unique_ptr<Widget>>& GetChildren() const { return m_children; }
    Widget* GetParent() const { return m_parent; }
    void RemoveChild(const std::string& name);
    void ClearChildren();

    // ── Event handling ──────────────────────────────────────────
    // Returns true if the event was consumed
    virtual bool HandleInput(InputState& input);

    // Called every frame with delta time
    virtual void Update(float dt);

    // Draw this widget and all children
    virtual void Render(Canvas& canvas, FontManager& fonts);

    // ── Callbacks ───────────────────────────────────────────────
    using ClickCallback = std::function<void(Widget*)>;
    void SetOnClick(ClickCallback cb) { m_onClick = std::move(cb); }

    // ── User data (for XML-defined properties) ──────────────────
    void SetUserData(const std::string& key, const std::string& value);
    const std::string& GetUserData(const std::string& key) const;

    // ── Tooltip ─────────────────────────────────────────────────
    void SetTooltip(const std::string& text) { m_tooltip = text; }
    const std::string& GetTooltip() const { return m_tooltip; }

    // ── Enabled state ───────────────────────────────────────────
    void SetEnabled(bool enabled) { m_enabled = enabled; }
    bool IsEnabled() const { return m_enabled; }

    // ── Focus ───────────────────────────────────────────────────
    bool IsFocused() const { return m_focused; }
    void ClearFocus();  // Recursively clear focus from this widget and all children

protected:
    // Helper: check if mouse is inside this widget
    bool HitTest(int mx, int my) const;

    // Draw background and border
    void DrawBackground(Canvas& canvas);

    // Render all children
    void RenderChildren(Canvas& canvas, FontManager& fonts);

    // Handle input for all children (returns true if any child consumed it)
    bool HandleChildrenInput(InputState& input);

    // ── Members ─────────────────────────────────────────────────
    std::string  m_name;
    std::string  m_type = "widget";
    Rect         m_rect;
    Color        m_color       = Color::White();
    Color        m_bgColor     = Color::Transparent();
    Color        m_borderColor = Color::Transparent();
    AlignH       m_alignH = AlignH::Left;
    AlignV       m_alignV = AlignV::Top;
    bool         m_visible = true;
    bool         m_enabled = true;
    bool         m_focused = false;
    std::string  m_tooltip;

    Widget*                           m_parent = nullptr;
    std::vector<std::unique_ptr<Widget>> m_children;
    std::map<std::string, std::string>   m_userData;
    ClickCallback                     m_onClick;
};

} // namespace nui
