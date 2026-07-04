#pragma once
// NUI: Color struct
// Extracted to avoid circular dependency between canvas.h and font.h.

#include <cstdint>

namespace nui {

struct Color {
    uint8_t r, g, b, a;
    Color() : r(255), g(255), b(255), a(255) {}
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}

    uint32_t ToRGBA() const {
        return (uint32_t(r) << 24) | (uint32_t(g) << 16) |
               (uint32_t(b) << 8)  | uint32_t(a);
    }

    static Color White()      { return {255, 255, 255, 255}; }
    static Color Black()      { return {0,   0,   0,   255}; }
    static Color Red()        { return {255, 0,   0,   255}; }
    static Color Green()      { return {0,   255, 0,   255}; }
    static Color Blue()       { return {0,   0,   255, 255}; }
    static Color Gray()       { return {128, 128, 128, 255}; }
    static Color DarkGray()   { return {64,  64,  64,  255}; }
    static Color Transparent(){ return {0,   0,   0,   0};   }
};

} // namespace nui
