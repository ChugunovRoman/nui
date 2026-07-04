#pragma once
// NUI: Texture - Image loading and management
// Uses stb_image for loading, SDL_Surface for storage.
// Supports PNG, JPG, BMP, TGA, GIF.

#include <string>
#include <cstdint>
#include <map>

#include <SDL.h>

namespace nui {

class Texture {
public:
    Texture();
    ~Texture();

    // Non-copyable, movable
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    // Load from file (PNG, JPG, BMP, TGA, GIF) — tries embedded first, then disk
    bool LoadFromFile(const std::string& path);

    // Load from memory buffer (PNG, JPG, BMP, TGA, GIF)
    bool LoadFromMemory(const uint8_t* data, size_t size, const std::string& name = "");

    // Create from raw RGBA pixel data
    bool CreateFromPixels(int width, int height, const uint8_t* rgbaPixels);

    // Create a solid-color texture
    bool CreateSolid(int width, int height, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    // Free the surface
    void Free();

    // Accessors
    SDL_Surface* GetSurface() const { return m_surface; }
    int GetWidth()  const { return m_surface ? m_surface->w : 0; }
    int GetHeight() const { return m_surface ? m_surface->h : 0; }
    bool IsLoaded() const { return m_surface != nullptr; }
    const std::string& GetPath() const { return m_path; }

private:
    SDL_Surface* m_surface = nullptr;
    std::string  m_path;
};

// ── Texture cache (avoids reloading the same image) ─────────────
class TextureCache {
public:
    // Get or load a texture. Returns nullptr on failure.
    Texture* Get(const std::string& path);

    // Clear all cached textures
    void Clear();

private:
    std::map<std::string, Texture> m_cache;
};

} // namespace nui
