// NUI: Treeview implementation
// Flattens the node model into rows and renders them with indentation,
// expand/collapse glyphs, hover/selection highlight and vertical scrolling.

#include "ui/treeview.h"
#include "core/input.h"

#include <algorithm>

namespace nui {

Treeview::Treeview() {
    m_type = "treeview";
    m_bgColor = Color(25, 25, 35, 255);
}

void Treeview::FlattenNode(Node* node, const NodePath& basePath, int level,
                           std::vector<FlatRow>& out) {
    FlatRow row;
    row.node = node;
    row.path = basePath;
    row.level = level;
    row.hasChildren = node->hasChildren || !node->children.empty();
    out.push_back(row);

    if (node->expanded) {
        for (int i = 0; i < static_cast<int>(node->children.size()); ++i) {
            NodePath childPath = basePath;
            childPath.push_back(i);
            FlattenNode(&node->children[i], childPath, level + 1, out);
        }
    }
}

void Treeview::RebuildFlat() {
    m_flat.clear();
    // Root itself is not rendered; only its children are top-level rows.
    for (int i = 0; i < static_cast<int>(m_root.children.size()); ++i) {
        NodePath p;
        p.push_back(i);
        FlattenNode(&m_root.children[i], p, 0, m_flat);
    }
}

void Treeview::SetRoot(Node root) {
    m_root = std::move(root);
    m_selectedPath.clear();
    // Reset view state: the old scroll offset/hover may be out of range for
    // the new (possibly shorter) tree.
    m_scrollY = 0;
    m_hoveredRow = -1;
    m_dragging = false;
    RebuildFlat();
}

void Treeview::SetSelectedPath(const NodePath& path) {
    if (path == m_selectedPath) return;
    m_selectedPath = path;
    if (m_onNodeSelected) m_onNodeSelected(this, m_selectedPath);
}

int Treeview::GetMaxScrollY() const {
    int totalH = static_cast<int>(m_flat.size()) * m_rowHeight;
    return std::max(0, totalH - m_rect.h);
}

bool Treeview::HandleInput(InputState& input) {
    if (!m_visible || !m_enabled) return false;

    Rect abs = GetAbsoluteRect();
    int mx = input.GetMouseX();
    int my = input.GetMouseY();

    // Wheel scrolling.
    int wheel = input.GetWheelY();
    if (wheel != 0 && abs.Contains(mx, my)) {
        m_scrollY -= wheel * m_scrollSpeed;
        m_scrollY = std::max(0, std::min(m_scrollY, GetMaxScrollY()));
        return true;
    }

    // Drag scrollbar (right edge).
    if (input.IsMouseClicked(MouseButton::Left) && abs.Contains(mx, my)) {
        int scrollbarX = abs.x + abs.w - 10;
        if (mx >= scrollbarX) {
            m_dragging = true;
            m_dragStartY = my;
            m_dragStartScroll = m_scrollY;
        }
    }
    if (m_dragging) {
        if (input.IsMouseDown(MouseButton::Left)) {
            int dy = my - m_dragStartY;
            int maxScroll = GetMaxScrollY();
            if (abs.h > 0 && maxScroll > 0) {
                float ratio = static_cast<float>(dy) / abs.h;
                m_scrollY = m_dragStartScroll + static_cast<int>(ratio * maxScroll);
                m_scrollY = std::max(0, std::min(m_scrollY, maxScroll));
            }
        } else {
            m_dragging = false;
        }
        return true;
    }

    // Row hit-testing (accounting for scroll offset).
    m_hoveredRow = -1;
    if (abs.Contains(mx, my)) {
        int relY = my - abs.y + m_scrollY;
        int idx = relY / m_rowHeight;
        if (idx >= 0 && idx < static_cast<int>(m_flat.size())) m_hoveredRow = idx;
    }

    if (m_hoveredRow >= 0 && input.IsMouseClicked(MouseButton::Left)) {
        FlatRow& row = m_flat[m_hoveredRow];
        int rowY = abs.y + m_hoveredRow * m_rowHeight - m_scrollY;
        int glyphX = abs.x + row.level * m_indent;

        // Click on the expand/collapse glyph toggles the node.
        if (row.hasChildren &&
            mx >= glyphX && mx < glyphX + 12 &&
            my >= rowY && my < rowY + m_rowHeight) {
            row.node->expanded = !row.node->expanded;
            RebuildFlat();
            if (m_onNodeToggled) m_onNodeToggled(this, row.path, row.node->expanded);
            return true;
        }

        // Click on the row body selects the node.
        SetSelectedPath(row.path);
        return true;
    }

    // Consume clicks inside our bounds so they don't fall through.
    if (abs.Contains(mx, my) && input.IsMouseClicked(MouseButton::Left)) {
        return true;
    }
    return false;
}

void Treeview::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;
    Rect abs = GetAbsoluteRect();

    canvas.FillRect(abs, m_bgColor);

    Font* font = m_font ? m_font : fonts.GetDefault(m_fontSize);
    int maxScroll = GetMaxScrollY();

    canvas.PushClip(abs);
    for (int i = 0; i < static_cast<int>(m_flat.size()); ++i) {
        const FlatRow& row = m_flat[i];
        int rowY = abs.y + i * m_rowHeight - m_scrollY;
        if (rowY + m_rowHeight < abs.y || rowY > abs.y + abs.h) continue; // culled

        Rect rowRect(abs.x, rowY, abs.w, m_rowHeight);

        // Selection / hover background.
        if (row.path == m_selectedPath) {
            canvas.FillRect(rowRect, m_selectedColor);
        } else if (i == m_hoveredRow) {
            canvas.FillRect(rowRect, m_hoverColor);
        }

        int x = abs.x + row.level * m_indent;

        // Expand/collapse glyph.
        if (row.hasChildren) {
            int gx = x + 2;
            int gy = rowY + m_rowHeight / 2;
            if (row.node->expanded) {
                // Down-pointing triangle.
                canvas.DrawLine(gx, gy - 3, gx + 6, gy - 3, m_glyphColor);
                canvas.DrawLine(gx, gy - 3, gx + 3, gy + 3, m_glyphColor);
                canvas.DrawLine(gx + 6, gy - 3, gx + 3, gy + 3, m_glyphColor);
            } else {
                // Right-pointing triangle.
                canvas.DrawLine(gx, gy - 3, gx, gy + 3, m_glyphColor);
                canvas.DrawLine(gx, gy - 3, gx + 6, gy, m_glyphColor);
                canvas.DrawLine(gx, gy + 3, gx + 6, gy, m_glyphColor);
            }
        }

        // Node text.
        if (font) {
            int ty = rowY + (m_rowHeight - font->GetHeight()) / 2;
            canvas.DrawText(*font, row.node->text, x + 14, ty, m_textColor);
        }
    }
    canvas.PopClip();

    // Scrollbar.
    if (maxScroll > 0) {
        int scrollbarW = 10;
        int scrollbarX = abs.x + abs.w - scrollbarW;
        canvas.FillRect(Rect(scrollbarX, abs.y, scrollbarW, abs.h), m_scrollbarBgColor);
        float viewRatio = static_cast<float>(abs.h) / (abs.h + maxScroll);
        int thumbH = std::min(abs.h, std::max(20, static_cast<int>(abs.h * viewRatio)));
        float scrollRatio = static_cast<float>(m_scrollY) / maxScroll;
        int thumbY = abs.y + static_cast<int>((abs.h - thumbH) * scrollRatio);
        canvas.FillRect(Rect(scrollbarX, thumbY, scrollbarW, thumbH), m_scrollbarColor);
    }

    if (m_borderColor.a > 0) {
        canvas.DrawRect(abs, m_borderColor);
    }

    RenderChildren(canvas, fonts);
}

} // namespace nui
