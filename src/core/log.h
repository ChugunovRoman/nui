#pragma once
// NUI: Logging utility
// Outputs to stdout AND OutputDebugString (VS Output Window on Windows).

#include <cstdio>
#include <cstdarg>
#include <cstring>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
// Suppress Win32 macros that collide with our method names (e.g. Canvas::DrawText
// would be macro-expanded to DrawTextW by user32). Keep OutputDebugStringA.
#ifndef NODRAWTEXT
#define NODRAWTEXT
#endif
#include <windows.h>
#endif

namespace nui {

inline void LogToDebug(const char* msg) {
#ifdef _WIN32
    OutputDebugStringA(msg);
#endif
    (void)msg;
}

} // namespace nui

// Format to buffer, write to stdout + VS Output Window
#define NUI_LOG(...) do { \
    char _nui_buf[2048]; \
    snprintf(_nui_buf, sizeof(_nui_buf), __VA_ARGS__); \
    std::printf("%s", _nui_buf); \
    nui::LogToDebug(_nui_buf); \
} while(0)

#define NUI_LOG_ERROR(...) do { \
    char _nui_buf[2048]; \
    snprintf(_nui_buf, sizeof(_nui_buf), __VA_ARGS__); \
    std::fprintf(stderr, "%s", _nui_buf); \
    nui::LogToDebug(_nui_buf); \
} while(0)
