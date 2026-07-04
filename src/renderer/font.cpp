#include "core/log.h"
// NUI: Font implementation
// SDL_ttf for text rendering, supports TTF and OTF fonts.

#include "renderer/font.h"
#include "renderer/canvas.h"  // for Color
#include "renderer/resource.h"
#include <cstdio>

namespace nui {

// ── Font ────────────────────────────────────────────────────────

Font::Font() {}

Font::~Font() {
    Free();
}

bool Font::Load(const std::string& path, int size) {
    Free();

    // Try embedded resource first
    ResourceData res = ResourceManager::Load(path);
    if (res.valid()) {
        return LoadFromMemory(res.data, res.size, size);
    }

    // Fall back to filesystem
    m_font = TTF_OpenFont(path.c_str(), size);
    if (!m_font) {
        NUI_LOG_ERROR( "[NUI] TTF_OpenFont failed: %s (%s)\n",
                     path.c_str(), TTF_GetError());
        return false;
    }
    m_size = size;
    return true;
}

bool Font::LoadFromMemory(const uint8_t* data, size_t size, int fontSize) {
    Free();
    SDL_RWops* rw = SDL_RWFromConstMem(data, static_cast<int>(size));
    if (!rw) {
        NUI_LOG_ERROR( "[NUI] SDL_RWFromConstMem failed\n");
        return false;
    }
    m_font = TTF_OpenFontRW(rw, 1, fontSize);  // 1 = auto-close RWops
    if (!m_font) {
        NUI_LOG_ERROR( "[NUI] TTF_OpenFontRW failed: %s\n", TTF_GetError());
        return false;
    }
    m_size = fontSize;
    return true;
}

SDL_Surface* Font::RenderText(const std::string& text, const Color& color) const {
    if (!m_font || text.empty()) return nullptr;

    SDL_Color sc = {color.r, color.g, color.b, color.a};
    // TTF_RenderUTF8_Blended produces high-quality anti-aliased text
    return TTF_RenderUTF8_Blended(m_font, text.c_str(), sc);
}

void Font::FreeRenderedSurface(SDL_Surface* surface) const {
    if (surface) {
        SDL_FreeSurface(surface);
    }
}

int Font::GetHeight() const {
    return m_font ? TTF_FontHeight(m_font) : 0;
}

int Font::GetTextWidth(const std::string& text) const {
    if (!m_font || text.empty()) return 0;
    int w = 0;
    TTF_SizeUTF8(m_font, text.c_str(), &w, nullptr);
    return w;
}

void Font::Free() {
    if (m_font) {
        TTF_CloseFont(m_font);
        m_font = nullptr;
    }
}

// ── FontManager ─────────────────────────────────────────────────

FontManager::FontManager() {}

FontManager::~FontManager() {
    Shutdown();
}

bool FontManager::Initialize() {
    if (TTF_Init() != 0) {
        NUI_LOG_ERROR( "[NUI] TTF_Init failed: %s\n", TTF_GetError());
        return false;
    }

    // Try to find a default font
    // First check bundled fonts, then system fonts
    const char* candidates[] = {
        "resources/fonts/Roboto-Regular.ttf",
        "resources/fonts/DejaVuSans.ttf",
        "resources/fonts/arial.ttf",
        // Linux system fonts
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        // macOS system fonts
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/SFNSText.ttf",
        // Windows system fonts
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
    };

    for (const char* path : candidates) {
        TTF_Font* test = TTF_OpenFont(path, 16);
        if (test) {
            TTF_CloseFont(test);
            m_defaultFontPath = path;
            NUI_LOG("[NUI] Default font: %s\n", path);
            break;
        }
    }

    if (m_defaultFontPath.empty()) {
        NUI_LOG_ERROR( "[NUI] WARNING: No default font found!\n");
        return false;
    }

    return true;
}

void FontManager::Shutdown() {
    m_fonts.clear();
    TTF_Quit();
}

Font* FontManager::Get(const std::string& path, int size) {
    auto key = std::make_pair(path, size);
    auto it = m_fonts.find(key);
    if (it != m_fonts.end()) {
        return &it->second;
    }

    auto [ins, ok] = m_fonts.try_emplace(key);
    if (!ins->second.Load(path, size)) {
        m_fonts.erase(ins);
        return nullptr;
    }
    return &ins->second;
}

Font* FontManager::GetDefault(int size) {
    if (m_defaultFontPath.empty()) return nullptr;
    return Get(m_defaultFontPath, size);
}

} // namespace nui
