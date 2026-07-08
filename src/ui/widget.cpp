// NUI: Widget base implementation
// Parent/child tree, event routing, basic rendering.

#include "ui/widget.h"
#include "core/input.h"

#include <algorithm>

namespace nui {

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
