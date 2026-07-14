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
#include <cstdint>
#include <climits>

namespace nui {

class InputState;
class FontManager;
class Canvas;

// ── Alignment modes (like xrUICore) ─────────────────────────────
enum class AlignH { Left, Center, Right };
enum class AlignV { Top,  Center, Bottom };

// ── Anchor system (responsive layout, v0.4.0) ───────────────────
// Edge flags (WinForms-style). A widget pinned to two opposite edges
// (Left|Right or Top|Bottom) stretches between them. Composable with |.
enum class AnchorFlag : uint8_t {
    None   = 0,
    Left   = 1 << 0,
    Top    = 1 << 1,
    Right  = 1 << 2,
    Bottom = 1 << 3,
};
AnchorFlag operator|(AnchorFlag a, AnchorFlag b) noexcept;
AnchorFlag operator&(AnchorFlag a, AnchorFlag b) noexcept;

// How a dimension is resolved when the parent resizes.
enum class StretchMode {
    Fixed,         // keep declared size
    Fill,          // fill the space between two opposite anchors
    Proportional,  // scale with the parent size (relative to design)
};

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

    // Check whether a screen-space point (mouse coords) is inside this widget.
    bool HitTest(int mx, int my) const;

    // ── Visibility ──────────────────────────────────────────────
    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }

    // ── Alignment ───────────────────────────────────────────────
    void SetAlignH(AlignH a) { m_alignH = a; }
    void SetAlignV(AlignV a) { m_alignV = a; }

    // ── Colors ──────────────────────────────────────────────────
    void SetColor(const Color& c) { if (m_color != c) { m_color = c; MarkDirty(); } }
    const Color& GetColor() const { return m_color; }
    void SetBgColor(const Color& c) { if (m_bgColor != c) { m_bgColor = c; MarkDirty(); } }
    const Color& GetBgColor() const { return m_bgColor; }
    void SetBorderColor(const Color& c) { if (m_borderColor != c) { m_borderColor = c; MarkDirty(); } }

    // ── Parent / Child tree ─────────────────────────────────────
    void AddChild(std::unique_ptr<Widget> child);
    Widget* GetChild(const std::string& name) const;
    const std::vector<std::unique_ptr<Widget>>& GetChildren() const { return m_children; }
    Widget* GetParent() const { return m_parent; }
    void RemoveChild(const std::string& name);
    void ClearChildren();

    // Move this widget to the end of its parent's child list, making it the
    // topmost sibling (last rendered, first to receive input). No-op if this
    // widget has no parent. Used by overlays/submenus to come out on top.
    void Raise();

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
    void ClearFocus();

    // ── Rotation ───────────────────────────────────────────────
    void SetRotation(float degrees);
    float GetRotation() const { return m_rotation; }
    void SetRotationCenter(float cx, float cy); // 0.0-1.0, relative to rect

    // ── Anchor / responsive layout ─────────────────────────────
    // Flag-based anchor (WinForms-style mnemonic). Pins the widget to one or
    // more edges of its parent. Once anchored, the widget's geometry is
    // recomputed automatically on parent resize.
    void SetAnchor(AnchorFlag flags);
    // Explicit normalized anchor points (Godot-style). Each value is in [0,1];
    // pass -1.f for an edge to derive it from the current flags snapshot.
    void SetAnchor(float l, float r, float t, float b);
    AnchorFlag GetAnchorFlags() const { return m_anchorFlags; }
    bool IsAnchored() const { return m_anchored; }

    void SetStretch(StretchMode w, StretchMode h);
    void SetStretchW(StretchMode m);
    void SetStretchH(StretchMode m);
    StretchMode GetStretchW() const { return m_stretchW; }
    StretchMode GetStretchH() const { return m_stretchH; }

    void SetMinSize(int w, int h);
    void SetMaxSize(int w, int h);
    void SetMinWidth(int w)  { SetMinSize(w, m_minH); }
    void SetMinHeight(int h) { SetMinSize(m_minW, h); }
    void SetMaxWidth(int w)  { SetMaxSize(w, m_maxH); }
    void SetMaxHeight(int h) { SetMaxSize(m_maxW, h); }

    // Recompute geometry for this widget (if anchored) and recurse into
    // children. Called by Application on resize / SetRoot; safe to call on any
    // widget at any time.
    virtual void UpdateLayout();

    // Mark this widget and its ancestors as needing a layout pass.
    void MarkLayoutDirty();

protected:
    // Rebuild the offscreen render cache (used for rotation)
    void RebuildRenderCache(FontManager& fonts);
    void FreeRenderCache();

    // Mark widget as needing re-render (for cache invalidation)
    void MarkDirty() { m_renderDirty = true; }

    // Draw background and border
    void DrawBackground(Canvas& canvas);

    // Render all children
    void RenderChildren(Canvas& canvas, FontManager& fonts);

    // Handle input for all children (returns true if any child consumed it)
    bool HandleChildrenInput(InputState& input);

    // Anchor core: recompute m_rect from the anchor configuration and the
    // given parent content size. Called from UpdateLayout().
    void ApplyAnchors(int parentW, int parentH);

    // Write the rect directly, bypassing the layout-dirty bookkeeping but
    // still invalidating the render cache. Used by ApplyAnchors.
    void SetRectUnchecked(int x, int y, int w, int h);

    // Capture the current rect/parent-size as the design snapshot used by
    // ApplyAnchors as the source of truth.
    void CaptureDesignSnapshot(int parentW, int parentH);

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

    // Rotation. Rotation origin (pivot) is in normalized [0,1] rect coords and
    // is applied to the widget's own rect only.
    // COMPOSITION NOTE: when a rotated widget has a rotated child, the child's
    // own rotation is first baked into the parent's render cache (parent caches
    // its subtree), and the parent's rotation is then applied to that cache.
    // Effectively the child is rotated twice (parent_rot + child_rot). This is
    // intended for simple cases; avoid nesting rotated widgets if you need a
    // single rigid rotation of a subtree.
    float        m_rotation = 0.0f;
    float        m_rotationCenterX = 0.5f;
    float        m_rotationCenterY = 0.5f;

    // Render cache for rotation
    SDL_Surface* m_renderCache = nullptr;
    bool         m_renderDirty = true;

    // ── Anchor / responsive layout members ─────────────────────
    // false = static (legacy behaviour: widget keeps its rect as-is).
    bool        m_anchored = false;
    AnchorFlag  m_anchorFlags = AnchorFlag::None;
    StretchMode m_stretchW = StretchMode::Fixed;
    StretchMode m_stretchH = StretchMode::Fixed;

    // Optional explicit normalized anchor points (-1.f = derive from flags).
    float m_anchorLeft = -1.f, m_anchorRight = -1.f;
    float m_anchorTop  = -1.f, m_anchorBottom = -1.f;

    // Design snapshot — captured once on first ApplyAnchors / SetAnchor from
    // the widget's current rect and the parent size. The layout never mutates
    // these; they are the source of truth for offsets / proportional scaling.
    Rect  m_designRect        = Rect(0, 0, 0, 0);
    int   m_designParentW     = 0;
    int   m_designParentH     = 0;
    bool  m_hasDesignSnapshot = false;

    // Explicit pixel offsets vs. the anchor edges (Godot-style margins).
    // INT_MIN = derive from the design snapshot.
    int   m_offLeft = INT_MIN, m_offRight = INT_MIN;
    int   m_offTop  = INT_MIN, m_offBottom = INT_MIN;

    // Size constraints (clamped after layout).
    int   m_minW = 0, m_minH = 0;
    int   m_maxW = INT_MAX, m_maxH = INT_MAX;

    bool  m_layoutDirty = false;

    Widget*                               m_parent = nullptr;
    std::vector<std::unique_ptr<Widget>>  m_children;
    std::map<std::string, std::string>    m_userData;
    ClickCallback                         m_onClick;
};

} // namespace nui
