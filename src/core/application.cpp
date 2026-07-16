#include "core/log.h"
// NUI: Application implementation
// SDL2 window creation, event loop, software rendering pipeline.

#include "core/application.h"
#include "core/input.h"
#include "core/async.h"
#include "animation/animator.h"
#include "renderer/canvas.h"
#include "renderer/font.h"
#include "renderer/texture.h"
#include "ui/widget.h"
#include "ui/tooltip.h"

#include <SDL.h>
#include <cstdio>

namespace nui {

static Application* g_app = nullptr;
Application* GetApp() { return g_app; }

Application::Application() {
    g_app = this;
}

Application::~Application() {
    Shutdown();
    g_app = nullptr;
}

bool Application::Initialize(const AppDesc& desc) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        NUI_LOG_ERROR( "[NUI] SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    m_width  = desc.width;
    m_height = desc.height;
    m_borderless = desc.borderless;

    // ISSUE 1: take configurable borderless constants from AppDesc (they
    // override the defaults; non-zero values win).
    if (desc.resizeBorderWidth > 0) m_resizeBorderWidth = desc.resizeBorderWidth;
    if (desc.minWindowWidth  > 0) m_minWindowW = desc.minWindowWidth;
    if (desc.minWindowHeight > 0) m_minWindowH = desc.minWindowHeight;
    if (desc.titlebarButtonSize > 0) m_titlebarButtonSize = desc.titlebarButtonSize;
    if (desc.doubleClickMs > 0) m_doubleClickMs = desc.doubleClickMs;
    if (desc.dragThreshold > 0) m_dragThreshold = desc.dragThreshold;

    // Create window - no GPU flags, pure software
    Uint32 flags = SDL_WINDOW_SHOWN;
    if (desc.resizable)
        flags |= SDL_WINDOW_RESIZABLE;
    if (desc.borderless)
        flags |= SDL_WINDOW_BORDERLESS;

    m_window = SDL_CreateWindow(
        desc.title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        m_width, m_height,
        flags
    );
    if (!m_window) {
        NUI_LOG_ERROR( "[NUI] SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    // Set window icon if path provided (ISSUE 4: load via the shared helper so
    // stb_image stays confined to texture.cpp and no other TU includes it).
    if (!desc.iconPath.empty()) {
        SDL_Surface* icon = LoadImageToSurface(desc.iconPath);
        if (icon) {
            int iw = icon->w, ih = icon->h;
            SDL_SetWindowIcon(m_window, icon);
            SDL_FreeSurface(icon);
            NUI_LOG("[NUI] Icon loaded: %s (%dx%d)\n", desc.iconPath.c_str(), iw, ih);
        }
    }

    // Create resize cursors for borderless mode
    if (m_borderless) {
        m_resizeCursors[0] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE); // nwse
        m_resizeCursors[1] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW); // nesw
        m_resizeCursors[2] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);   // horiz
        m_resizeCursors[3] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);   // vert
        m_defaultCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    }

    // Get the window surface (CPU-only, no renderer)
    m_screen = SDL_GetWindowSurface(m_window);
    if (!m_screen) {
        NUI_LOG_ERROR( "[NUI] SDL_GetWindowSurface failed: %s\n", SDL_GetError());
        return false;
    }

    // ISSUE 1: detect display content scale for HiDPI-aware borderless geometry.
    // SDL_GetDisplayDPI returns raw DPI; we derive a content scale relative to
    // a 96 dpi baseline (the logical-pixel convention SDL/Windows use).
    if (m_dpiScale == 1.0f) {  // not overridden via SetDpiScale
        float ddpi = 0.0f;
        int display = SDL_GetWindowDisplayIndex(m_window);
        if (display >= 0 && SDL_GetDisplayDPI(display, &ddpi, nullptr, nullptr) == 0 && ddpi > 0.0f) {
            m_dpiScale = ddpi / 96.0f;
            if (m_dpiScale < 1.0f) m_dpiScale = 1.0f;
        }
    }
    // Apply DPI to the configurable borderless hit zones so they stay usable on
    // high-density displays.
    m_resizeBorderWidth = ScalePx(m_resizeBorderWidth);
    m_dragThreshold     = ScalePx(m_dragThreshold);
    m_titlebarButtonSize = ScalePx(m_titlebarButtonSize);

    // Initialize subsystems
    m_canvas = std::make_unique<Canvas>();
    if (!m_canvas->Initialize(m_screen)) {
        NUI_LOG_ERROR( "[NUI] Canvas initialization failed\n");
        return false;
    }

    m_fontManager = std::make_unique<FontManager>();
    if (!m_fontManager->Initialize()) {
        NUI_LOG_ERROR( "[NUI] FontManager initialization failed\n");
        return false;
    }

    m_input = std::make_unique<InputState>();

    // Create default root widget (fullscreen)
    m_root = std::make_unique<Widget>();
    m_root->SetRect(0, 0, m_width, m_height);
    m_root->UpdateLayout();

    m_running = true;
    NUI_LOG("[NUI] Initialized: %dx%d (CPU software rendering)\n", m_width, m_height);
    return true;
}

void Application::SetRoot(std::unique_ptr<Widget> root) {
    // Clear capture before swapping the tree — the captured widget belonged to
    // the old tree and would become a dangling pointer otherwise.
    m_captureWidget = nullptr;
    // Overlays reference the old tree too; drop them for the same reason.
    // Notify the tooltip manager first: it holds a raw pointer to a tooltip
    // widget owned by the overlay stack, which is about to be destroyed.
    TooltipManager::Instance().OnOverlaysCleared();
    m_overlayStack.clear();
    m_root = std::move(root);
    if (m_root) {
        m_root->SetRect(0, 0, m_width, m_height);
        m_root->UpdateLayout();
    }
}

Widget* Application::PushOverlay(std::unique_ptr<Widget> w, bool modal, bool closeOnOutsideClick) {
    if (!w) return nullptr;
    Widget* raw = w.get();
    OverlayEntry entry;
    entry.widget = std::move(w);
    entry.modal = modal;
    entry.closeOnOutsideClick = closeOnOutsideClick;
    m_overlayStack.push_back(std::move(entry));
    return raw;
}

void Application::PopOverlay(Widget* w) {
    if (!w) return;
    for (auto it = m_overlayStack.begin(); it != m_overlayStack.end(); ++it) {
        if (it->widget.get() == w) {
            m_overlayStack.erase(it);
            return;
        }
    }
}

void Application::ClearOverlays() {
    TooltipManager::Instance().OnOverlaysCleared();
    m_overlayStack.clear();
}

int Application::Run() {
    Uint64 lastTick = SDL_GetPerformanceCounter();
    const Uint64 freq = SDL_GetPerformanceFrequency();

    while (m_running) {
        // Calculate delta time
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = static_cast<float>(now - lastTick) / static_cast<float>(freq);
        lastTick = now;

        // Cap delta time to avoid spiral of death
        if (dt > 0.1f) dt = 0.1f;

        // Process SDL events
        m_input->BeginFrame();
        ProcessEvents();

        // User tick callback
        if (m_onTick) {
            m_onTick(dt);
        }

        // Process async callbacks from background threads
        Async::ProcessMainThreadQueue();

        // Update all active animations
        Animator::UpdateAll(dt);

        // Update and render UI tree
        if (m_root) {
            m_root->Update(dt);
        }

        // Update overlay widgets (popups, menus, dialogs)
        for (auto& entry : m_overlayStack) {
            entry.widget->Update(dt);
        }

        // Drive the built-in tooltip manager: find the widget under the cursor
        // and show its tooltip after the hover delay.
        if (m_tooltipEnabled && m_root && m_input) {
            TooltipManager::Instance().Update(
                m_root.get(),
                m_input->GetMouseX(), m_input->GetMouseY(),
                m_width, m_height, dt);
        }

        Render();

        // Update the window surface
        SDL_UpdateWindowSurface(m_window);
    }

    return 0;
}

void Application::ProcessEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                m_running = false;
                break;

            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        m_width  = event.window.data1;
                        m_height = event.window.data2;
                        m_screen = SDL_GetWindowSurface(m_window);
                        if (m_screen) {
                            m_canvas->SetSurface(m_screen);
                        }
                        if (m_root) {
                            m_root->SetRect(0, 0, m_width, m_height);
                            // Recompute anchored descendants against the new size.
                            m_root->UpdateLayout();
                        }
                        break;
                    }
                    // BUG 2: keep m_maximized in sync with OS-initiated state
                    // changes (Win+Up/Down, Aero Snap, Snap Layouts on Win11).
                    case SDL_WINDOWEVENT_MAXIMIZED:
                        m_maximized = true;
                        break;
                    case SDL_WINDOWEVENT_MINIMIZED:
                    case SDL_WINDOWEVENT_RESTORED:
                        m_maximized = false;
                        break;
                    // BUG 5: reset drag/resize if focus is lost mid-drag
                    // (e.g. Alt-Tab while dragging leaves dangling state).
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        m_dragging = false;
                        m_dragPending = false;
                        m_resizing = false;
                        break;
                    // BUG 7: reset cursor when mouse leaves the window
                    case SDL_WINDOWEVENT_LEAVE:
                        if (m_borderless && m_defaultCursor) {
                            SDL_SetCursor(m_defaultCursor);
                        }
                        break;
                }
                break;

            case SDL_MOUSEMOTION:
                m_input->SetMousePos(event.motion.x, event.motion.y);

                // ── Window drag (borderless mode) ──────────────
                // Promote pending drag to active only after the mouse crosses
                // m_dragThreshold pixels (ISSUE 1/3), so a simple click on the
                // titlebar does not move the window.
                if (m_dragPending && !m_dragging) {
                    int dx = event.motion.x - m_dragStartMouseX;
                    int dy = event.motion.y - m_dragStartMouseY;
                    if (dx*dx + dy*dy >= m_dragThreshold * m_dragThreshold) {
                        m_dragging = true;
                        m_dragOffsetX = m_dragStartMouseX;
                        m_dragOffsetY = m_dragStartMouseY;
                    }
                }

                if (m_dragging) {
                    // BUG 4: dragging a maximized window restores it first
                    // (standard OS behaviour). The restore rect was captured
                    // at maximize time.
                    if (m_maximized) {
                        RestoreFromMaximize();
                    }
                    int wx, wy;
                    SDL_GetWindowPosition(m_window, &wx, &wy);
                    SDL_SetWindowPosition(m_window,
                        wx + event.motion.x - m_dragOffsetX,
                        wy + event.motion.y - m_dragOffsetY);
                    break;  // skip widget dispatch
                }

                // ── Window resize (borderless mode) ────────────
                if (m_resizing) {
                    int dx = event.motion.x - m_resizeStartMouseX;
                    int dy = event.motion.y - m_resizeStartMouseY;
                    int newX = m_resizeStartWinX;
                    int newY = m_resizeStartWinY;
                    int newW = m_resizeStartW;
                    int newH = m_resizeStartH;

                    if (m_resizeEdge & 1) { // Left
                        newX = m_resizeStartWinX + dx;
                        newW = m_resizeStartW - dx;
                    }
                    if (m_resizeEdge & 2) { // Right
                        newW = m_resizeStartW + dx;
                    }
                    if (m_resizeEdge & 4) { // Top
                        newY = m_resizeStartWinY + dy;
                        newH = m_resizeStartH - dy;
                    }
                    if (m_resizeEdge & 8) { // Bottom
                        newH = m_resizeStartH + dy;
                    }

                    // Enforce minimum size (BUG 1 customization)
                    if (newW < m_minWindowW) {
                        if (m_resizeEdge & 1) newX = m_resizeStartWinX + m_resizeStartW - m_minWindowW;
                        newW = m_minWindowW;
                    }
                    if (newH < m_minWindowH) {
                        if (m_resizeEdge & 4) newY = m_resizeStartWinY + m_resizeStartH - m_minWindowH;
                        newH = m_minWindowH;
                    }

                    SDL_SetWindowPosition(m_window, newX, newY);
                    SDL_SetWindowSize(m_window, newW, newH);
                    break;  // skip widget dispatch
                }

                // Update resize cursor when not dragging/resizing
                if (m_borderless && !m_dragging && !m_resizing && !m_dragPending) {
                    UpdateResizeCursor(event.motion.x, event.motion.y);
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                bool down = (event.type == SDL_MOUSEBUTTONDOWN);
                MouseButton btn = MouseButton::Left;
                if (event.button.button == SDL_BUTTON_MIDDLE) btn = MouseButton::Middle;
                if (event.button.button == SDL_BUTTON_RIGHT)  btn = MouseButton::Right;

                // ── Borderless: intercept resize border before widgets ──
                // BUG 4: no resize while maximized
                if (down && event.button.button == SDL_BUTTON_LEFT &&
                    m_borderless && !m_maximized) {
                    int edge = GetResizeEdge(event.button.x, event.button.y);
                    if (edge != 0) {
                        m_resizing = true;
                        m_resizeEdge = edge;
                        m_resizeStartMouseX = event.button.x;
                        m_resizeStartMouseY = event.button.y;
                        SDL_GetWindowPosition(m_window, &m_resizeStartWinX, &m_resizeStartWinY);
                        m_resizeStartW = m_width;
                        m_resizeStartH = m_height;
                        break;  // consume
                    }
                }

                // End drag/resize on mouse up
                if (!down && event.button.button == SDL_BUTTON_LEFT &&
                    (m_dragging || m_dragPending || m_resizing)) {
                    m_dragging = false;
                    m_dragPending = false;
                    m_resizing = false;
                    break;
                }

                m_input->SetMouseButton(btn, down);
                break;
            }

            case SDL_MOUSEWHEEL:
                m_input->SetWheelY(event.wheel.y);
                break;

            case SDL_KEYDOWN:
                m_input->SetKeyDown(event.key.keysym.sym);
                break;
            case SDL_KEYUP:
                m_input->SetKeyUp(event.key.keysym.sym);
                break;

            case SDL_TEXTINPUT:
                m_input->AppendTextInput(event.text.text);
                break;
        }
    }

    // Deliver events to widget tree.
    if (m_root) {
        // A capture widget (if any) gets the first shot at input — this lets
        // e.g. an expanded Dropdown close on an outside click regardless of
        // which widget the cursor is over. If it consumes the event, skip the
        // normal tree traversal to avoid double-processing the same click.
        bool consumed = false;
        if (m_captureWidget) {
            consumed = m_captureWidget->HandleInput(*m_input);
        }

        // Overlay layer: popups, menus and dialogs are dispatched before the
        // root tree. Traverse top-of-stack first (LIFO). A modal overlay swallows
        // the event entirely (root is never polled while it is up). A non-modal
        // overlay with closeOnOutsideClick dismisses itself on a click that
        // lands outside its rect.
        if (!consumed && !m_overlayStack.empty()) {
            consumed = DispatchOverlayInput(*m_input);
        }

        if (!consumed) {
            consumed = m_root->HandleInput(*m_input);
        }

        // ── Borderless drag detection (AFTER widget dispatch) ──
        // Only start a window drag if the left click was NOT consumed by
        // any widget (e.g. a button). This lets buttons work normally
        // while still allowing drag on empty draggable areas.
        // BUG 3: walk up the parent chain — a click on a child of a
        // draggable widget (e.g. a label inside a draggable panel) should
        // still move the window.
        if (!consumed && m_borderless && !m_dragging && !m_dragPending && !m_resizing &&
            m_input->IsMouseClicked(MouseButton::Left)) {
            int mx = m_input->GetMouseX();
            int my = m_input->GetMouseY();
            Widget* w = m_root->FindWidgetAt(mx, my);
            while (w) {
                if (w->IsWindowDragEnabled()) {
                    // BUG 1: don't start dragging immediately — set a pending
                    // state and wait for the mouse to cross the threshold.
                    m_dragPending = true;
                    m_dragStartMouseX = mx;
                    m_dragStartMouseY = my;
                    break;
                }
                w = w->GetParent();
            }
        }
    }
}

bool Application::DispatchOverlayInput(InputState& input) {
    const int mx = input.GetMouseX();
    const int my = input.GetMouseY();
    const bool leftClicked = input.IsMouseClicked(MouseButton::Left);
    const bool anyButtonDown = input.IsMouseDown(MouseButton::Left) ||
                               input.IsMouseDown(MouseButton::Middle) ||
                               input.IsMouseDown(MouseButton::Right);

    // Walk from the topmost overlay down. Once we hit a modal one we stop:
    // it blocks everything beneath it (root + lower overlays).
    //
    // IMPORTANT: an overlay's HandleInput may close itself (e.g. a Dialog
    // button click, a Menu action) and thus mutate m_overlayStack. After
    // calling HandleInput we must NOT touch the entry/widget references we
    // held before — capture the flags we need into locals up front and
    // re-check the stack size before further iteration.
    for (int i = static_cast<int>(m_overlayStack.size()) - 1; i >= 0; --i) {
        OverlayEntry& entry = m_overlayStack[i];
        Widget* w = entry.widget.get();
        const bool entryModal = entry.modal;
        const bool entryCloseOutside = entry.closeOnOutsideClick;
        const bool inside = w->HitTest(mx, my);

        // Close-on-outside-click: only react to an actual click (not a held
        // button or plain motion). Dismiss the overlay and consume the click
        // so it never reaches root or other overlays.
        if (entryCloseOutside && leftClicked && !inside) {
            m_overlayStack.erase(m_overlayStack.begin() + i);
            return true; // consume the click
        }

        // Offer the input to this overlay. If it consumes it, we stop. Note
        // that HandleInput may erase entries (including this one) from the
        // stack — do not use `entry`/`w`/`inside` after this point.
        const bool consumed = w->HandleInput(input);

        if (consumed) {
            // A modal overlay always blocks downward propagation.
            if (entryModal) return true;
            // Non-modal: if the click was inside it, stop propagation too;
            // otherwise let lower overlays / root see it.
            if (inside || anyButtonDown) return true;
        }

        // A modal overlay stops the descent regardless of consumption.
        if (entryModal) return true;
    }
    return false;
}

void Application::Render() {
    // Lock surface for direct pixel access (needed for software rendering)
    if (SDL_MUSTLOCK(m_screen)) {
        SDL_LockSurface(m_screen);
    }

    // Clear with dark background
    m_canvas->Clear(Color(30, 30, 40, 255));

    // Render widget tree
    if (m_root) {
        m_root->Render(*m_canvas, *m_fontManager);
    }

    // Overlay pass: render popups/menus/dialogs on top. At this point the
    // clip stack is empty (root never leaves an active clip), so overlays may
    // draw in arbitrary screen coordinates without the Dropdown-style clip
    // save/restore dance. Draw bottom-up so the topmost overlay is on top.
    for (auto& entry : m_overlayStack) {
        if (entry.modal) {
            // Dim background so the modal stands out.
            m_canvas->FillRect(Rect(0, 0, m_width, m_height), Color(0, 0, 0, 120));
        }
        entry.widget->Render(*m_canvas, *m_fontManager);
    }

    if (SDL_MUSTLOCK(m_screen)) {
        SDL_UnlockSurface(m_screen);
    }
}

void Application::DispatchOnMainThread(std::function<void()> cb) {
    Async::DispatchOnMainThread(std::move(cb));
}

void Application::SetTooltipDelay(float seconds) {
    TooltipManager::Instance().SetShowDelay(seconds);
}

float Application::GetTooltipDelay() const {
    return TooltipManager::Instance().GetShowDelay();
}

// ── Window management (borderless mode) ────────────────────────

void Application::ToggleMaximize() {
    if (!m_window) return;
    if (m_maximized) {
        RestoreFromMaximize();
    } else {
        // ISSUE 2: capture the restore rect ONLY when we are not already
        // maximized. Without this guard, a rapid double-toggle
        // (maximize -> maximize) would overwrite the saved rect with the
        // maximized geometry and the window could never return to its real
        // pre-maximize size.
        SDL_GetWindowPosition(m_window, &m_restoreX, &m_restoreY);
        SDL_GetWindowSize(m_window, &m_restoreW, &m_restoreH);
        m_hasRestoreRect = true;

        SDL_Rect bounds;
        int display = SDL_GetWindowDisplayIndex(m_window);
        if (SDL_GetDisplayUsableBounds(display, &bounds) == 0) {
            SDL_SetWindowPosition(m_window, bounds.x, bounds.y);
            SDL_SetWindowSize(m_window, bounds.w, bounds.h);
        }
        m_maximized = true;
    }
}

// Restore window to the rect captured at maximize time.
void Application::RestoreFromMaximize() {
    if (!m_window) return;
    // ISSUE 2: nothing to restore if we never captured a pre-maximize rect
    // (e.g. the OS maximized the window and we only reacted to the event).
    if (!m_hasRestoreRect) {
        SDL_RestoreWindow(m_window);
        m_maximized = false;
        return;
    }
    SDL_SetWindowPosition(m_window, m_restoreX, m_restoreY);
    SDL_SetWindowSize(m_window, m_restoreW, m_restoreH);
    m_maximized = false;
    m_hasRestoreRect = false;
}

int Application::GetResizeEdge(int mx, int my) const {
    int edge = 0;
    const int b = m_resizeBorderWidth;
    if (mx < b)                      edge |= 1; // Left
    if (mx >= m_width - b)           edge |= 2; // Right
    if (my < b)                      edge |= 4; // Top
    if (my >= m_height - b)          edge |= 8; // Bottom
    return edge;
}

void Application::UpdateResizeCursor(int mx, int my) {
    int edge = GetResizeEdge(mx, my);
    SDL_Cursor* cur = m_defaultCursor;
    if (edge == (1|4) || edge == (2|8))      cur = m_resizeCursors[0]; // nwse
    else if (edge == (2|4) || edge == (1|8)) cur = m_resizeCursors[1]; // nesw
    else if (edge == 1 || edge == 2)         cur = m_resizeCursors[2]; // horiz
    else if (edge == 4 || edge == 8)         cur = m_resizeCursors[3]; // vert
    if (cur) SDL_SetCursor(cur);
}

// ISSUE 1: allow the host application to force a content scale (e.g. read from
// its own settings). Clamped to >= 1 so a bogus value never shrinks hit zones.
void Application::SetDpiScale(float scale) {
    m_dpiScale = (scale > 1.0f) ? scale : 1.0f;
}

// ISSUE 1: scale a logical-pixel size by the current content scale and round.
int Application::ScalePx(int logicalPx) const {
    return static_cast<int>(logicalPx * m_dpiScale + 0.5f);
}

void Application::Shutdown() {
    m_root.reset();
    m_input.reset();
    m_fontManager.reset();
    m_canvas.reset();

    // Free resize cursors
    for (auto& c : m_resizeCursors) {
        if (c) { SDL_FreeCursor(c); c = nullptr; }
    }
    if (m_defaultCursor) { SDL_FreeCursor(m_defaultCursor); m_defaultCursor = nullptr; }

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
    m_running = false;
}

} // namespace nui
