#pragma once
// NUI: RadioButton widget
// Radio button with group management (only one selected per group).

#include "ui/widget.h"
#include <string>
#include <functional>

namespace nui {

class Font;

class RadioButton : public Widget {
public:
    friend class RadioGroupRegistry;
    RadioButton();
    ~RadioButton();

    // State
    void SetSelected(bool selected);
    bool IsSelected() const { return m_selected; }

    // Group: all RadioButtons sharing a group name are mutually exclusive.
    // Group membership is tracked in a global registry, so radio buttons in
    // different parent containers can still belong to the same group.
    void SetGroup(const std::string& group);
    const std::string& GetGroup() const { return m_group; }

    // Appearance
    void SetText(const std::string& text) { m_text = text; }
    const std::string& GetText() const { return m_text; }
    void SetFontSize(int size) { m_fontSize = size; }
    void SetFont(Font* font) { m_font = font; }
    void SetTextColor(const Color& c) { m_textColor = c; }
    void SetDotColor(const Color& c) { m_dotColor = c; }
    void SetDotRadius(int r) { m_dotRadius = r; }

    // Callback
    using SelectedCallback = std::function<void(Widget*)>;
    void SetOnSelectedChanged(SelectedCallback cb) { m_onSelectedChanged = std::move(cb); }

    // Widget overrides
    bool HandleInput(InputState& input) override;
    void Render(Canvas& canvas, FontManager& fonts) override;

private:
    bool m_selected = false;
    std::string m_text;
    std::string m_group;
    int m_fontSize = 14;
    Font* m_font = nullptr;
    int m_dotRadius = 8;

    Color m_textColor = Color::White();
    Color m_dotColor  = Color(80, 180, 255, 255);

    SelectedCallback m_onSelectedChanged;
};

} // namespace nui
