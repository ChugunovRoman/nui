// NUI: Canvas implementation
// CPU-only 2D rendering using SDL_Surface pixel manipulation.

#include "renderer/canvas.h"
#include "renderer/texture.h"
#include "renderer/font.h"

#include <SDL.h>
#include <algorithm>
#include <cstring>

namespace nui {

Canvas::Canvas() {}

Canvas::~Canvas() {}

bool Canvas::Initialize(SDL_Surface* screen) {
    m_surface = screen;
    return m_surface != nullptr;
}

void Canvas::Clear(const Color& color) {
    if (!m_surface) return;
    Uint32 c = SDL_MapRGBA(m_surface->format, color.r, color.g, color.b, color.a);
    SDL_FillRect(m_surface, nullptr, c);
}

// ── Scissor (clipping) rect stack ───────────────────────────────

static Rect IntersectRects(const Rect& a, const Rect& b) {
    int x1 = std::max(a.x, b.x);
    int y1 = std::max(a.y, b.y);
    int x2 = std::min(a.x + a.w, b.x + b.w);
    int y2 = std::min(a.y + a.h, b.y + b.h);
    if (x2 <= x1 || y2 <= y1) return Rect(0, 0, 0, 0);
    return Rect(x1, y1, x2 - x1, y2 - y1);
}

void Canvas::PushClip(const Rect& rect) {
    if (!m_surface) return;
    Rect clip = rect;
    if (!m_clipStack.empty()) {
        clip = IntersectRects(rect, m_clipStack.back());
    }
    m_clipStack.push_back(clip);
    SDL_Rect sr = {clip.x, clip.y, clip.w, clip.h};
    SDL_SetClipRect(m_surface, &sr);
}

void Canvas::PopClip() {
    if (!m_surface) return;
    if (!m_clipStack.empty()) {
        m_clipStack.pop_back();
    }
    if (m_clipStack.empty()) {
        SDL_SetClipRect(m_surface, nullptr);
    } else {
        Rect& clip = m_clipStack.back();
        SDL_Rect sr = {clip.x, clip.y, clip.w, clip.h};
        SDL_SetClipRect(m_surface, &sr);
    }
}

Rect Canvas::GetClipRect() const {
    if (m_clipStack.empty()) {
        return Rect(0, 0, m_surface ? m_surface->w : 0, m_surface ? m_surface->h : 0);
    }
    return m_clipStack.back();
}

bool Canvas::ClipRect(Rect& r) const {
    if (m_clipStack.empty()) return true;
    Rect clipped = IntersectRects(r, m_clipStack.back());
    if (clipped.w <= 0 || clipped.h <= 0) return false;
    r = clipped;
    return true;
}

bool Canvas::ClipRect(SDL_Rect& r) const {
    if (m_clipStack.empty()) return true;
    Rect sr = {r.x, r.y, r.w, r.h};
    if (!ClipRect(sr)) return false;
    r.x = sr.x; r.y = sr.y; r.w = sr.w; r.h = sr.h;
    return true;
}

void Canvas::FillRect(const Rect& rect, const Color& color) {
    if (!m_surface) return;
    SDL_Rect r = {rect.x, rect.y, rect.w, rect.h};
    Uint32 c = SDL_MapRGBA(m_surface->format, color.r, color.g, color.b, color.a);
    SDL_FillRect(m_surface, &r, c);
}

void Canvas::DrawRect(const Rect& rect, const Color& color) {
    // Top
    FillRect(Rect(rect.x, rect.y, rect.w, 1), color);
    // Bottom
    FillRect(Rect(rect.x, rect.y + rect.h - 1, rect.w, 1), color);
    // Left
    FillRect(Rect(rect.x, rect.y, 1, rect.h), color);
    // Right
    FillRect(Rect(rect.x + rect.w - 1, rect.y, 1, rect.h), color);
}

void Canvas::DrawLine(int x1, int y1, int x2, int y2, const Color& color) {
    if (!m_surface) return;

    // Bresenham's line algorithm
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    Uint32 c = SDL_MapRGBA(m_surface->format, color.r, color.g, color.b, color.a);
    int bpp = m_surface->format->BytesPerPixel;
    Rect clip = GetClipRect();

    while (true) {
        if (x1 >= clip.x && x1 < clip.x + clip.w &&
            y1 >= clip.y && y1 < clip.y + clip.h) {
            uint8_t* p = static_cast<uint8_t*>(m_surface->pixels) +
                         y1 * m_surface->pitch + x1 * bpp;
            switch (bpp) {
                case 2: *reinterpret_cast<uint16_t*>(p) = static_cast<uint16_t>(c); break;
                case 4: *reinterpret_cast<uint32_t*>(p) = c; break;
            }
        }
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 <  dx) { err += dx; y1 += sy; }
    }
}

void Canvas::DrawPixel(int x, int y, const Color& color) {
    if (!m_surface) return;
    Rect clip = GetClipRect();
    if (x < clip.x || x >= clip.x + clip.w || y < clip.y || y >= clip.y + clip.h) return;

    Uint32 c = SDL_MapRGBA(m_surface->format, color.r, color.g, color.b, color.a);
    int bpp = m_surface->format->BytesPerPixel;
    uint8_t* p = static_cast<uint8_t*>(m_surface->pixels) +
                 y * m_surface->pitch + x * bpp;
    switch (bpp) {
        case 2: *reinterpret_cast<uint16_t*>(p) = static_cast<uint16_t>(c); break;
        case 4: *reinterpret_cast<uint32_t*>(p) = c; break;
    }
}

void Canvas::DrawTexture(Texture& texture, const Rect& dest) {
    if (!m_surface || !texture.GetSurface()) return;

    SDL_Surface* src = texture.GetSurface();
    SDL_Rect srcRect  = {0, 0, src->w, src->h};
    SDL_Rect destRect = {dest.x, dest.y, dest.w, dest.h};

    // Convert to screen format if needed
    if (src->format->format != m_surface->format->format) {
        SDL_Surface* converted = SDL_ConvertSurface(src, m_surface->format, 0);
        if (converted) {
            SDL_BlitScaled(converted, &srcRect, m_surface, &destRect);
            SDL_FreeSurface(converted);
            return;
        }
    }
    SDL_BlitScaled(src, &srcRect, m_surface, &destRect);
}

void Canvas::DrawTexture(Texture& texture, const Rect& src, const Rect& dest) {
    if (!m_surface || !texture.GetSurface()) return;

    SDL_Surface* srcSurface = texture.GetSurface();
    SDL_Rect srcRect  = {src.x, src.y, src.w, src.h};
    SDL_Rect destRect = {dest.x, dest.y, dest.w, dest.h};

    if (srcSurface->format->format != m_surface->format->format) {
        SDL_Surface* converted = SDL_ConvertSurface(srcSurface, m_surface->format, 0);
        if (converted) {
            SDL_BlitScaled(converted, &srcRect, m_surface, &destRect);
            SDL_FreeSurface(converted);
            return;
        }
    }
    SDL_BlitScaled(srcSurface, &srcRect, m_surface, &destRect);
}

void Canvas::DrawText(Font& font, const std::string& text, int x, int y, const Color& color) {
    if (!m_surface || text.empty()) return;

    SDL_Surface* textSurface = font.RenderText(text, color);
    if (!textSurface) return;

    SDL_Rect destRect = {x, y, 0, 0};
    SDL_BlitSurface(textSurface, nullptr, m_surface, &destRect);
    font.FreeRenderedSurface(textSurface);
}

void Canvas::DrawTextWrapped(Font& font, const std::string& text,
                              const Rect& bounds, const Color& color) {
    if (!m_surface || text.empty()) return;

    // Simple word-wrap: split by spaces, accumulate lines
    int lineHeight = font.GetHeight();
    int curX = bounds.x;
    int curY = bounds.y;
    std::string currentLine;

    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\n') {
            DrawText(font, currentLine, curX, curY, color);
            currentLine.clear();
            curX = bounds.x;
            curY += lineHeight;
            if (curY + lineHeight > bounds.y + bounds.h) break;
            continue;
        }

        currentLine += text[i];
        int tw = font.GetTextWidth(currentLine);
        if (tw > bounds.w) {
            // Find last space
            size_t lastSpace = currentLine.rfind(' ');
            if (lastSpace != std::string::npos) {
                std::string toDraw = currentLine.substr(0, lastSpace);
                DrawText(font, toDraw, curX, curY, color);
                currentLine = currentLine.substr(lastSpace + 1);
                curX = bounds.x;
                curY += lineHeight;
                if (curY + lineHeight > bounds.y + bounds.h) break;
            } else {
                DrawText(font, currentLine, curX, curY, color);
                currentLine.clear();
                curX = bounds.x;
                curY += lineHeight;
                if (curY + lineHeight > bounds.y + bounds.h) break;
            }
        }
    }

    if (!currentLine.empty() && curY + lineHeight <= bounds.y + bounds.h) {
        DrawText(font, currentLine, curX, curY, color);
    }
}

} // namespace nui
