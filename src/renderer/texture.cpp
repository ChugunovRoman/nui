#include "core/log.h"
// NUI: Texture implementation
// Image loading via stb_image, stored as SDL_Surface for CPU rendering.

#include "renderer/texture.h"
#include "renderer/resource.h"

#include <SDL.h>
#include <cstdio>
#include <cstring>
#include <map>

// stb_image: include and define implementation in this translation unit
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "renderer/stb_image.h"

namespace nui {

Texture::Texture() {}

Texture::~Texture() {
    Free();
}

Texture::Texture(Texture&& other) noexcept
    : m_surface(other.m_surface), m_path(std::move(other.m_path))
{
    other.m_surface = nullptr;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        Free();
        m_surface = other.m_surface;
        m_path = std::move(other.m_path);
        other.m_surface = nullptr;
    }
    return *this;
}

bool Texture::LoadFromFile(const std::string& path) {
    Free();

    // Try embedded resource first
    ResourceData res = ResourceManager::Load(path);
    if (res.valid()) {
        return LoadFromMemory(res.data, res.size, path);
    }

    // Fall back to filesystem
    int w, h, channels;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!data) {
        NUI_LOG_ERROR( "[NUI] Failed to load image: %s (%s)\n",
                     path.c_str(), stbi_failure_reason());
        return false;
    }

    bool ok = CreateFromPixels(w, h, data);
    stbi_image_free(data);
    if (ok) m_path = path;
    return ok;
}

bool Texture::LoadFromMemory(const uint8_t* data, size_t size, const std::string& name) {
    Free();

    int w, h, channels;
    unsigned char* pixels = stbi_load_from_memory(data, static_cast<int>(size), &w, &h, &channels, 4);
    if (!pixels) {
        NUI_LOG_ERROR( "[NUI] Failed to load image from memory: %s\n", name.c_str());
        return false;
    }

    bool ok = CreateFromPixels(w, h, pixels);
    stbi_image_free(pixels);
    if (ok) m_path = name;
    return ok;
}

bool Texture::CreateFromPixels(int width, int height, const uint8_t* rgbaPixels) {
    Free();

    // Create an SDL_Surface in RGBA32 format
    // R/G/B/A masks for 32-bit RGBA (little-endian)
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Uint32 rmask = 0xFF000000;
    Uint32 gmask = 0x00FF0000;
    Uint32 bmask = 0x0000FF00;
    Uint32 amask = 0x000000FF;
#else
    Uint32 rmask = 0x000000FF;
    Uint32 gmask = 0x0000FF00;
    Uint32 bmask = 0x00FF0000;
    Uint32 amask = 0xFF000000;
#endif

    m_surface = SDL_CreateRGBSurfaceFrom(
        const_cast<void*>(static_cast<const void*>(rgbaPixels)),
        width, height, 32, width * 4,
        rmask, gmask, bmask, amask
    );

    if (!m_surface) {
        NUI_LOG_ERROR( "[NUI] SDL_CreateRGBSurfaceFrom failed: %s\n", SDL_GetError());
        return false;
    }

    // Make a copy so we own the pixel data (stb_image frees its buffer)
    SDL_Surface* copy = SDL_ConvertSurface(m_surface, m_surface->format, 0);
    SDL_FreeSurface(m_surface);
    m_surface = copy;

    return m_surface != nullptr;
}

bool Texture::CreateSolid(int width, int height, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    Free();

    m_surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    if (!m_surface) return false;

    Uint32 color = SDL_MapRGBA(m_surface->format, r, g, b, a);
    SDL_FillRect(m_surface, nullptr, color);
    return true;
}

void Texture::Free() {
    if (m_surface) {
        SDL_FreeSurface(m_surface);
        m_surface = nullptr;
    }
    m_path.clear();
}

// ── TextureCache ────────────────────────────────────────────────

Texture* TextureCache::Get(const std::string& path) {
    auto it = m_cache.find(path);
    if (it != m_cache.end()) {
        return &it->second;
    }

    Texture tex;
    if (!tex.LoadFromFile(path)) {
        return nullptr;
    }

    auto [ins, ok] = m_cache.emplace(path, std::move(tex));
    return &ins->second;
}

void TextureCache::Clear() {
    m_cache.clear();
}

} // namespace nui
