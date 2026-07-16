#pragma once
// NUI: Application core
// Manages the main window, event loop, and widget tree.

#include <string>
#include <memory>
#include <functional>
#include <vector>

#include <SDL.h>

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
    std::string title      = "NUI";
    std::string iconPath;              // path to PNG/BMP icon for window + taskbar
    int         width      = 1024;
    int         height     = 768;
    bool        resizable  = true;
    bool        borderless = false;    // SDL_WINDOW_BORDERLESS (no frame)

    // ── Borderless customization (ISSUE 1: no hardcoded magic numbers) ──
    // All sizes are in logical (DPI-scaled) pixels. They override the built-in
    // defaults; leave at 0 to keep the default.
    int  resizeBorderWidth = 5;   // px hit zone for edge/corner resize
    int  minWindowWidth    = 100; // minimum window size enforced on resize
    int  minWindowHeight   = 100;
    int  titlebarButtonSize = 28; // square size of titlebar min/max/close buttons
    int  doubleClickMs     = 500; // double-click window for titlebar maximize
    int  dragThreshold     = 4;   // px the mouse must travel before a drag starts
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
    struct SDL_Window* GetSDLWindow()  const { return m_window; }
    bool              IsBorderless()   const { return m_borderless; }
    bool              IsMaximized()    const { return m_maximized; }

    // Window management (borderless mode)
    void ToggleMaximize();

    // Borderless customization (ISSUE 1: configurable, DPI-aware)
    void SetResizeBorderWidth(int px) { m_resizeBorderWidth = px; }
    int  GetResizeBorderWidth() const { return m_resizeBorderWidth; }
    void SetMinWindowSize(int w, int h) { m_minWindowW = w; m_minWindowH = h; }
    int  GetMinWindowWidth()  const { return m_minWindowW; }
    int  GetMinWindowHeight() const { return m_minWindowH; }

    // Drag threshold (ISSUE 1/3): minimal mouse travel before a window drag
    // begins. Prevents a click from nudging the window.
    void SetDragThreshold(int px) { m_dragThreshold = px; }
    int  GetDragThreshold() const { return m_dragThreshold; }

    // Titlebar button size and double-click window (ISSUE 1). Exposed here so
    // TitlebarWidget can read the app-wide settings; the titlebar also keeps
    // per-widget overrides for standalone use.
    void SetTitlebarButtonSize(int px) { m_titlebarButtonSize = px; }
    int  GetTitlebarButtonSize() const { return m_titlebarButtonSize; }
    void SetDoubleClickMs(int ms) { m_doubleClickMs = ms; }
    int  GetDoubleClickMs() const { return m_doubleClickMs; }

    // ── DPI awareness (ISSUE 1: HiDPI scaling) ─────────────────────
    // Returns the content scale factor of the window's display (>= 1.0).
    // On HiDPI monitors SDL reports a scale > 1; borderless geometry (resize
    // border, button size, drag threshold) is multiplied by this factor so the
    // hit zones stay comfortable regardless of the display DPI.
    float GetDpiScale() const { return m_dpiScale; }
    void  SetDpiScale(float scale);  // override (e.g. from app settings)
    // Logical->physical pixel helper for borderless hit testing.
    int   ScalePx(int logicalPx) const;

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

    // ── Borderless window management ───────────────────────────
    bool m_borderless = false;
    bool m_maximized  = false;
    // Saved window rect before maximize (for borderless manual maximize)
    int  m_restoreX = 0, m_restoreY = 0;
    int  m_restoreW = 0, m_restoreH = 0;
    // ISSUE 2: true once a valid pre-maximize rect has been captured. Guards
    // RestoreFromMaximize() against using stale/garbage coordinates (e.g. when
    // the OS maximized the window via Win+Up before we ever saved a rect).
    bool m_hasRestoreRect = false;

    // Drag state (window move via draggable widgets)
    bool m_dragging    = false;
    int  m_dragOffsetX = 0;
    int  m_dragOffsetY = 0;

    // Pending drag: mouse pressed on a draggable widget, but threshold not
    // yet crossed. Promoted to m_dragging once the mouse moves beyond the
    // threshold (prevents accidental drag on simple clicks).
    bool m_dragPending    = false;
    int  m_dragStartMouseX = 0;
    int  m_dragStartMouseY = 0;
    int  m_dragThreshold = 4;  // px — min movement to start drag (ISSUE 1: configurable)

    // Resize state (border resize in borderless mode)
    bool m_resizing = false;
    int  m_resizeEdge = 0;  // bitmask: 1=Left, 2=Right, 4=Top, 8=Bottom
    int  m_resizeStartMouseX = 0, m_resizeStartMouseY = 0;
    int  m_resizeStartWinX   = 0, m_resizeStartWinY   = 0;
    int  m_resizeStartW      = 0, m_resizeStartH      = 0;

    int m_resizeBorderWidth = 5;   // customizable via SetResizeBorderWidth
    int m_minWindowW = 100;        // customizable via SetMinWindowSize
    int m_minWindowH = 100;

    // Titlebar defaults shared via AppDesc (ISSUE 1)
    int m_titlebarButtonSize = 28;
    int m_doubleClickMs = 500;

    // DPI content scale of the window's display (ISSUE 1: HiDPI)
    float m_dpiScale = 1.0f;

    // Resize cursors: [0]=nwse, [1]=nesw, [2]=horiz, [3]=vert
    SDL_Cursor* m_resizeCursors[4] = {};
    SDL_Cursor* m_defaultCursor    = nullptr;

    int GetResizeEdge(int mx, int my) const;
    void UpdateResizeCursor(int mx, int my);
    void RestoreFromMaximize();
};

// Singleton access (convenience)
Application* GetApp();

} // namespace nui
