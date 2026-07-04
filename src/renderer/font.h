#pragma once
// NUI: Font rendering via SDL_ttf (FreeType wrapper)
// Supports TTF/OTF fonts with configurable size and style.

#include <string>
#include <map>
#include <cstdint>

#include <SDL.h>
#include <SDL_ttf.h>
#include "renderer/color.h"

namespace nui {

// ── Font ────────────────────────────────────────────────────────
class Font {
public:
    Font();
    ~Font();

    // Non-copyable
    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    // Load TTF/OTF font at given pixel size (tries embedded first, then disk)
    bool Load(const std::string& path, int size);

    // Load from memory buffer
    bool LoadFromMemory(const uint8_t* data, size_t size, int fontSize);

    // Render text to a new SDL_Surface (caller must free with FreeRenderedSurface)
    SDL_Surface* RenderText(const std::string& text, const Color& color) const;

    // Free a surface returned by RenderText
    void FreeRenderedSurface(SDL_Surface* surface) const;

    // Metrics
    int GetHeight() const;
    int GetTextWidth(const std::string& text) const;
    int GetSize() const { return m_size; }

    void Free();

private:
    TTF_Font* m_font = nullptr;
    int       m_size = 16;
};

// ── FontManager ─────────────────────────────────────────────────
// Caches loaded fonts by path+size pair.
class FontManager {
public:
    FontManager();
    ~FontManager();

    bool Initialize();
    void Shutdown();

    // Get or load a font. Returns nullptr on failure.
    Font* Get(const std::string& path, int size);

    // Default font (bundled or system fallback)
    Font* GetDefault(int size = 16);

private:
    std::map<std::pair<std::string, int>, Font> m_fonts;
    std::string m_defaultFontPath;
};

} // namespace nui
