// NUI: RadioButton implementation

#include "ui/radiobutton.h"
#include "core/input.h"
#include "renderer/font.h"

#include <map>
#include <mutex>
#include <vector>
#include <cmath>

namespace nui {

// ── Group registry ──────────────────────────────────────────────
// Radio buttons are grouped by name regardless of where they live in the
// widget tree (they may sit in different parent containers and still belong to
// the same mutually-exclusive group). Each group owns its member list and the
// currently-selected button. The registry is thread-safe since radio buttons
// can be created/destroyed from worker threads in async scenarios.
class RadioGroupRegistry {
public:
    static RadioGroupRegistry& Instance() {
        static RadioGroupRegistry s;
        return s;
    }

    // (Re)assign a button to a group. Handles move between groups and first
    // registration.
    void SetGroup(RadioButton* rb, const std::string& newGroup) {
        std::string oldGroup = rb->m_group;
        if (oldGroup == newGroup) return;
        rb->m_group = newGroup;

        std::lock_guard<std::mutex> lock(m_mutex);
        if (!oldGroup.empty()) RemoveFromGroup(rb, oldGroup);
        if (!newGroup.empty()) m_groups[newGroup].members.push_back(rb);
    }

    // Remove a button from its group (called from the dtor).
    void Unregister(RadioButton* rb) {
        if (rb->m_group.empty()) return;
        std::string group = rb->m_group;
        std::lock_guard<std::mutex> lock(m_mutex);
        RemoveFromGroup(rb, group);
    }

    // Make `rb` the sole selected button of its group, returning the list of
    // previously-selected buttons that were deselected (callers fire their
    // callbacks after committing state).
    std::vector<RadioButton*> SelectExclusive(RadioButton* rb) {
        std::vector<RadioButton*> deselected;
        if (rb->m_group.empty()) return deselected;
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_groups.find(rb->m_group);
        if (it == m_groups.end()) return deselected;
        for (auto* other : it->second.members) {
            if (other == rb) continue;
            if (other->m_selected) {
                other->m_selected = false;
                deselected.push_back(other);
            }
        }
        it->second.selected = rb;
        return deselected;
    }

    // Clear the group's selected pointer (used when a button is deselected).
    void ClearSelected(RadioButton* rb) {
        if (rb->m_group.empty()) return;
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_groups.find(rb->m_group);
        if (it != m_groups.end() && it->second.selected == rb) {
            it->second.selected = nullptr;
        }
    }

private:
    struct Group {
        std::vector<RadioButton*> members;
        RadioButton* selected = nullptr;
    };

    // Caller must hold m_mutex.
    void RemoveFromGroup(RadioButton* rb, const std::string& group) {
        auto it = m_groups.find(group);
        if (it == m_groups.end()) return;
        auto& members = it->second.members;
        for (size_t i = 0; i < members.size(); ++i) {
            if (members[i] == rb) {
                members.erase(members.begin() + i);
                break;
            }
        }
        if (it->second.selected == rb) it->second.selected = nullptr;
        if (members.empty()) m_groups.erase(it);
    }

    std::mutex m_mutex;
    std::map<std::string, Group> m_groups;
};

RadioButton::RadioButton() {
    m_type = "radiobutton";
    m_bgColor = Color::Transparent();
}

RadioButton::~RadioButton() {
    RadioGroupRegistry::Instance().Unregister(this);
}

void RadioButton::SetGroup(const std::string& group) {
    RadioGroupRegistry::Instance().SetGroup(this, group);
}

void RadioButton::SetSelected(bool selected) {
    if (selected) {
        if (m_selected) return; // already selected
        // Deselect siblings in the same group first.
        auto deselected = RadioGroupRegistry::Instance().SelectExclusive(this);
        m_selected = true;
        // Fire callbacks AFTER committing state so observers see consistency.
        for (auto* d : deselected) {
            if (d->m_onSelectedChanged) d->m_onSelectedChanged(d);
        }
        if (m_onSelectedChanged) m_onSelectedChanged(this);
    } else {
        if (!m_selected) return; // already deselected
        m_selected = false;
        RadioGroupRegistry::Instance().ClearSelected(this);
        if (m_onSelectedChanged) m_onSelectedChanged(this);
    }
}

bool RadioButton::HandleInput(InputState& input) {
    if (!m_visible || !m_enabled) return false;

    Rect abs = GetAbsoluteRect();
    int mx = input.GetMouseX();
    int my = input.GetMouseY();

    if (abs.Contains(mx, my) && input.IsMouseClicked(MouseButton::Left)) {
        // Clicking an already-selected radio button still consumes the click so
        // it does not fall through to widgets below.
        if (!m_selected) SetSelected(true);
        return true;
    }

    return false;
}

void RadioButton::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;

    Rect abs = GetAbsoluteRect();

    // Background
    if (m_bgColor.a > 0) {
        canvas.FillRect(abs, m_bgColor);
    }

    // Outer ring (outline): drawn as a 2px-thick circle via horizontal
    // scanlines. Cheaper than per-pixel DrawPixel in the hot loop (bug 15).
    int dotCenterX = abs.x + m_dotRadius;
    int dotCenterY = abs.y + abs.h / 2;
    int r = m_dotRadius;
    Color ringColor(90, 90, 110, 255);

    for (int dy = -r; dy <= r; ++dy) {
        int outerRad2 = r * r - dy * dy;
        if (outerRad2 <= 0) continue;
        int outer = static_cast<int>(std::sqrt(static_cast<float>(outerRad2)) + 0.5f);
        // Inner radius (r-2) defines the ring thickness; clamp the radicand to
        // 0 so we never pass a negative value to sqrt (which would yield NaN).
        int innerRad2 = (r > 2) ? (r - 2) * (r - 2) - dy * dy : 0;
        int inner = (innerRad2 > 0)
            ? static_cast<int>(std::sqrt(static_cast<float>(innerRad2)) + 0.5f)
            : 0;
        int y = dotCenterY + dy;
        int bandW = outer - inner;
        if (bandW > 0) {
            // Left arc band of the ring.
            canvas.FillRect(Rect(dotCenterX - outer, y, bandW, 1), ringColor);
            // Right arc band of the ring.
            canvas.FillRect(Rect(dotCenterX + inner, y, bandW, 1), ringColor);
        }
    }

    // Inner filled dot when selected.
    if (m_selected) {
        int innerR = r - 3;
        if (innerR > 0) {
            for (int dy = -innerR; dy <= innerR; ++dy) {
                int rad2 = innerR * innerR - dy * dy;
                if (rad2 <= 0) continue;
                int span = static_cast<int>(std::sqrt(static_cast<float>(rad2)) + 0.5f);
                if (span <= 0) continue;
                canvas.FillRect(Rect(dotCenterX - span, dotCenterY + dy,
                                     span * 2, 1), m_dotColor);
            }
        } else {
            canvas.FillRect(Rect(dotCenterX, dotCenterY, 1, 1), m_dotColor);
        }
    }

    // Text (right of dot)
    if (!m_text.empty()) {
        Font* font = m_font ? m_font : fonts.GetDefault(m_fontSize);
        if (font) {
            int textX = abs.x + m_dotRadius * 2 + 8;
            int textY = abs.y + (abs.h - font->GetHeight()) / 2;
            canvas.DrawText(*font, m_text, textX, textY, m_textColor);
        }
    }

    // Border — the base Render() override was silently dropping this (bug 10).
    if (m_borderColor.a > 0) {
        canvas.DrawRect(abs, m_borderColor);
    }

    RenderChildren(canvas, fonts);
}

} // namespace nui
