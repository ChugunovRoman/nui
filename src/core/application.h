#pragma once
// NUI: Application core
// Manages the main window, event loop, and widget tree.

#include <string>
#include <memory>
#include <functional>

struct SDL_Window;
struct SDL_Surface;

namespace nui {

// Forward declarations
class Canvas;
class Widget;
class FontManager;
class InputState;

// ── Application configuration ───────────────────────────────────
struct AppDesc {
    std::string title   = "NUI";
    int         width   = 1024;
    int         height  = 768;
    bool        resizable = true;
};

// ── Application class ───────────────────────────────────────────
class Application {
public:
    Application();
    ~Application();

    // Initialize SDL2 window with software renderer
    bool Initialize(const AppDesc& desc);

    // Run the main loop. Returns exit code.
    int Run();

    // Shutdown and release resources
    void Shutdown();

    // Access subsystems
    Canvas*           GetCanvas()      const { return m_canvas.get(); }
    FontManager*      GetFontManager() const { return m_fontManager.get(); }
    const InputState* GetInput()       const { return m_input.get(); }
    int               GetWidth()       const { return m_width; }
    int               GetHeight()      const { return m_height; }

    // Root widget (the entire UI tree hangs from here)
    Widget*           GetRoot()        const { return m_root.get(); }
    void              SetRoot(std::unique_ptr<Widget> root);

    // Capture widget: receives input FIRST, before the normal widget tree is
    // traversed. Used by widgets that need to grab all input while in a
    // transient state (e.g. an expanded Dropdown that must close on any click,
    // a Slider being dragged). Passing nullptr clears the capture.
    Widget*           GetCaptureWidget() const { return m_captureWidget; }
    void              SetCaptureWidget(Widget* w) { m_captureWidget = w; }

    // Callbacks
    using TickCallback = std::function<void(float dt)>;
    void SetOnTick(TickCallback cb) { m_onTick = std::move(cb); }

    // Exit the main loop
    void Quit() { m_running = false; }

    // Async: dispatch a callback to run on the main thread next frame
    static void DispatchOnMainThread(std::function<void()> cb);

private:
    void ProcessEvents();
    void Render();

    SDL_Window*              m_window   = nullptr;
    SDL_Surface*             m_screen   = nullptr;
    std::unique_ptr<Canvas>       m_canvas;
    std::unique_ptr<FontManager>  m_fontManager;
    std::unique_ptr<InputState>   m_input;
    std::unique_ptr<Widget>       m_root;
    Widget*                       m_captureWidget = nullptr;
    TickCallback             m_onTick;

    int  m_width   = 0;
    int  m_height  = 0;
    bool m_running = false;
};

// Singleton access (convenience)
Application* GetApp();

} // namespace nui
