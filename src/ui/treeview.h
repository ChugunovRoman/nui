#pragma once
// NUI: Treeview widget
// Collapsible hierarchical list. Modelled as an in-memory recursive node tree
// (not a widget-per-node) that is flattened on render with indentation per
// nesting level. Uses a vertical scroll offset and a clip, like ScrollView.

#include "ui/widget.h"
#include "renderer/font.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace nui {

class Treeview : public Widget {
public:
    Treeview();

    // A single tree node. Owns its children recursively. `userData` is an
    // opaque application payload (e.g. an entity id) carried with the node.
    struct Node {
        std::string text;
        std::vector<Node> children;
        bool expanded = false;
        bool hasChildren = false; // hint for the expand/collapse glyph; set true
                                  // even if children are loaded lazily.
        std::string userData;

        Node() = default;
        explicit Node(const std::string& t) : text(t) {}
    };

    // Build the tree programmatically. The treeview takes a copy of the root.
    void SetRoot(Node root);
    Node& GetRoot() { return m_root; }
    const Node& GetRoot() const { return m_root; }

    // Currently selected node path (indices from root). Empty when nothing is
    // selected.
    using NodePath = std::vector<int>;
    const NodePath& GetSelectedPath() const { return m_selectedPath; }
    void SetSelectedPath(const NodePath& path);

    // Rebuild the flattened row list from the model. m_flat caches raw Node*
    // into m_root, so ANY mutation of the model performed via GetRoot() (e.g.
    // push_back / erase on a node's children, which can reallocate the vector
    // and move Node objects) invalidates those pointers. Call Rebuild() after
    // every such mutation, otherwise rendering/clicks will use stale pointers.
    void Rebuild() { RebuildFlat(); }

    // Appearance
    void SetFontSize(int size) { m_fontSize = size; }
    void SetFont(Font* font) { m_font = font; }
    void SetTextColor(const Color& c) { m_textColor = c; }
    void SetIndent(int px) { m_indent = px; }
    void SetRowHeight(int h) { m_rowHeight = h; }
    void SetSelectedColor(const Color& c) { m_selectedColor = c; }
    void SetHoverColor(const Color& c) { m_hoverColor = c; }
    void SetGlyphColor(const Color& c) { m_glyphColor = c; }
    // Scrollbar colours (kept consistent with ScrollView's API).
    void SetScrollbarColor(const Color& c) { m_scrollbarColor = c; }
    void SetScrollbarBgColor(const Color& c) { m_scrollbarBgColor = c; }

    // Callbacks
    using NodeSelectedCallback = std::function<void(Widget*, const NodePath&)>;
    void SetOnNodeSelected(NodeSelectedCallback cb) { m_onNodeSelected = std::move(cb); }
    using NodeToggledCallback = std::function<void(Widget*, const NodePath&, bool expanded)>;
    void SetOnNodeToggled(NodeToggledCallback cb) { m_onNodeToggled = std::move(cb); }

    // Widget overrides
    bool HandleInput(InputState& input) override;
    void Render(Canvas& canvas, FontManager& fonts) override;

private:
    // One row of the flattened tree, pointing back into the model.
    struct FlatRow {
        Node* node = nullptr;
        NodePath path;
        int  level = 0;
        bool hasChildren = false;
    };

    // Rebuild m_flat from m_root honouring expanded state.
    void RebuildFlat();
    int  GetMaxScrollY() const;
    // Recursive flattening helper.
    static void FlattenNode(Node* node, const NodePath& basePath, int level,
                            std::vector<FlatRow>& out);

    Node m_root;
    std::vector<FlatRow> m_flat;
    NodePath m_selectedPath;
    int  m_hoveredRow = -1;
    int  m_scrollY = 0;
    bool m_dragging = false;
    int  m_dragStartY = 0;
    int  m_dragStartScroll = 0;

    int  m_fontSize = 14;
    Font* m_font = nullptr;
    int  m_indent = 16;
    int  m_rowHeight = 20;
    int  m_scrollSpeed = 28;

    Color m_textColor       = Color::White();
    Color m_selectedColor   = Color(40, 80, 140, 255);
    Color m_hoverColor      = Color(50, 50, 70, 255);
    Color m_glyphColor      = Color(180, 180, 200, 255);
    Color m_scrollbarColor  = Color(80, 80, 100, 255);
    Color m_scrollbarBgColor = Color(20, 20, 30, 255);

    NodeSelectedCallback m_onNodeSelected;
    NodeToggledCallback  m_onNodeToggled;
};

} // namespace nui
