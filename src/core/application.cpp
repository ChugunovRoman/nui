#include "core/log.h"
// NUI: Application implementation
// SDL2 window creation, event loop, software rendering pipeline.

#include "core/application.h"
#include "core/input.h"
#include "core/async.h"
#include "animation/animator.h"
#include "renderer/canvas.h"
#include "renderer/font.h"
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

    // Create window - no GPU flags, pure software
    Uint32 flags = SDL_WINDOW_SHOWN;
    if (desc.resizable)
        flags |= SDL_WINDOW_RESIZABLE;

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

    // Get the window surface (CPU-only, no renderer)
    m_screen = SDL_GetWindowSurface(m_window);
    if (!m_screen) {
        NUI_LOG_ERROR( "[NUI] SDL_GetWindowSurface failed: %s\n", SDL_GetError());
        return false;
    }

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
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    m_width  = event.window.data1;
                    m_height = event.window.data2;
                    m_screen = SDL_GetWindowSurface(m_window);
                    if (m_screen) {
                        m_canvas->SetSurface(m_screen);
                    }
                    if (m_root) {
                        m_root->SetRect(0, 0, m_width, m_height);
                    }
                }
                break;

            case SDL_MOUSEMOTION:
                m_input->SetMousePos(event.motion.x, event.motion.y);
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                bool down = (event.type == SDL_MOUSEBUTTONDOWN);
                MouseButton btn = MouseButton::Left;
                if (event.button.button == SDL_BUTTON_MIDDLE) btn = MouseButton::Middle;
                if (event.button.button == SDL_BUTTON_RIGHT)  btn = MouseButton::Right;
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
            m_root->HandleInput(*m_input);
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

void Application::Shutdown() {
    m_root.reset();
    m_input.reset();
    m_fontManager.reset();
    m_canvas.reset();

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
    m_running = false;
}

} // namespace nui
