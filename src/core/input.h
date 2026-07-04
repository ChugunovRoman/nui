#pragma once
// NUI: Input state tracking
// Stores current mouse/keyboard state for widget hit-testing.

#include <string>
#include <map>

namespace nui {

// ── Mouse button enumeration ────────────────────────────────────
enum class MouseButton {
    Left   = 0,
    Middle = 1,
    Right  = 2,
    COUNT  = 3
};

// ── Input state ─────────────────────────────────────────────────
class InputState {
public:
    void BeginFrame();

    // Mouse
    int  GetMouseX() const { return m_mouseX; }
    int  GetMouseY() const { return m_mouseY; }
    bool IsMouseDown(MouseButton btn) const;
    bool IsMouseClicked(MouseButton btn) const;  // true on the frame button was pressed
    bool IsMouseReleased(MouseButton btn) const;

    // Mouse wheel (positive = up, negative = down)
    int  GetWheelY() const { return m_wheelY; }

    // Keyboard - uses SDL key codes (any value, not limited to 256)
    bool IsKeyDown(int keycode) const;
    bool IsKeyClicked(int keycode) const;  // true on the frame key was pressed

    // Text input (UTF-8 string entered this frame)
    const std::string& GetTextInput() const { return m_textInput; }

    // Setters (called by Application during event processing)
    void SetMousePos(int x, int y) { m_mouseX = x; m_mouseY = y; }
    void SetMouseButton(MouseButton btn, bool down);
    void SetKeyDown(int keycode);
    void SetKeyUp(int keycode);
    void AppendTextInput(const std::string& text) { m_textInput += text; }
    void SetWheelY(int y) { m_wheelY = y; }

private:
    int  m_mouseX = 0;
    int  m_mouseY = 0;
    int  m_wheelY = 0;
    bool m_mouseDown[3]    = {false, false, false};
    bool m_mouseClicked[3] = {false, false, false};
    bool m_mouseReleased[3]= {false, false, false};

    // Key state - uses map to support full SDL key range
    std::map<int, bool> m_keyDown;
    std::map<int, bool> m_keyClicked;

    std::string m_textInput;
};

} // namespace nui
