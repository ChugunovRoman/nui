#pragma once
// NUI: Popup geometry helpers
// Shared positioning/clamping utilities used by overlay widgets (Tooltip,
// Menu/ContextMenu). Generalises Dropdown::GetPanelRect so that every popup
// does not reinvent the same arithmetic.

#include "renderer/canvas.h" // Rect

namespace nui {

// Direction a popup prefers to open relative to an anchor.
enum class PopupDirection {
    Down,   // open below the anchor (default)
    Up,     // open above the anchor (when not enough room below)
    Right,  // open to the right of the anchor (context menus, submenus)
    Left,   // open to the left of the anchor
};

struct PopupGeometry {
    // Clamp an already-positioned rect so it stays fully inside the screen.
    // If it does not fit, the top-left is clamped to [margin, screen - size].
    // NOTE: when the rect is larger than the screen (minus margins), there is
    // no position that fits it entirely — the left edge is clamped to `margin`
    // and the right edge overflows. Callers dealing with potentially-oversized
    // content (e.g. a very long tooltip) should wrap/measure the text first.
    static Rect ClampToScreen(Rect rect, int screenW, int screenH, int margin) {
        if (rect.x < margin) rect.x = margin;
        if (rect.y < margin) rect.y = margin;
        if (rect.x + rect.w > screenW - margin) rect.x = screenW - margin - rect.w;
        if (rect.y + rect.h > screenH - margin) rect.y = screenH - margin - rect.h;
        if (rect.x < margin) rect.x = margin;
        if (rect.y < margin) rect.y = margin;
        return rect;
    }

    // Position a popup of the given size at a screen-space point (usually the
    // cursor), then clamp it inside the screen. Prefers opening below-right of
    // the point; flips up/left if there is no room.
    static Rect PlaceAtCursor(int w, int h, int cursorX, int cursorY,
                              int screenW, int screenH, int margin) {
        int x = cursorX + margin;
        int y = cursorY + margin;
        // Flip horizontally if it would overflow the right edge.
        if (x + w > screenW - margin) x = cursorX - w - margin;
        // Flip vertically if it would overflow the bottom edge.
        if (y + h > screenH - margin) y = cursorY - h - margin;
        Rect rect(x, y, w, h);
        return ClampToScreen(rect, screenW, screenH, margin);
    }

    // Position a popup relative to an anchor rect (e.g. a menu header or a
    // tree node). Opens below by default; flips up when there is not enough
    // vertical space. Width is taken from the anchor unless popupW > 0.
    static Rect PlaceByAnchor(const Rect& anchor, int popupW, int popupH,
                              int screenW, int screenH, int margin,
                              PopupDirection dir = PopupDirection::Down) {
        int w = popupW > 0 ? popupW : anchor.w;
        int x = anchor.x;
        int y;
        if (dir == PopupDirection::Down) {
            y = anchor.y + anchor.h;
            if (y + popupH > screenH - margin) {
                // Not enough room below — open upward if it fits better.
                int upY = anchor.y - popupH;
                if (upY >= margin) y = upY;
            }
        } else { // Up
            y = anchor.y - popupH;
            if (y < margin) {
                int downY = anchor.y + anchor.h;
                if (downY + popupH <= screenH - margin) y = downY;
            }
        }
        Rect rect(x, y, w, popupH);
        return ClampToScreen(rect, screenW, screenH, margin);
    }
};

} // namespace nui
