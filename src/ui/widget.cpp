// NUI: Widget base implementation
// Parent/child tree, event routing, basic rendering.

#include "ui/widget.h"
#include "core/input.h"

#include <algorithm>

namespace nui {

Widget::Widget() {}

Widget::~Widget() {
    ClearChildren();
}

// ── Geometry ────────────────────────────────────────────────────

void Widget::SetRect(int x, int y, int w, int h) {
    m_rect = Rect(x, y, w, h);
}

void Widget::SetPos(int x, int y) {
    m_rect.x = x;
    m_rect.y = y;
}

void Widget::SetSize(int w, int h) {
    m_rect.w = w;
    m_rect.h = h;
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

void Widget::Update(float dt) {
    if (!m_visible) return;
    for (auto& child : m_children) {
        child->Update(dt);
    }
}

void Widget::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;
    DrawBackground(canvas);
    RenderChildren(canvas, fonts);
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
