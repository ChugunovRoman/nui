// NUI: Widget base implementation
// Parent/child tree, event routing, basic rendering.

#include "ui/widget.h"
#include "core/input.h"

#include <algorithm>

namespace nui {

// ── AnchorFlag bitmask operators ────────────────────────────────
AnchorFlag operator|(AnchorFlag a, AnchorFlag b) noexcept {
    return static_cast<AnchorFlag>(
        static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
AnchorFlag operator&(AnchorFlag a, AnchorFlag b) noexcept {
    return static_cast<AnchorFlag>(
        static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

Widget::Widget() {}

Widget::~Widget() {
    ClearChildren();
    FreeRenderCache();
}

void Widget::FreeRenderCache() {
    if (m_renderCache) {
        SDL_FreeSurface(m_renderCache);
        m_renderCache = nullptr;
    }
}

// ── Geometry ────────────────────────────────────────────────────

void Widget::SetRect(int x, int y, int w, int h) {
    if (m_rect.x != x || m_rect.y != y || m_rect.w != w || m_rect.h != h) {
        m_rect = Rect(x, y, w, h);
        MarkDirty();
    }
}

void Widget::SetPos(int x, int y) {
    if (m_rect.x != x || m_rect.y != y) {
        m_rect.x = x;
        m_rect.y = y;
        MarkDirty();
    }
}

void Widget::SetSize(int w, int h) {
    if (m_rect.w != w || m_rect.h != h) {
        m_rect.w = w;
        m_rect.h = h;
        MarkDirty();
    }
}

Rect Widget::GetAbsoluteRect() const {
    if (!m_parent) return m_rect;
    Rect parentAbs = m_parent->GetAbsoluteRect();
    return Rect(parentAbs.x + m_rect.x, parentAbs.y + m_rect.y,
                m_rect.w, m_rect.h);
}

// ── Anchor / responsive layout ──────────────────────────────────

void Widget::CaptureDesignSnapshot(int parentW, int parentH) {
    m_designRect        = m_rect;
    m_designParentW     = parentW;
    m_designParentH     = parentH;
    m_hasDesignSnapshot = true;

    // Derive margins from the design rect. For Godot-style normalized anchors
    // the offset is the margin FROM the anchor point (design_pos - anchor_pt),
    // not the absolute position — otherwise ApplyAnchors double-counts it.
    // For flag-style anchors the offset is the margin from the parent edge.
    if (m_offLeft == INT_MIN) {
        if (m_anchorLeft >= 0.f)
            m_offLeft = m_designRect.x - static_cast<int>(parentW * m_anchorLeft);
        else
            m_offLeft = m_designRect.x;
    }
    if (m_offRight == INT_MIN) {
        int rightEdge = m_designRect.x + m_designRect.w;
        if (m_anchorRight >= 0.f)
            m_offRight = rightEdge - static_cast<int>(parentW * m_anchorRight);
        else
            m_offRight = m_designParentW - rightEdge;
    }
    if (m_offTop == INT_MIN) {
        if (m_anchorTop >= 0.f)
            m_offTop = m_designRect.y - static_cast<int>(parentH * m_anchorTop);
        else
            m_offTop = m_designRect.y;
    }
    if (m_offBottom == INT_MIN) {
        int bottomEdge = m_designRect.y + m_designRect.h;
        if (m_anchorBottom >= 0.f)
            m_offBottom = bottomEdge - static_cast<int>(parentH * m_anchorBottom);
        else
            m_offBottom = m_designParentH - bottomEdge;
    }
}

void Widget::SetRectUnchecked(int x, int y, int w, int h) {
    // Same effect as SetRect, but layout bookkeeping is handled by the caller
    // (UpdateLayout). Still invalidate the render cache (rotation cache).
    if (m_rect.x != x || m_rect.y != y || m_rect.w != w || m_rect.h != h) {
        m_rect = Rect(x, y, w, h);
        MarkDirty();
    }
}

void Widget::SetAnchor(AnchorFlag flags) {
    m_anchored    = true;
    m_anchorFlags = flags;
    // Reset explicit normalized points — the flag set drives the layout now.
    m_anchorLeft = m_anchorRight = m_anchorTop = m_anchorBottom = -1.f;
    MarkLayoutDirty();
    UpdateLayout();
}

void Widget::SetAnchor(float l, float r, float t, float b) {
    m_anchored = true;
    m_anchorLeft   = l;
    m_anchorRight  = r;
    m_anchorTop    = t;
    m_anchorBottom = b;
    // Derive edge flags from the explicit points so that "both opposite edges
    // present" still triggers Fill sizing by default.
    AnchorFlag f = AnchorFlag::None;
    if (l >= 0.f) f = f | AnchorFlag::Left;
    if (r >= 0.f) f = f | AnchorFlag::Right;
    if (t >= 0.f) f = f | AnchorFlag::Top;
    if (b >= 0.f) f = f | AnchorFlag::Bottom;
    m_anchorFlags = f;
    MarkLayoutDirty();
    UpdateLayout();
}

void Widget::SetStretch(StretchMode w, StretchMode h) {
    m_stretchW = w;
    m_stretchH = h;
    MarkLayoutDirty();
    UpdateLayout();
}

void Widget::SetStretchW(StretchMode m) { SetStretch(m, m_stretchH); }
void Widget::SetStretchH(StretchMode m) { SetStretch(m_stretchW, m); }

void Widget::SetMinSize(int w, int h) {
    m_minW = w;
    m_minH = h;
    if (m_anchored) {
        MarkLayoutDirty();
        UpdateLayout();
    }
}

void Widget::SetMaxSize(int w, int h) {
    m_maxW = w;
    m_maxH = h;
    if (m_anchored) {
        MarkLayoutDirty();
        UpdateLayout();
    }
}

void Widget::MarkLayoutDirty() {
    m_layoutDirty = true;
    // Propagate upward so a parent's layout pass reaches this widget.
    if (m_parent) m_parent->MarkLayoutDirty();
}

void Widget::UpdateLayout() {
    if (m_anchored && m_parent) {
        Rect p = m_parent->GetRect();
        ApplyAnchors(p.w, p.h);
    }
    m_layoutDirty = false;
    // Children are laid out relative to this widget's (now up-to-date) rect.
    for (auto& c : m_children) c->UpdateLayout();
}

void Widget::ApplyAnchors(int parentW, int parentH) {
    if (!m_hasDesignSnapshot) CaptureDesignSnapshot(parentW, parentH);

    int newX = m_rect.x, newY = m_rect.y;
    int newW = m_rect.w, newH = m_rect.h;

    const bool hasExplicitH = (m_anchorLeft >= 0.f) || (m_anchorRight >= 0.f);
    const bool hasExplicitV = (m_anchorTop  >= 0.f) || (m_anchorBottom >= 0.f);

    // ── Horizontal ────────────────────────────────────────────
    if (hasExplicitH) {
        // Godot-style: edges at parentW*anchor + offset.
        float aL = (m_anchorLeft  >= 0.f) ? m_anchorLeft  : 0.f;
        float aR = (m_anchorRight >= 0.f) ? m_anchorRight : 1.f;
        int leftEdge   = static_cast<int>(parentW * aL) + m_offLeft;
        int rightEdge  = static_cast<int>(parentW * aR) + m_offRight;
        if (m_anchorLeft >= 0.f && m_anchorRight >= 0.f) {
            // Both edges pinned → width is determined by the span.
            newX = leftEdge;
            newW = rightEdge - leftEdge;
        } else if (m_anchorLeft >= 0.f) {
            // Left edge pinned, keep design width.
            newX = leftEdge;
            newW = m_designRect.w;
        } else {
            // Right edge pinned, keep design width.
            newW = m_designRect.w;
            newX = rightEdge - newW;
        }
    } else {
        const bool aL = (m_anchorFlags & AnchorFlag::Left)  != AnchorFlag::None;
        const bool aR = (m_anchorFlags & AnchorFlag::Right) != AnchorFlag::None;
        if (aL && aR) {
            // Stretch horizontally between fixed left/right margins.
            newX = m_offLeft;
            newW = parentW - m_offLeft - m_offRight;
        } else if (aL) {
            newX = m_offLeft;
            newW = m_designRect.w;
        } else if (aR) {
            newW = m_designRect.w;
            newX = parentW - m_offRight - newW;
        } else {
            // No horizontal anchor → center (keep design width).
            newW = m_designRect.w;
            newX = (parentW - newW) / 2;
        }
    }

    // ── Vertical ──────────────────────────────────────────────
    if (hasExplicitV) {
        float aT = (m_anchorTop    >= 0.f) ? m_anchorTop    : 0.f;
        float aB = (m_anchorBottom >= 0.f) ? m_anchorBottom : 1.f;
        int topEdge    = static_cast<int>(parentH * aT) + m_offTop;
        int bottomEdge = static_cast<int>(parentH * aB) + m_offBottom;
        if (m_anchorTop >= 0.f && m_anchorBottom >= 0.f) {
            newY = topEdge;
            newH = bottomEdge - topEdge;
        } else if (m_anchorTop >= 0.f) {
            newY = topEdge;
            newH = m_designRect.h;
        } else {
            newH = m_designRect.h;
            newY = bottomEdge - newH;
        }
    } else {
        const bool aT = (m_anchorFlags & AnchorFlag::Top)    != AnchorFlag::None;
        const bool aB = (m_anchorFlags & AnchorFlag::Bottom) != AnchorFlag::None;
        if (aT && aB) {
            newY = m_offTop;
            newH = parentH - m_offTop - m_offBottom;
        } else if (aT) {
            newY = m_offTop;
            newH = m_designRect.h;
        } else if (aB) {
            newH = m_designRect.h;
            newY = parentH - m_offBottom - newH;
        } else {
            newH = m_designRect.h;
            newY = (parentH - newH) / 2;
        }
    }

    // ── Stretch modes (override the width/height resolution) ──
    if (m_stretchW == StretchMode::Proportional && m_designParentW > 0) {
        newW = static_cast<int>(m_designRect.w *
                                static_cast<float>(parentW) / m_designParentW);
    } else if (m_stretchW == StretchMode::Fill && !((m_anchorFlags & AnchorFlag::Left) != AnchorFlag::None &&
                                                     (m_anchorFlags & AnchorFlag::Right) != AnchorFlag::None)) {
        // Fill without both horizontal anchors → fill the whole parent width.
        newX = 0;
        newW = parentW;
    }
    if (m_stretchH == StretchMode::Proportional && m_designParentH > 0) {
        newH = static_cast<int>(m_designRect.h *
                                static_cast<float>(parentH) / m_designParentH);
    } else if (m_stretchH == StretchMode::Fill && !((m_anchorFlags & AnchorFlag::Top) != AnchorFlag::None &&
                                                     (m_anchorFlags & AnchorFlag::Bottom) != AnchorFlag::None)) {
        newY = 0;
        newH = parentH;
    }

    // ── Clamp size ────────────────────────────────────────────
    if (newW < m_minW) newW = m_minW;
    if (newH < m_minH) newH = m_minH;
    if (newW > m_maxW) newW = m_maxW;
    if (newH > m_maxH) newH = m_maxH;

    SetRectUnchecked(newX, newY, newW, newH);
}

// ── Parent / Child tree ─────────────────────────────────────────

void Widget::AddChild(std::unique_ptr<Widget> child) {
    if (!child) return;
    child->m_parent = this;
    m_children.push_back(std::move(child));
}

Widget* Widget::GetChild(const std::string& name) const {
    for (auto& child : m_children) {
        if (child->GetName() == name) return child.get();
        // Recursively search
        Widget* found = child->GetChild(name);
        if (found) return found;
    }
    return nullptr;
}

void Widget::RemoveChild(const std::string& name) {
    auto it = std::remove_if(m_children.begin(), m_children.end(),
        [&name](const std::unique_ptr<Widget>& w) { return w->GetName() == name; });
    m_children.erase(it, m_children.end());
}

void Widget::ClearChildren() {
    m_children.clear();
}

void Widget::Raise() {
    if (!m_parent) return;
    auto& siblings = m_parent->m_children;
    for (size_t i = 0; i < siblings.size(); ++i) {
        if (siblings[i].get() == this) {
            // Move this entry to the end (topmost) without invalidating any
            // other unique_ptr ownership — std::rotate shifts pointers.
            std::rotate(siblings.begin() + i, siblings.begin() + i + 1, siblings.end());
            return;
        }
    }
}

// ── Event handling ──────────────────────────────────────────────

bool Widget::HandleInput(InputState& input) {
    if (!m_visible || !m_enabled) return false;
    return HandleChildrenInput(input);
}

bool Widget::HandleChildrenInput(InputState& input) {
    // Process children in reverse order (topmost first)
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->HandleInput(input)) {
            return true;
        }
    }
    return false;
}

void Widget::ClearFocus() {
    m_focused = false;
    for (auto& child : m_children) {
        child->ClearFocus();
    }
}

// ── Rotation ────────────────────────────────────────────────────
// NOTE: rotation and pivot do NOT invalidate the render cache.
// The cache stores the widget's content (background + children) rendered in
// local coordinates; rotation is applied to that finished cache in Render().
// Only content-affecting changes (size, bg color, children) call MarkDirty().

void Widget::SetRotation(float degrees) {
    if (m_rotation != degrees) m_rotation = degrees;
}

void Widget::SetRotationCenter(float cx, float cy) {
    if (m_rotationCenterX != cx || m_rotationCenterY != cy) {
        m_rotationCenterX = cx;
        m_rotationCenterY = cy;
    }
}

void Widget::RebuildRenderCache(FontManager& fonts) {
    if (m_rect.w <= 0 || m_rect.h <= 0) return;

    // Allocate or reallocate cache surface WITH alpha channel
    if (!m_renderCache || m_renderCache->w != m_rect.w || m_renderCache->h != m_rect.h) {
        FreeRenderCache();
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        m_renderCache = SDL_CreateRGBSurface(0, m_rect.w, m_rect.h, 32,
            0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
#else
        m_renderCache = SDL_CreateRGBSurface(0, m_rect.w, m_rect.h, 32,
            0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
#endif
        if (!m_renderCache) return;
    }

    // Clear to transparent
    SDL_FillRect(m_renderCache, nullptr, 0);

    // Render widget subtree to cache surface at LOCAL coordinates (0,0).
    // We temporarily detach from the parent and zero out position so that
    // GetAbsoluteRect() returns (0,0,w,h) for this widget and its subtree,
    // regardless of where it sits in the real tree. Without detaching the
    // parent, nested rotated widgets would render at wrong offsets in cache.
    Canvas cacheCanvas;
    cacheCanvas.Initialize(m_renderCache);

    int origX = m_rect.x;
    int origY = m_rect.y;
    Widget* origParent = m_parent;
    m_rect.x = 0;
    m_rect.y = 0;
    m_parent = nullptr;

    DrawBackground(cacheCanvas);
    RenderChildren(cacheCanvas, fonts);

    // Restore
    m_rect.x = origX;
    m_rect.y = origY;
    m_parent = origParent;

    m_renderDirty = false;
}

void Widget::Update(float dt) {
    if (!m_visible) return;
    for (auto& child : m_children) {
        child->Update(dt);
    }
}

void Widget::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;

    if (m_rotation != 0.0f && m_rect.w > 0 && m_rect.h > 0) {
        // Rotation path: render content to a cache (only when dirty), then
        // rotate the finished cache to the screen. Rebuilding every frame was
        // the main perf bottleneck; the cache only needs refresh on content
        // changes signaled via MarkDirty().
        if (m_renderDirty) {
            RebuildRenderCache(fonts);
        }
        if (m_renderCache) {
            Rect abs = GetAbsoluteRect();
            // Pivot point in screen coords
            float pivotX = abs.x + abs.w * m_rotationCenterX;
            float pivotY = abs.y + abs.h * m_rotationCenterY;
            canvas.DrawSurfaceRotated(m_renderCache, abs.x, abs.y,
                                       m_rotation, pivotX, pivotY);
        }
    } else {
        // Fast path: direct render to screen
        DrawBackground(canvas);
        RenderChildren(canvas, fonts);
    }
}

void Widget::DrawBackground(Canvas& canvas) {
    if (m_bgColor.a > 0) {
        Rect abs = GetAbsoluteRect();
        canvas.FillRect(abs, m_bgColor);
    }
    if (m_borderColor.a > 0) {
        Rect abs = GetAbsoluteRect();
        canvas.DrawRect(abs, m_borderColor);
    }
}

void Widget::RenderChildren(Canvas& canvas, FontManager& fonts) {
    for (auto& child : m_children) {
        child->Render(canvas, fonts);
    }
}

// ── Hit testing ─────────────────────────────────────────────────

bool Widget::HitTest(int mx, int my) const {
    Rect abs = GetAbsoluteRect();
    return abs.Contains(mx, my);
}

Widget* Widget::FindWidgetAt(int x, int y) {
    if (!m_visible) return nullptr;
    // Reverse order: topmost child first (last child = highest z-order)
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        Widget* found = (*it)->FindWidgetAt(x, y);
        if (found) return found;
    }
    // Leaf or no child hit — check self
    if (GetAbsoluteRect().Contains(x, y)) return this;
    return nullptr;
}

// ── User data ───────────────────────────────────────────────────

void Widget::SetUserData(const std::string& key, const std::string& value) {
    m_userData[key] = value;
}

const std::string& Widget::GetUserData(const std::string& key) const {
    static const std::string empty;
    auto it = m_userData.find(key);
    return (it != m_userData.end()) ? it->second : empty;
}

} // namespace nui
