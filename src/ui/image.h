#pragma once
// NUI: Image widget
// Displays a texture (PNG, JPG, BMP, TGA, GIF) with scaling modes.

#include "ui/widget.h"
#include <string>

namespace nui {

class Texture;
class TextureCache;

enum class ScaleMode {
    Stretch,    // Scale to fill the widget rect
    Fit,        // Scale to fit inside, preserving aspect ratio
    Fill,       // Scale to fill, cropping overflow
    Center,     // Draw at original size, centered
    Tile        // Tile the texture to fill
};

class Image : public Widget {
public:
    Image();

    // Load from file path
    bool LoadFromFile(const std::string& path, TextureCache& cache);

    // Set texture directly (does not take ownership)
    void SetTexture(Texture* tex) { m_texture = tex; }
    Texture* GetTexture() const { return m_texture; }

    // Scaling
    void SetScaleMode(ScaleMode mode) { m_scaleMode = mode; }
    ScaleMode GetScaleMode() const { return m_scaleMode; }

    // Widget overrides
    void Render(Canvas& canvas, FontManager& fonts) override;

private:
    Texture*  m_texture   = nullptr;
    ScaleMode m_scaleMode = ScaleMode::Fit;
};

} // namespace nui
