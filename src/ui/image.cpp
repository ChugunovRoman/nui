// NUI: Image widget implementation
// Renders textures with various scaling modes.

#include "ui/image.h"
#include "renderer/texture.h"

#include <algorithm>

namespace nui {

Image::Image() {
    m_type = "image";
}

bool Image::LoadFromFile(const std::string& path, TextureCache& cache) {
    m_texture = cache.Get(path);
    return m_texture != nullptr;
}

void Image::Render(Canvas& canvas, FontManager& fonts) {
    if (!m_visible) return;

    DrawBackground(canvas);

    if (!m_texture || !m_texture->IsLoaded()) {
        // Draw placeholder (gray rect with X)
        Rect abs = GetAbsoluteRect();
        canvas.FillRect(abs, Color(60, 60, 60, 255));
        canvas.DrawRect(abs, Color(100, 100, 100, 255));
        canvas.DrawLine(abs.x, abs.y, abs.x + abs.w, abs.y + abs.h, Color(100, 100, 100, 255));
        canvas.DrawLine(abs.x + abs.w, abs.y, abs.x, abs.y + abs.h, Color(100, 100, 100, 255));
        RenderChildren(canvas, fonts);
        return;
    }

    Rect abs = GetAbsoluteRect();
    int texW = m_texture->GetWidth();
    int texH = m_texture->GetHeight();

    if (texW <= 0 || texH <= 0) {
        RenderChildren(canvas, fonts);
        return;
    }

    switch (m_scaleMode) {
        case ScaleMode::Stretch: {
            canvas.DrawTexture(*m_texture, abs);
            break;
        }
        case ScaleMode::Fit: {
            // Scale to fit inside widget, preserve aspect ratio
            float scaleX = static_cast<float>(abs.w) / texW;
            float scaleY = static_cast<float>(abs.h) / texH;
            float scale = std::min(scaleX, scaleY);
            int dw = static_cast<int>(texW * scale);
            int dh = static_cast<int>(texH * scale);
            int dx = abs.x + (abs.w - dw) / 2;
            int dy = abs.y + (abs.h - dh) / 2;
            canvas.DrawTexture(*m_texture, Rect(dx, dy, dw, dh));
            break;
        }
        case ScaleMode::Fill: {
            // Scale to fill, cropping overflow
            float scaleX = static_cast<float>(abs.w) / texW;
            float scaleY = static_cast<float>(abs.h) / texH;
            float scale = std::max(scaleX, scaleY);
            int dw = static_cast<int>(texW * scale);
            int dh = static_cast<int>(texH * scale);
            int dx = abs.x + (abs.w - dw) / 2;
            int dy = abs.y + (abs.h - dh) / 2;
            canvas.DrawTexture(*m_texture, Rect(dx, dy, dw, dh));
            break;
        }
        case ScaleMode::Center: {
            int dx = abs.x + (abs.w - texW) / 2;
            int dy = abs.y + (abs.h - texH) / 2;
            canvas.DrawTexture(*m_texture, Rect(dx, dy, texW, texH));
            break;
        }
        case ScaleMode::Tile: {
            for (int y = abs.y; y < abs.y + abs.h; y += texH) {
                for (int x = abs.x; x < abs.x + abs.w; x += texW) {
                    int dw = std::min(texW, abs.x + abs.w - x);
                    int dh = std::min(texH, abs.y + abs.h - y);
                    canvas.DrawTexture(*m_texture,
                        Rect(0, 0, dw, dh),
                        Rect(x, y, dw, dh));
                }
            }
            break;
        }
    }

    RenderChildren(canvas, fonts);
}

} // namespace nui
