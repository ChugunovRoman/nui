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
#include "renderer/texture.h"
#include "renderer/font.h"
#include "renderer/resource.h"

#include <cstdio>
#include <cstring>
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
    } else {
        widget = std::make_unique<Widget>();
    }

    ParseCommonProps(widget.get(), node);

    for (auto& childNode : node.children()) {
        auto child = ParseWidget(childNode, textures, fonts);
        if (child)
            widget->AddChild(std::move(child));
    }
    return widget;
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

} // namespace nui
