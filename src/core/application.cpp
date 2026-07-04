#include "core/log.h"
// NUI: Application implementation
// SDL2 window creation, event loop, software rendering pipeline.

#include "core/application.h"
#include "core/input.h"
#include "core/async.h"
#include "renderer/canvas.h"
#include "renderer/font.h"
#include "ui/widget.h"

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
    m_root = std::move(root);
    if (m_root) {
        m_root->SetRect(0, 0, m_width, m_height);
    }
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

        // Update and render UI tree
        if (m_root) {
            m_root->Update(dt);
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

    // Deliver events to widget tree
    if (m_root) {
        m_root->HandleInput(*m_input);
    }
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

    if (SDL_MUSTLOCK(m_screen)) {
        SDL_UnlockSurface(m_screen);
    }
}

void Application::DispatchOnMainThread(std::function<void()> cb) {
    Async::DispatchOnMainThread(std::move(cb));
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
