#include "core/log.h"
// NUI: XML Layout Loader implementation
// Uses pugixml to parse XML layout files and instantiate widgets.

#include "xml/layout_loader.h"
#include "ui/widget.h"
#include "ui/label.h"
#include "ui/button.h"
#include "ui/image.h"
#include "ui/editbox.h"
#include "ui/progressbar.h"
#include "ui/scrollview.h"
#include "ui/slider.h"
#include "ui/checkbox.h"
#include "ui/radiobutton.h"
#include "ui/dropdown.h"
#include "ui/tabcontrol.h"
#include "ui/treeview.h"
#include "ui/menu.h"
#include "ui/dialog.h"
#include "renderer/texture.h"
#include "renderer/font.h"
#include "renderer/resource.h"

#include <cstdio>
#include <cstring>
#include <climits>
#include <algorithm>
#include <sstream>

namespace nui {

LayoutLoader::LayoutLoader() {}
LayoutLoader::~LayoutLoader() {}

// ── Color parsing ───────────────────────────────────────────────

static uint8_t ClampByte(int v) {
    return static_cast<uint8_t>((std::max)(0, (std::min)(255, v)));
}

Color LayoutLoader::ParseColor(const std::string& str) const {
    auto it = m_colorDefs.find(str);
    if (it != m_colorDefs.end()) return it->second;

    std::istringstream ss(str);
    int r, g, b, a = 255;
    char c1, c2;
    if (ss >> r >> c1 >> g >> c2 >> b) {
        if (c1 != ',' || c2 != ',') return Color();
        char c3;
        if (ss >> c3 >> a) {
            if (c3 != ',') return Color();
        }
        return Color(ClampByte(r), ClampByte(g), ClampByte(b), ClampByte(a));
    }
    return Color();
}

// ── Anchor / stretch parsing ────────────────────────────────────

AnchorFlag LayoutLoader::ParseAnchorFlags(const char* str) {
    // Tokens are space-separated. Recognized: left/right/top/bottom/center/
    // all/fill. "center" → no edge flags (center mode); "all"/"fill" → all
    // four edges (full stretch).
    AnchorFlag flags = AnchorFlag::None;
    std::istringstream ss(str);
    std::string tok;
    while (ss >> tok) {
        if      (tok == "left")   flags = flags | AnchorFlag::Left;
        else if (tok == "top")    flags = flags | AnchorFlag::Top;
        else if (tok == "right")  flags = flags | AnchorFlag::Right;
        else if (tok == "bottom") flags = flags | AnchorFlag::Bottom;
        else if (tok == "all" || tok == "fill")
            flags = AnchorFlag::Left | AnchorFlag::Top |
                    AnchorFlag::Right | AnchorFlag::Bottom;
        else if (tok == "center")
            flags = AnchorFlag::None; // center mode: no pinned edges
        // Unknown tokens are silently ignored (forward-compat).
    }
    return flags;
}

StretchMode LayoutLoader::ParseStretch(const char* str) {
    if (!str) return StretchMode::Fixed;
    if (strcmp(str, "fill") == 0)         return StretchMode::Fill;
    if (strcmp(str, "proportional") == 0) return StretchMode::Proportional;
    return StretchMode::Fixed; // "fixed" / unknown
}

void LayoutLoader::LoadColorDefs(const std::string& path) {
    pugi::xml_document doc;
    if (!doc.load_file(path.c_str())) {
        NUI_LOG_ERROR( "[NUI] Failed to load color defs: %s\n", path.c_str());
        return;
    }
    for (auto& node : doc.child("colors").children("color")) {
        std::string name = node.attribute("name").as_string();
        std::string value = node.attribute("value").as_string();
        if (!name.empty() && !value.empty())
            m_colorDefs[name] = ParseColor(value);
    }
}

Color LayoutLoader::GetColorByName(const std::string& name) const {
    auto it = m_colorDefs.find(name);
    return (it != m_colorDefs.end()) ? it->second : Color::White();
}

// ── Layout loading ──────────────────────────────────────────────

std::unique_ptr<Widget> LayoutLoader::LoadFromFile(const std::string& path,
                                                     TextureCache& textures,
                                                     FontManager& fonts) {
    pugi::xml_document doc;

    // Try embedded resource first
    ResourceData res = ResourceManager::Load(path);
    if (res.valid()) {
        pugi::xml_parse_result result = doc.load_buffer(res.data, res.size);
        if (!result) {
            NUI_LOG_ERROR( "[NUI] XML parse error in embedded %s: %s\n",
                         path.c_str(), result.description());
            return nullptr;
        }
    } else {
        pugi::xml_parse_result result = doc.load_file(path.c_str());
        if (!result) {
            NUI_LOG_ERROR( "[NUI] XML parse error in %s: %s\n",
                         path.c_str(), result.description());
            return nullptr;
        }
    }

    pugi::xml_node root = doc.child("layout");
    if (!root) root = doc.first_child();
    if (!root) return nullptr;
    return ParseWidget(root, textures, fonts);
}

std::unique_ptr<Widget> LayoutLoader::LoadFromString(const std::string& xml,
                                                       TextureCache& textures,
                                                       FontManager& fonts) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(xml.c_str());
    if (!result) {
        NUI_LOG_ERROR( "[NUI] XML parse error: %s\n", result.description());
        return nullptr;
    }
    pugi::xml_node root = doc.child("layout");
    if (!root) root = doc.first_child();
    if (!root) return nullptr;
    return ParseWidget(root, textures, fonts);
}

// ── Widget parsing ──────────────────────────────────────────────

std::unique_ptr<Widget> LayoutLoader::ParseWidget(pugi::xml_node node,
                                                    TextureCache& textures,
                                                    FontManager& fonts) {
    std::string type = node.name();
    std::unique_ptr<Widget> widget;

    if (type == "layout" || type == "window" || type == "panel") {
        widget = std::make_unique<Widget>();
    } else if (type == "label" || type == "text") {
        auto label = std::make_unique<Label>();
        ParseLabel(node, label.get(), fonts);
        widget = std::move(label);
    } else if (type == "button") {
        auto btn = std::make_unique<Button>();
        ParseButton(node, btn.get(), fonts);
        widget = std::move(btn);
    } else if (type == "image" || type == "sprite") {
        auto img = std::make_unique<Image>();
        ParseImage(node, img.get(), textures);
        widget = std::move(img);
    } else if (type == "editbox" || type == "input") {
        auto edit = std::make_unique<EditBox>();
        ParseEditBox(node, edit.get(), fonts);
        widget = std::move(edit);
    } else if (type == "progressbar" || type == "progress") {
        auto pb = std::make_unique<ProgressBar>();
        ParseProgressBar(node, pb.get(), fonts);
        widget = std::move(pb);
    } else if (type == "scrollview" || type == "scroll") {
        auto sv = std::make_unique<ScrollView>();
        ParseScrollView(node, sv.get());
        widget = std::move(sv);
    } else if (type == "slider") {
        auto sl = std::make_unique<Slider>();
        ParseSlider(node, sl.get());
        widget = std::move(sl);
    } else if (type == "checkbox") {
        auto cb = std::make_unique<CheckBox>();
        ParseCheckBox(node, cb.get(), fonts);
        widget = std::move(cb);
    } else if (type == "radiobutton") {
        auto rb = std::make_unique<RadioButton>();
        ParseRadioButton(node, rb.get(), fonts);
        widget = std::move(rb);
    } else if (type == "dropdown") {
        auto dd = std::make_unique<Dropdown>();
        ParseDropdown(node, dd.get(), fonts);
        widget = std::move(dd);
    } else if (type == "tabcontrol") {
        auto tc = std::make_unique<TabControl>();
        ParseTabControl(node, tc.get(), textures, fonts);
        widget = std::move(tc);
    } else if (type == "treeview") {
        auto tv = std::make_unique<Treeview>();
        ParseTreeview(node, tv.get(), fonts);
        widget = std::move(tv);
    } else if (type == "menu" || type == "contextmenu") {
        auto m = std::make_unique<Menu>();
        ParseMenu(node, m.get(), fonts);
        widget = std::move(m);
    } else if (type == "dialog" || type == "messagebox") {
        auto d = std::make_unique<Dialog>();
        ParseDialog(node, d.get(), textures, fonts);
        widget = std::move(d);
    } else {
        widget = std::make_unique<Widget>();
    }

    ParseCommonProps(widget.get(), node);

    // Auto-recurse into child XML nodes and attach them as widget children.
    // Some widgets consume specific child tags themselves (handled inside their
    // ParseXxx); skip those here so they are not duplicated as empty children.
    for (auto& childNode : node.children()) {
        std::string childType = childNode.name();
        if (IsConsumedChildTag(type, childType)) continue;
        auto child = ParseWidget(childNode, textures, fonts);
        if (child)
            widget->AddChild(std::move(child));
    }
    return widget;
}

// Returns true when a parent widget type handles `childTag` itself inside its
// ParseXxx (so the generic recursion should skip it). Without this, e.g. a
// <dropdown>'s <item> tags would also create stray empty child widgets.
bool LayoutLoader::IsConsumedChildTag(const std::string& parentType,
                                      const std::string& childTag) {
    if (parentType == "dropdown" && childTag == "item") return true;
    if (parentType == "tabcontrol" && childTag == "tab") return true;
    if (parentType == "treeview" && childTag == "node") return true;
    if ((parentType == "menu" || parentType == "contextmenu") &&
        (childTag == "item" || childTag == "separator")) return true;
    if (parentType == "dialog" && childTag == "button") return true;
    return false;
}

void LayoutLoader::ParseCommonProps(Widget* widget, pugi::xml_node n) {
    const char* name = n.attribute("name").as_string(nullptr);
    if (name) widget->SetName(name);

    int x = n.attribute("x").as_int(0);
    int y = n.attribute("y").as_int(0);
    int w = n.attribute("width").as_int(0);
    int h = n.attribute("height").as_int(0);
    widget->SetRect(x, y, w, h);

    if (n.attribute("visible"))
        widget->SetVisible(n.attribute("visible").as_bool(true));
    if (n.attribute("enabled"))
        widget->SetEnabled(n.attribute("enabled").as_bool(true));

    const char* bgColor = n.attribute("bg_color").as_string(nullptr);
    if (bgColor) widget->SetBgColor(ParseColor(bgColor));

    const char* borderColor = n.attribute("border_color").as_string(nullptr);
    if (borderColor) widget->SetBorderColor(ParseColor(borderColor));

    const char* color = n.attribute("color").as_string(nullptr);
    if (color) widget->SetColor(ParseColor(color));

    const char* alignH = n.attribute("align_h").as_string(nullptr);
    if (alignH) {
        if (strcmp(alignH, "center") == 0) widget->SetAlignH(AlignH::Center);
        else if (strcmp(alignH, "right") == 0) widget->SetAlignH(AlignH::Right);
    }
    const char* alignV = n.attribute("align_v").as_string(nullptr);
    if (alignV) {
        if (strcmp(alignV, "center") == 0) widget->SetAlignV(AlignV::Center);
        else if (strcmp(alignV, "bottom") == 0) widget->SetAlignV(AlignV::Bottom);
    }

    const char* tooltip = n.attribute("tooltip").as_string(nullptr);
    if (tooltip) widget->SetTooltip(tooltip);

    // ── Anchor / responsive layout ────────────────────────────
    // Order matters: stretch/min/max must be applied BEFORE SetAnchor so the
    // immediate UpdateLayout() inside SetAnchor honours the constraints.
    // Note: ParseCommonProps runs before AddChild, so the widget has no parent
    // yet — the live recomputation happens later when Application calls
    // UpdateLayout() on the root after loading.
    const char* sw = n.attribute("stretch_w").as_string(nullptr);
    const char* sh = n.attribute("stretch_h").as_string(nullptr);
    if (sw) widget->SetStretchW(ParseStretch(sw));
    if (sh) widget->SetStretchH(ParseStretch(sh));

    if (n.attribute("min_width") || n.attribute("min_height")) {
        widget->SetMinSize(n.attribute("min_width").as_int(0),
                           n.attribute("min_height").as_int(0));
    }
    if (n.attribute("max_width") || n.attribute("max_height")) {
        // Default to a very large value (effectively unbounded) when one of the
        // max_* attributes is omitted, matching the SetMaxSize default.
        int mw = n.attribute("max_width").as_int(INT_MAX);
        int mh = n.attribute("max_height").as_int(INT_MAX);
        widget->SetMaxSize(mw, mh);
    }

    const char* anchor = n.attribute("anchor").as_string(nullptr);
    if (anchor) {
        widget->SetAnchor(ParseAnchorFlags(anchor));
    }
    // Explicit normalized anchor points (Godot-style) override the flag set.
    if (n.attribute("anchor_left") || n.attribute("anchor_right") ||
        n.attribute("anchor_top")  || n.attribute("anchor_bottom")) {
        widget->SetAnchor(
            n.attribute("anchor_left").as_float(-1.f),
            n.attribute("anchor_right").as_float(-1.f),
            n.attribute("anchor_top").as_float(-1.f),
            n.attribute("anchor_bottom").as_float(-1.f));
    }
}

void LayoutLoader::ParseLabel(pugi::xml_node n, Widget* widget, FontManager& fonts) {
    auto* label = static_cast<Label*>(widget);
    const char* text = n.attribute("text").as_string(nullptr);
    if (text) label->SetText(text);

    int fontSize = n.attribute("font_size").as_int(16);
    label->SetFontSize(fontSize);

    const char* fontPath = n.attribute("font").as_string(nullptr);
    if (fontPath) {
        Font* font = fonts.Get(fontPath, fontSize);
        if (font) label->SetFont(font);
    }

    const char* textColor = n.attribute("text_color").as_string(nullptr);
    if (textColor) label->SetTextColor(ParseColor(textColor));

    if (n.attribute("word_wrap"))
        label->SetWordWrap(n.attribute("word_wrap").as_bool(false));
}

void LayoutLoader::ParseButton(pugi::xml_node n, Widget* widget, FontManager& fonts) {
    auto* btn = static_cast<Button*>(widget);
    const char* text = n.attribute("text").as_string(nullptr);
    if (text) btn->SetText(text);

    int fontSize = n.attribute("font_size").as_int(16);
    btn->SetFontSize(fontSize);

    const char* fontPath = n.attribute("font").as_string(nullptr);
    if (fontPath) {
        Font* font = fonts.Get(fontPath, fontSize);
        if (font) btn->SetFont(font);
    }

    const char* textColor = n.attribute("text_color").as_string(nullptr);
    if (textColor) btn->SetTextColor(ParseColor(textColor));

    const char* hoverColor = n.attribute("hover_color").as_string(nullptr);
    if (hoverColor) btn->SetHoverColor(ParseColor(hoverColor));

    const char* pressedColor = n.attribute("pressed_color").as_string(nullptr);
    if (pressedColor) btn->SetPressedColor(ParseColor(pressedColor));
}

void LayoutLoader::ParseImage(pugi::xml_node n, Widget* widget, TextureCache& textures) {
    auto* img = static_cast<Image*>(widget);
    const char* src = n.attribute("src").as_string(nullptr);
    if (src) img->LoadFromFile(src, textures);

    const char* scale = n.attribute("scale").as_string(nullptr);
    if (scale) {
        if (strcmp(scale, "stretch") == 0) img->SetScaleMode(ScaleMode::Stretch);
        else if (strcmp(scale, "fit") == 0) img->SetScaleMode(ScaleMode::Fit);
        else if (strcmp(scale, "fill") == 0) img->SetScaleMode(ScaleMode::Fill);
        else if (strcmp(scale, "center") == 0) img->SetScaleMode(ScaleMode::Center);
        else if (strcmp(scale, "tile") == 0) img->SetScaleMode(ScaleMode::Tile);
    }
}

void LayoutLoader::ParseEditBox(pugi::xml_node n, Widget* widget, FontManager& fonts) {
    auto* edit = static_cast<EditBox*>(widget);
    const char* placeholder = n.attribute("placeholder").as_string(nullptr);
    if (placeholder) edit->SetPlaceholder(placeholder);

    const char* text = n.attribute("text").as_string(nullptr);
    if (text) edit->SetText(text);

    int fontSize = n.attribute("font_size").as_int(16);
    edit->SetFontSize(fontSize);

    const char* fontPath = n.attribute("font").as_string(nullptr);
    if (fontPath) {
        Font* font = fonts.Get(fontPath, fontSize);
        if (font) edit->SetFont(font);
    }

    if (n.attribute("max_length"))
        edit->SetMaxLength(n.attribute("max_length").as_int(0));
    if (n.attribute("password"))
        edit->SetPasswordMode(n.attribute("password").as_bool(false));
}

void LayoutLoader::ParseProgressBar(pugi::xml_node n, Widget* widget, FontManager& /*fonts*/) {
    auto* pb = static_cast<ProgressBar*>(widget);
    float value = n.attribute("value").as_float(0.0f);
    pb->SetValue(value);

    const char* fillColor = n.attribute("fill_color").as_string(nullptr);
    if (fillColor) pb->SetFillColor(ParseColor(fillColor));

    if (n.attribute("show_percent"))
        pb->SetShowPercent(n.attribute("show_percent").as_bool(false));

    const char* label = n.attribute("label").as_string(nullptr);
    if (label) pb->SetLabel(label);
}

void LayoutLoader::ParseScrollView(pugi::xml_node n, Widget* widget) {
    auto* sv = static_cast<ScrollView*>(widget);
    const char* sbColor = n.attribute("scrollbar_color").as_string(nullptr);
    if (sbColor) sv->SetScrollbarColor(ParseColor(sbColor));
}

void LayoutLoader::ParseSlider(pugi::xml_node n, Widget* widget) {
    auto* sl = static_cast<Slider*>(widget);

    // Resolve range first so the value attribute can be interpreted in the
    // caller's units (e.g. min="0" max="100" value="50" → 0.5 internally).
    // Without this, value="50" would be clamped to 1.0 and ignore min/max.
    float minVal = n.attribute("min").as_float(0.0f);
    float maxVal = n.attribute("max").as_float(1.0f);
    sl->SetRange(minVal, maxVal);

    if (n.attribute("value")) {
        float rawValue = n.attribute("value").as_float(0.5f);
        // If a non-trivial range is set, convert from [min,max] to [0,1].
        // Otherwise treat value as already-normalized (legacy behaviour).
        if (n.attribute("min") || n.attribute("max")) {
            float range = maxVal - minVal;
            if (range != 0.0f) {
                sl->SetValue((rawValue - minVal) / range);
            } else {
                sl->SetValue(rawValue);
            }
        } else {
            sl->SetValue(rawValue);
        }
    }

    const char* fillColor = n.attribute("fill_color").as_string(nullptr);
    if (fillColor) sl->SetFillColor(ParseColor(fillColor));

    const char* trackColor = n.attribute("track_color").as_string(nullptr);
    if (trackColor) sl->SetTrackColor(ParseColor(trackColor));

    const char* thumbColor = n.attribute("thumb_color").as_string(nullptr);
    if (thumbColor) sl->SetThumbColor(ParseColor(thumbColor));
}

void LayoutLoader::ParseCheckBox(pugi::xml_node n, Widget* widget, FontManager& fonts) {
    auto* cb = static_cast<CheckBox*>(widget);

    const char* text = n.attribute("text").as_string(nullptr);
    if (text) cb->SetText(text);

    if (n.attribute("checked"))
        cb->SetChecked(n.attribute("checked").as_bool(false));

    int fontSize = n.attribute("font_size").as_int(14);
    cb->SetFontSize(fontSize);

    const char* fontPath = n.attribute("font").as_string(nullptr);
    if (fontPath) {
        Font* font = fonts.Get(fontPath, fontSize);
        if (font) cb->SetFont(font);
    }

    const char* textColor = n.attribute("text_color").as_string(nullptr);
    if (textColor) cb->SetTextColor(ParseColor(textColor));

    const char* checkColor = n.attribute("check_color").as_string(nullptr);
    if (checkColor) cb->SetCheckColor(ParseColor(checkColor));
}

void LayoutLoader::ParseRadioButton(pugi::xml_node n, Widget* widget, FontManager& fonts) {
    auto* rb = static_cast<RadioButton*>(widget);

    const char* text = n.attribute("text").as_string(nullptr);
    if (text) rb->SetText(text);

    const char* group = n.attribute("group").as_string(nullptr);
    if (group) rb->SetGroup(group);

    if (n.attribute("selected"))
        rb->SetSelected(n.attribute("selected").as_bool(false));

    int fontSize = n.attribute("font_size").as_int(14);
    rb->SetFontSize(fontSize);

    const char* fontPath = n.attribute("font").as_string(nullptr);
    if (fontPath) {
        Font* font = fonts.Get(fontPath, fontSize);
        if (font) rb->SetFont(font);
    }

    const char* textColor = n.attribute("text_color").as_string(nullptr);
    if (textColor) rb->SetTextColor(ParseColor(textColor));

    const char* dotColor = n.attribute("dot_color").as_string(nullptr);
    if (dotColor) rb->SetDotColor(ParseColor(dotColor));
}

void LayoutLoader::ParseDropdown(pugi::xml_node n, Widget* widget, FontManager& fonts) {
    auto* dd = static_cast<Dropdown*>(widget);

    int fontSize = n.attribute("font_size").as_int(14);
    dd->SetFontSize(fontSize);

    const char* fontPath = n.attribute("font").as_string(nullptr);
    if (fontPath) {
        Font* font = fonts.Get(fontPath, fontSize);
        if (font) dd->SetFont(font);
    }

    const char* textColor = n.attribute("text_color").as_string(nullptr);
    if (textColor) dd->SetTextColor(ParseColor(textColor));

    if (n.attribute("item_height"))
        dd->SetItemHeight(n.attribute("item_height").as_int(28));

    if (n.attribute("max_visible"))
        dd->SetMaxVisibleItems(n.attribute("max_visible").as_int(5));

    // Parse <item> children
    for (auto& item : n.children("item")) {
        const char* text = item.text().get();
        if (text) dd->AddItem(text);
    }
}

void LayoutLoader::ParseTabControl(pugi::xml_node n, Widget* widget,
                                   TextureCache& textures, FontManager& fonts) {
    auto* tc = static_cast<TabControl*>(widget);

    if (n.attribute("font_size"))
        tc->SetFontSize(n.attribute("font_size").as_int(14));
    if (n.attribute("tab_height"))
        tc->SetTabHeight(n.attribute("tab_height").as_int(28));
    if (n.attribute("active_tab"))
        tc->SetActiveTab(n.attribute("active_tab").as_int(0));
    if (n.attribute("text_color"))
        tc->SetTextColor(ParseColor(n.attribute("text_color").as_string()));
    if (n.attribute("active_color"))
        tc->SetActiveTabColor(ParseColor(n.attribute("active_color").as_string()));
    if (n.attribute("tab_color"))
        tc->SetTabBgColor(ParseColor(n.attribute("tab_color").as_string()));
    if (n.attribute("hover_color"))
        tc->SetHoverTabColor(ParseColor(n.attribute("hover_color").as_string()));
    if (n.attribute("content_color"))
        tc->SetContentBgColor(ParseColor(n.attribute("content_color").as_string()));

    // Each <tab title="..."> holds a page built from its own child widgets.
    // Build a transparent panel as the page and pack the tab's children into it.
    for (auto& tabNode : n.children("tab")) {
        std::string title = tabNode.attribute("title").as_string("Tab");
        auto page = std::make_unique<Widget>();
        for (auto& childNode : tabNode.children()) {
            auto child = ParseWidget(childNode, textures, fonts);
            if (child) page->AddChild(std::move(child));
        }
        tc->AddTab(title, std::move(page));
    }
}

// Recursive helper: parse a <node> tag (and its nested <node> children) into a
// Treeview::Node. Attributes: text, expanded, user_data.
static Treeview::Node ParseTreeNode(pugi::xml_node nodeNode) {
    Treeview::Node node;
    node.text = nodeNode.attribute("text").as_string("");
    if (nodeNode.attribute("expanded"))
        node.expanded = nodeNode.attribute("expanded").as_bool(false);
    if (nodeNode.attribute("user_data"))
        node.userData = nodeNode.attribute("user_data").as_string();
    for (auto& childNode : nodeNode.children("node")) {
        node.children.push_back(ParseTreeNode(childNode));
    }
    node.hasChildren = !node.children.empty();
    return node;
}

void LayoutLoader::ParseTreeview(pugi::xml_node n, Widget* widget, FontManager& fonts) {
    (void)fonts;
    auto* tv = static_cast<Treeview*>(widget);

    if (n.attribute("font_size"))
        tv->SetFontSize(n.attribute("font_size").as_int(14));
    if (n.attribute("row_height"))
        tv->SetRowHeight(n.attribute("row_height").as_int(20));
    if (n.attribute("indent"))
        tv->SetIndent(n.attribute("indent").as_int(16));
    if (n.attribute("text_color"))
        tv->SetTextColor(ParseColor(n.attribute("text_color").as_string()));
    if (n.attribute("selected_color"))
        tv->SetSelectedColor(ParseColor(n.attribute("selected_color").as_string()));
    if (n.attribute("hover_color"))
        tv->SetHoverColor(ParseColor(n.attribute("hover_color").as_string()));

    // Build the model from top-level <node> children.
    Treeview::Node root;
    for (auto& nodeNode : n.children("node")) {
        root.children.push_back(ParseTreeNode(nodeNode));
    }
    root.hasChildren = !root.children.empty();
    tv->SetRoot(std::move(root));
}

// Recursive helper: build a Menu (with nested submenus) from a <menu> tag.
static void ParseMenuItems(pugi::xml_node parent, Menu* menu, FontManager& fonts) {
    (void)fonts;
    for (auto& child : parent.children()) {
        std::string name = child.name();
        if (name == "separator") {
            menu->AddSeparator();
        } else if (name == "item") {
            std::string text = child.attribute("text").as_string("");
            std::string actionId = child.attribute("action").as_string("");
            bool enabled = child.attribute("enabled").as_bool(true);
            if (!enabled) {
                menu->AddDisabledItem(text);
            } else if (!actionId.empty()) {
                // Defer the actual callback to Menu::SetActionResolver at click
                // time; carry the id only.
                menu->AddItemWithId(text, actionId);
            } else {
                menu->AddItem(text);
            }
        } else if (name == "menu" || name == "submenu") {
            std::string text = child.attribute("text").as_string("");
            auto sub = std::make_unique<Menu>();
            ParseMenuItems(child, sub.get(), fonts);
            menu->AddSubmenu(text, std::move(sub));
        }
    }
}

void LayoutLoader::ParseMenu(pugi::xml_node n, Widget* widget, FontManager& fonts) {
    auto* menu = static_cast<Menu*>(widget);

    if (n.attribute("font_size"))
        menu->SetFontSize(n.attribute("font_size").as_int(14));
    if (n.attribute("item_height"))
        menu->SetItemHeight(n.attribute("item_height").as_int(26));
    // Min panel width. Prefer min_panel_width; accept the legacy min_width
    // attribute as an alias (it is also parsed as a common size-constraint
    // by ParseCommonProps, which is harmless here).
    if (n.attribute("min_panel_width"))
        menu->SetMinPanelWidth(n.attribute("min_panel_width").as_int(120));
    else if (n.attribute("min_width"))
        menu->SetMinPanelWidth(n.attribute("min_width").as_int(120));
    if (n.attribute("text_color"))
        menu->SetTextColor(ParseColor(n.attribute("text_color").as_string()));
    if (n.attribute("hover_color"))
        menu->SetHoverColor(ParseColor(n.attribute("hover_color").as_string()));
    if (n.attribute("panel_color"))
        menu->SetPanelColor(ParseColor(n.attribute("panel_color").as_string()));

    ParseMenuItems(n, menu, fonts);
}

void LayoutLoader::ParseDialog(pugi::xml_node n, Widget* widget,
                               TextureCache& textures, FontManager& fonts) {
    auto* dlg = static_cast<Dialog*>(widget);

    if (n.attribute("title"))
        dlg->SetTitle(n.attribute("title").as_string(""));
    if (n.attribute("message"))
        dlg->SetMessage(n.attribute("message").as_string(""));
    if (n.attribute("font_size"))
        dlg->SetFontSize(n.attribute("font_size").as_int(14));

    if (n.attribute("buttons")) {
        std::string b = n.attribute("buttons").as_string("ok");
        DialogButtons db = DialogButtons::Ok;
        if (b == "ok")            db = DialogButtons::Ok;
        else if (b == "okcancel") db = DialogButtons::OkCancel;
        else if (b == "yesno")    db = DialogButtons::YesNo;
        else if (b == "yesnocancel") db = DialogButtons::YesNoCancel;
        else if (b == "retrycancel") db = DialogButtons::RetryCancel;
        dlg->SetButtons(db);
    }

    if (n.attribute("title_color"))
        dlg->SetTitleColor(ParseColor(n.attribute("title_color").as_string()));
    if (n.attribute("panel_color"))
        dlg->SetPanelColor(ParseColor(n.attribute("panel_color").as_string()));
    if (n.attribute("button_color"))
        dlg->SetButtonColor(ParseColor(n.attribute("button_color").as_string()));

    // Non-<button> children become dialog content widgets.
    for (auto& childNode : n.children()) {
        std::string childName = childNode.name();
        if (childName == "button") continue; // buttons handled via SetButtons
        auto child = ParseWidget(childNode, textures, fonts);
        if (child) dlg->AddContent(std::move(child));
    }
}

} // namespace nui
