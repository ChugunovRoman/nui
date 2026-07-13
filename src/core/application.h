#pragma once
// NUI: Application core
// Manages the main window, event loop, and widget tree.

#include <string>
#include <memory>
#include <functional>
#include <vector>

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

    // ── Overlay layer (popup / modal) ────────────────────────────
    // A separate stack of widgets rendered on top of the root tree and
    // receiving input before it. Used by Tooltip, Menu/ContextMenu and
    // Dialog/MessageBox. This generalises the single capture-widget above:
    // multiple overlays may coexist (e.g. a submenu over a parent menu).
    struct OverlayEntry {
        std::unique_ptr<Widget> widget;
        bool modal = false;              // modal: blocks input to root and lower overlays
        bool closeOnOutsideClick = false; // (non-modal) a click outside dismisses it
    };

    // Push a widget onto the overlay stack. modal overlays dim the background
    // and swallow all input (root is not polled). closeOnOutsideClick makes a
    // non-modal overlay dismiss itself when the user clicks outside its rect
    // (typical for menus). Returns a raw pointer to the pushed widget.
    Widget* PushOverlay(std::unique_ptr<Widget> w, bool modal, bool closeOnOutsideClick);

    // Remove a specific overlay widget from the stack (by raw pointer).
    void PopOverlay(Widget* w);

    // Remove and drop all overlays.
    void ClearOverlays();

    // Read-only access to the overlay stack.
    const std::vector<OverlayEntry>& GetOverlays() const { return m_overlayStack; }

    // Built-in tooltip support. When enabled (default), the Application polls
    // the widget under the cursor each frame and shows a TooltipWidget on the
    // overlay stack after a hover delay, using each widget's SetTooltip text.
    void SetTooltipEnabled(bool enabled) { m_tooltipEnabled = enabled; }
    bool IsTooltipEnabled() const { return m_tooltipEnabled; }
    // Hover delay (seconds) before a tooltip appears.
    void SetTooltipDelay(float seconds);
    float GetTooltipDelay() const;

    // Exit the main loop
    void Quit() { m_running = false; }

    // Async: dispatch a callback to run on the main thread next frame
    static void DispatchOnMainThread(std::function<void()> cb);

private:
    void ProcessEvents();
    void Render();
    // Route input through the overlay stack (top first). Returns true if the
    // event was consumed (root should not be polled). See ProcessEvents.
    bool DispatchOverlayInput(InputState& input);

    SDL_Window*              m_window   = nullptr;
    SDL_Surface*             m_screen   = nullptr;
    std::unique_ptr<Canvas>       m_canvas;
    std::unique_ptr<FontManager>  m_fontManager;
    std::unique_ptr<InputState>   m_input;
    std::unique_ptr<Widget>       m_root;
    Widget*                       m_captureWidget = nullptr;
    std::vector<OverlayEntry>     m_overlayStack;
    bool                     m_tooltipEnabled = true;
    TickCallback             m_onTick;

    int  m_width   = 0;
    int  m_height  = 0;
    bool m_running = false;
};

// Singleton access (convenience)
Application* GetApp();

} // namespace nui
