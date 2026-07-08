// NUI: Canvas implementation
// CPU-only 2D rendering using SDL_Surface pixel manipulation.

#include "renderer/canvas.h"
#include "renderer/texture.h"
#include "renderer/font.h"
#include "core/log.h"
#include "core/math.h"

#include <SDL.h>
#include <algorithm>
#include <cmath>
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
    if (color.a == 0) return;

    if (color.a == 255) {
        // Opaque — fast path
        SDL_Rect r = {rect.x, rect.y, rect.w, rect.h};
        Uint32 c = SDL_MapRGBA(m_surface->format, color.r, color.g, color.b, 255);
        SDL_FillRect(m_surface, &r, c);
        return;
    }

    // Semi-transparent — blit a colored surface with alpha blending
    SDL_Rect sr = {0, 0, rect.w, rect.h};
    SDL_Rect dr = {rect.x, rect.y, rect.w, rect.h};

    // Clip to surface bounds
    if (dr.x < 0) { sr.x -= dr.x; sr.w += dr.x; dr.x = 0; }
    if (dr.y < 0) { sr.y -= dr.y; sr.h += dr.y; dr.y = 0; }
    if (dr.x + dr.w > m_surface->w) { int diff = dr.x + dr.w - m_surface->w; sr.w -= diff; dr.w -= diff; }
    if (dr.y + dr.h > m_surface->h) { int diff = dr.y + dr.h - m_surface->h; sr.h -= diff; dr.h -= diff; }
    if (sr.w <= 0 || sr.h <= 0) return;

    // Create a temporary surface with the fill color
    SDL_Surface* tmp = SDL_CreateRGBSurface(0, sr.w, sr.h, 32,
        m_surface->format->Rmask, m_surface->format->Gmask,
        m_surface->format->Bmask, m_surface->format->Amask);
    if (!tmp) return;

    // Fill with the color (opaque)
    Uint32 c = SDL_MapRGBA(tmp->format, color.r, color.g, color.b, 255);
    SDL_FillRect(tmp, nullptr, c);

    // Set alpha on the entire surface
    SDL_SetSurfaceAlphaMod(tmp, color.a);
    SDL_SetSurfaceBlendMode(tmp, SDL_BLENDMODE_BLEND);

    // Blit with alpha blending
    SDL_BlitSurface(tmp, &sr, m_surface, &dr);
    SDL_FreeSurface(tmp);
}

void Canvas::DrawSurfaceRotated(SDL_Surface* src, int dstX, int dstY,
                                  float angleDeg, float pivotX, float pivotY,
                                  bool bilinear) {
    if (!m_surface || !src) return;

    if (angleDeg == 0.0f) {
        SDL_Rect dr = {dstX, dstY, 0, 0};
        SDL_BlitSurface(src, nullptr, m_surface, &dr);
        return;
    }

    // Only 32bpp source/destination are supported for the hand-rolled rotator.
    // The widget render cache is always created as 32bpp RGBA, and the screen
    // surface is 32bpp on any modern desktop. Bail out (with a diagnostic) for
    // unusual formats rather than silently rendering nothing.
    const int srcBpp = src->format->BytesPerPixel;
    const int dstBpp = m_surface->format->BytesPerPixel;
    if (srcBpp != 4 || dstBpp != 4) {
        NUI_LOG("[Canvas] DrawSurfaceRotated: unsupported format (src=%db, dst=%db), skipped\n",
                srcBpp * 8, dstBpp * 8);
        return;
    }

    // No conversion: read the source (RGBA8888 cache) by its own masks and
    // write the destination by the screen masks. This avoids the per-frame
    // SDL_ConvertSurface allocation that the old code did when the two
    // SDL_PixelFormat values differed (RGBA8888 vs XRGB8888).
    SDL_Surface* work = src;

    const float rad = nui::DegToRad(angleDeg);
    const float cosA = cosf(rad);
    const float sinA = sinf(rad);

    const int srcW = work->w;
    const int srcH = work->h;

    // Source masks (precompute shifts once instead of per pixel).
    const int srcRsh = work->format->Rshift;
    const int srcGsh = work->format->Gshift;
    const int srcBsh = work->format->Bshift;
    const int srcAsh = work->format->Ashift;
    const int dstRsh = m_surface->format->Rshift;
    const int dstGsh = m_surface->format->Gshift;
    const int dstBsh = m_surface->format->Bshift;
    const int dstAsh = m_surface->format->Ashift;

    // Pivot in source-local coords
    const float srcPivotX = pivotX - dstX;
    const float srcPivotY = pivotY - dstY;

    // Bounding box: rotate corners around pivot
    float cornersX[4] = {0.0f, float(srcW), float(srcW), 0.0f};
    float cornersY[4] = {0.0f, 0.0f, float(srcH), float(srcH)};
    float minX = pivotX, maxX = pivotX, minY = pivotY, maxY = pivotY;
    for (int i = 0; i < 4; ++i) {
        float rx = cornersX[i] - srcPivotX;
        float ry = cornersY[i] - srcPivotY;
        float px = rx * cosA - ry * sinA + pivotX;
        float py = rx * sinA + ry * cosA + pivotY;
        if (px < minX) minX = px;
        if (px > maxX) maxX = px;
        if (py < minY) minY = py;
        if (py > maxY) maxY = py;
    }

    // Intersect the rotated bounding box with the active clip stack so that a
    // rotated widget nested inside a ScrollView (which PushClip's its bounds)
    // does not bleed outside the scroll viewport. Use floor/ceil (not truncating
    // casts) so negative coordinates are rounded outwards, keeping every pixel
    // that the rotated quad can cover.
    int x1 = static_cast<int>(floorf(minX)) - 1;
    int y1 = static_cast<int>(floorf(minY)) - 1;
    int x2 = static_cast<int>(ceilf(maxX)) + 1;
    int y2 = static_cast<int>(ceilf(maxY)) + 1;

    Rect bbox(x1, y1, x2 - x1, y2 - y1);
    if (!ClipRect(bbox)) return;
    x1 = bbox.x; y1 = bbox.y;
    x2 = bbox.x + bbox.w; y2 = bbox.y + bbox.h;

    for (int dy = y1; dy < y2; ++dy) {
        uint8_t* dstRow = static_cast<uint8_t*>(m_surface->pixels) + dy * m_surface->pitch;
        for (int dx = x1; dx < x2; ++dx) {
            float rx = static_cast<float>(dx) - pivotX;
            float ry = static_cast<float>(dy) - pivotY;
            // Inverse rotation R(-theta): maps destination pixel -> source pixel.
            // Must be the transpose (inverse) of the forward matrix used for the bbox above.
            float sx = rx * cosA + ry * sinA + srcPivotX;
            float sy = -rx * sinA + ry * cosA + srcPivotY;

            // Bounds: with padding required for bilinear taps.
            const int pad = bilinear ? 1 : 0;
            if (sx < 0 || sx > srcW - 1 - pad ||
                sy < 0 || sy > srcH - 1 - pad) continue;

            uint8_t sr, sg, sb, sa;

            if (bilinear) {
                // Bilinear sample 4 neighbours for smooth edges.
                int x0 = static_cast<int>(sx);
                int y0 = static_cast<int>(sy);
                int x1s = x0 + 1;
                int y1s = y0 + 1;
                float fx = sx - x0;
                float fy = sy - y0;

                auto sample = [&](int xx, int yy, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
                    uint32_t p;
                    std::memcpy(&p,
                        static_cast<uint8_t*>(work->pixels) + yy * work->pitch + xx * 4, 4);
                    r = (p >> srcRsh) & 0xFF;
                    g = (p >> srcGsh) & 0xFF;
                    b = (p >> srcBsh) & 0xFF;
                    a = (p >> srcAsh) & 0xFF;
                };
                uint8_t r0, g0, b0, a0, r1, g1, b1, a1, r2, g2, b2, a2, r3, g3, b3, a3;
                sample(x0, y0, r0, g0, b0, a0);
                sample(x1s, y0, r1, g1, b1, a1);
                sample(x0, y1s, r2, g2, b2, a2);
                sample(x1s, y1s, r3, g3, b3, a3);

                auto lerp = [](uint8_t a, uint8_t b, float f) {
                    return static_cast<uint8_t>(a + (b - a) * f + 0.5f);
                };
                uint8_t tr = lerp(r0, r1, fx);
                uint8_t tg = lerp(g0, g1, fx);
                uint8_t tb = lerp(b0, b1, fx);
                uint8_t ta = lerp(a0, a1, fx);
                uint8_t br = lerp(r2, r3, fx);
                uint8_t bg = lerp(g2, g3, fx);
                uint8_t bb = lerp(b2, b3, fx);
                uint8_t ba = lerp(a2, a3, fx);
                sr = lerp(tr, br, fy);
                sg = lerp(tg, bg, fy);
                sb = lerp(tb, bb, fy);
                sa = lerp(ta, ba, fy);
            } else {
                // Nearest-neighbor: fast path.
                int isx = static_cast<int>(sx);
                int isy = static_cast<int>(sy);
                uint32_t srcPx;
                std::memcpy(&srcPx,
                    static_cast<uint8_t*>(work->pixels) + isy * work->pitch + isx * 4, 4);
                sr = (srcPx >> srcRsh) & 0xFF;
                sg = (srcPx >> srcGsh) & 0xFF;
                sb = (srcPx >> srcBsh) & 0xFF;
                sa = (srcPx >> srcAsh) & 0xFF;
            }

            if (sa == 0) continue;

            uint8_t* dp = dstRow + dx * 4;

            if (sa == 255) {
                uint32_t out =
                    (static_cast<uint32_t>(sr) << dstRsh) |
                    (static_cast<uint32_t>(sg) << dstGsh) |
                    (static_cast<uint32_t>(sb) << dstBsh) |
                    (static_cast<uint32_t>(0xFF) << dstAsh);
                std::memcpy(dp, &out, 4);
            } else {
                // Alpha blending: division-free using (v + 128) >> 8, which
                // approximates round(v/255) closely and removes a per-channel
                // integer divide from the hot loop.
                uint32_t dstPx;
                std::memcpy(&dstPx, dp, 4);
                uint8_t dr = (dstPx >> dstRsh) & 0xFF;
                uint8_t dg = (dstPx >> dstGsh) & 0xFF;
                uint8_t db = (dstPx >> dstBsh) & 0xFF;
                uint32_t inv = 255 - sa;
                auto blend = [](uint8_t sc, uint8_t dc, uint32_t w, uint32_t iw) -> uint32_t {
                    return (static_cast<uint32_t>(sc) * w + static_cast<uint32_t>(dc) * iw + 128) >> 8;
                };
                uint32_t out =
                    (blend(sr, dr, sa, inv) << dstRsh) |
                    (blend(sg, dg, sa, inv) << dstGsh) |
                    (blend(sb, db, sa, inv) << dstBsh) |
                    (static_cast<uint32_t>(0xFF) << dstAsh);
                std::memcpy(dp, &out, 4);
            }
        }
    }
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
