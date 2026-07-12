#pragma once
// NUI: Canvas - CPU software rendering surface
// Wraps SDL_Surface for 2D drawing primitives.
// All rendering is CPU-based, no GPU required.

#include <cstdint>
#include <string>
#include <vector>

#include <SDL.h>
#include "renderer/color.h"

namespace nui {

class Texture;
class Font;

// ── Rect helper ─────────────────────────────────────────────────
struct Rect {
    int x, y, w, h;
    Rect() : x(0), y(0), w(0), h(0) {}
    Rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}

    bool Contains(int px, int py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
};

// ── Canvas ──────────────────────────────────────────────────────
class Canvas {
public:
    Canvas();
    ~Canvas();

    bool Initialize(SDL_Surface* screen);
    void SetSurface(SDL_Surface* surface) { m_surface = surface; }
    SDL_Surface* GetSurface() const { return m_surface; }

    // Clear entire surface
    void Clear(const Color& color);

    // Basic primitives
    void FillRect(const Rect& rect, const Color& color);
    void DrawRect(const Rect& rect, const Color& color);
    void DrawLine(int x1, int y1, int x2, int y2, const Color& color);
    void DrawPixel(int x, int y, const Color& color);

    // Textured rectangle (stretch texture to fit rect)
    void DrawTexture(Texture& texture, const Rect& dest);
    void DrawTexture(Texture& texture, const Rect& src, const Rect& dest);

    // Text rendering
    void DrawText(Font& font, const std::string& text, int x, int y, const Color& color);

    // Text with word-wrap inside a bounding rectangle
    void DrawTextWrapped(Font& font, const std::string& text,
                         const Rect& bounds, const Color& color);

    // Rotated surface blit (CPU rotation). pivotX/Y is the rotation origin in
    // destination coordinates. When bilinear is true, bilinear sampling is
    // used for smoother edges at the cost of ~4x pixel reads; otherwise the
    // fast nearest-neighbour path is used.
    void DrawSurfaceRotated(SDL_Surface* src, int dstX, int dstY,
                             float angleDeg, float pivotX, float pivotY,
                             bool bilinear = false);

    // Scissor (clipping) rect stack
    void PushClip(const Rect& rect);
    void PopClip();
    Rect GetClipRect() const;
    bool IsClipped() const { return !m_clipStack.empty(); }
    // Number of entries currently on the clip stack. Useful for saving/restoring
    // the exact clip state (e.g. when an overlay must temporarily escape its
    // ancestor's clip rect).
    size_t GetClipStackSize() const { return m_clipStack.size(); }

private:
    bool ClipRect(Rect& r) const;
    bool ClipRect(SDL_Rect& r) const;

    SDL_Surface* m_surface = nullptr;
    std::vector<Rect> m_clipStack;
};

} // namespace nui
