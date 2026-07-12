#pragma once
// NUI: XML Layout Loader
// Parses XML layout files and builds widget trees.
// Inspired by xrUICore's CUIXmlInitBase.

#include <string>
#include <memory>
#include <map>

#include <pugixml.hpp>
#include "renderer/color.h"

namespace nui {

class Widget;
class TextureCache;
class FontManager;

// ── Layout loader ───────────────────────────────────────────────
class LayoutLoader {
public:
    LayoutLoader();
    ~LayoutLoader();

    // Load layout from XML file. Returns root widget.
    std::unique_ptr<Widget> LoadFromFile(const std::string& path,
                                          TextureCache& textures,
                                          FontManager& fonts);

    // Load layout from XML string.
    std::unique_ptr<Widget> LoadFromString(const std::string& xml,
                                            TextureCache& textures,
                                            FontManager& fonts);

    // Color definitions (named colors referenced in XML)
    void LoadColorDefs(const std::string& path);
    Color GetColorByName(const std::string& name) const;

private:
    std::unique_ptr<Widget> ParseWidget(pugi::xml_node node,
                                         TextureCache& textures,
                                         FontManager& fonts);

    // Parse widget-specific properties
    void ParseCommonProps(Widget* widget, pugi::xml_node node);
    void ParseLabel(pugi::xml_node node, Widget* widget, FontManager& fonts);
    void ParseButton(pugi::xml_node node, Widget* widget, FontManager& fonts);
    void ParseImage(pugi::xml_node node, Widget* widget, TextureCache& textures);
    void ParseEditBox(pugi::xml_node node, Widget* widget, FontManager& fonts);
    void ParseProgressBar(pugi::xml_node node, Widget* widget, FontManager& fonts);
    void ParseScrollView(pugi::xml_node node, Widget* widget);
    void ParseSlider(pugi::xml_node node, Widget* widget);
    void ParseCheckBox(pugi::xml_node node, Widget* widget, FontManager& fonts);
    void ParseRadioButton(pugi::xml_node node, Widget* widget, FontManager& fonts);
    void ParseDropdown(pugi::xml_node node, Widget* widget, FontManager& fonts);

    // Utility: parse "r,g,b,a" string to Color
    Color ParseColor(const std::string& str) const;

    std::map<std::string, Color> m_colorDefs;
};

} // namespace nui
