#pragma once
// NUI: Dialog / MessageBox widget
// A modal overlay window: title bar, arbitrary content widgets and a row of
// action buttons. Pushed onto the application overlay stack as modal, so the
// background is dimmed and underlying widgets receive no input until it closes.
// DialogResult is reported through the OnResult callback.

#include "ui/widget.h"
#include "renderer/font.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace nui {

// Standard button sets for message boxes.
enum class DialogButtons : int {
    Ok            = 1, // [OK]
    OkCancel      = 2, // [OK] [Cancel]
    YesNo         = 3, // [Yes] [No]
    YesNoCancel   = 4, // [Yes] [No] [Cancel]
    RetryCancel   = 5, // [Retry] [Cancel]
};

// Which button the user pressed to close the dialog. None = dismissed without
// a button (e.g. the host called Close()).
enum class DialogResult {
    None,
    Ok,
    Cancel,
    Yes,
    No,
    Retry,
};

class Dialog : public Widget {
public:
    Dialog();

    using ResultCallback = std::function<void(DialogResult)>;

    // Set the dialog title (shown in the title bar).
    void SetTitle(const std::string& title) { m_title = title; }
    const std::string& GetTitle() const { return m_title; }

    // Set the body message text. If you also add content widgets, both are
    // shown (text first, then widgets).
    void SetMessage(const std::string& msg) { m_message = msg; }
    const std::string& GetMessage() const { return m_message; }

    // Configure the standard button set. Adds the matching buttons.
    void SetButtons(DialogButtons buttons);
    DialogButtons GetButtons() const { return m_buttons; }

    // Add a custom content widget (ownership transferred). Rendered below the
    // message text.
    void AddContent(std::unique_ptr<Widget> w);

    // Callback fired once when the dialog closes with a button result.
    void SetOnResult(ResultCallback cb) { m_onResult = std::move(cb); }

    // Open as a modal overlay, centered in the screen. Takes ownership.
    static void Open(std::unique_ptr<Dialog> dlg, int screenW, int screenH);

    // Convenience: a simple message box. Fires `cb` with the pressed button.
    // NOTE: named ShowMessage (not "MessageBox") to avoid clashing with the
    // Win32 `MessageBox` macro from <windows.h>, which would mangle the name.
    static void ShowMessage(const std::string& title, const std::string& message,
                            DialogButtons buttons, ResultCallback cb,
                            int screenW, int screenH);

    // Close with a result; fires OnResult and removes from overlay stack.
    void Close(DialogResult result);

    // Appearance
    void SetFontSize(int size) { m_fontSize = size; }
    void SetFont(Font* font) { m_font = font; }
    void SetTitleColor(const Color& c) { m_titleColor = c; }
    void SetPanelColor(const Color& c) { m_panelColor = c; }
    void SetButtonColor(const Color& c) { m_buttonColor = c; }
    void SetButtonHoverColor(const Color& c) { m_buttonHoverColor = c; }

    // Widget overrides
    bool HandleInput(InputState& input) override;
    void Render(Canvas& canvas, FontManager& fonts) override;

private:
    struct ButtonInfo {
        std::string label;
        DialogResult result;
        Rect rect;       // absolute, recomputed each render
    };

    void LayoutButtons(const Rect& abs);
    Rect GetButtonRect(const Rect& abs, int index) const;

    std::string m_title;
    std::string m_message;
    DialogButtons m_buttons = DialogButtons::Ok;
    std::vector<ButtonInfo> m_buttonInfos;
    int m_hoveredButton = -1;

    int  m_fontSize = 14;
    Font* m_font = nullptr;
    Color m_titleColor       = Color(70, 100, 160, 255);
    Color m_panelColor       = Color(35, 35, 50, 255);
    Color m_buttonColor      = Color(50, 50, 70, 255);
    Color m_buttonHoverColor = Color(70, 70, 95, 255);
    Color m_textColor        = Color::White();

    ResultCallback m_onResult;
    bool m_closed = false;
};

} // namespace nui
