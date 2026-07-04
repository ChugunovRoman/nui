// NUI: Input state implementation

#include "core/input.h"

namespace nui {

void InputState::BeginFrame() {
    for (int i = 0; i < 3; ++i) {
        m_mouseClicked[i]  = false;
        m_mouseReleased[i] = false;
    }
    m_textInput.clear();
    m_wheelY = 0;
    m_keyClicked.clear();
}

bool InputState::IsMouseDown(MouseButton btn) const {
    int i = static_cast<int>(btn);
    return (i >= 0 && i < 3) ? m_mouseDown[i] : false;
}

bool InputState::IsMouseClicked(MouseButton btn) const {
    int i = static_cast<int>(btn);
    return (i >= 0 && i < 3) ? m_mouseClicked[i] : false;
}

bool InputState::IsMouseReleased(MouseButton btn) const {
    int i = static_cast<int>(btn);
    return (i >= 0 && i < 3) ? m_mouseReleased[i] : false;
}

bool InputState::IsKeyDown(int keycode) const {
    auto it = m_keyDown.find(keycode);
    return (it != m_keyDown.end()) ? it->second : false;
}

bool InputState::IsKeyClicked(int keycode) const {
    auto it = m_keyClicked.find(keycode);
    return (it != m_keyClicked.end()) ? it->second : false;
}

void InputState::SetMouseButton(MouseButton btn, bool down) {
    int i = static_cast<int>(btn);
    if (i < 0 || i >= 3) return;
    if (down && !m_mouseDown[i]) m_mouseClicked[i] = true;
    if (!down && m_mouseDown[i]) m_mouseReleased[i] = true;
    m_mouseDown[i] = down;
}

void InputState::SetKeyDown(int keycode) {
    if (!m_keyDown[keycode]) m_keyClicked[keycode] = true;
    m_keyDown[keycode] = true;
}

void InputState::SetKeyUp(int keycode) {
    m_keyDown[keycode] = false;
}

} // namespace nui
