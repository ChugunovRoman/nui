// NUI: Frameless Window Demo
// Demonstrates custom window management features (v0.5.0):
//   1. Borderless window (no system frame)
//   2. Window icon (set via AppDesc::iconPath)
//   3. Custom TitlebarWidget with drag-to-move
//   4. Minimize / Maximize-Restore / Close buttons
//   5. Double-click titlebar to maximize/restore
//   6. Resize borders (drag edges/corners to resize)
//   7. Draggable background (any widget can be a drag zone)

#include "core/application.h"
#include "core/input.h"
#include "core/log.h"
#include "renderer/resource.h"
#include "renderer/canvas.h"
#include "renderer/font.h"
#include "renderer/texture.h"
#include "ui/widget.h"
#include "ui/label.h"
#include "ui/button.h"
#include "ui/titlebar.h"
#include "ui/image.h"
#include "xml/layout_loader.h"

#include <cstdio>
#include <cstring>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

// ── Change working directory to exe location ────────────────────
static void SetWorkDirToExe() {
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    for (int i = static_cast<int>(strlen(path)) - 1; i >= 0; --i) {
        if (path[i] == '\\' || path[i] == '/') { path[i] = '\0'; break; }
    }
    SetCurrentDirectoryA(path);
#elif defined(__linux__)
    char path[4096];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len > 0) {
        path[len] = '\0';
        for (int i = static_cast<int>(len) - 1; i >= 0; --i) {
            if (path[i] == '/') { path[i] = '\0'; break; }
        }
        chdir(path);
    }
#elif defined(__APPLE__)
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        for (int i = static_cast<int>(strlen(path)) - 1; i >= 0; --i) {
            if (path[i] == '/') { path[i] = '\0'; break; }
        }
        chdir(path);
    }
#endif
}

// ── Main ────────────────────────────────────────────────────────
int main(int /*argc*/, char* /*argv*/[]) {
    SetWorkDirToExe();

    // Initialize resource manager for embedded resources
    nui::ResourceManager::Initialize();

    // ── Application configuration ─────────────────────────────
    nui::AppDesc desc;
    desc.title      = "NUI Frameless Demo";
    desc.iconPath   = "resources/icon.png";   // window + taskbar icon
    desc.width      = 1280;
    desc.height     = 720;
    desc.resizable  = true;
    desc.borderless = true;                    // no system frame

    // Borderless geometry is fully configurable (and DPI-scaled automatically):
    desc.resizeBorderWidth = 5;    // px hit zone for edge/corner resize
    desc.minWindowWidth    = 320;  // minimum window size on resize
    desc.minWindowHeight   = 240;
    desc.titlebarButtonSize = 32;  // min/max/close button size
    desc.doubleClickMs     = 450;  // double-click window for titlebar maximize
    desc.dragThreshold     = 4;    // px the mouse must travel to start a drag

    nui::Application app;
    if (!app.Initialize(desc)) {
        NUI_LOG_ERROR("[Frameless] Failed to initialize application\n");
        return 1;
    }

    // ── Build UI tree ──────────────────────────────────────────
    nui::TextureCache textures;
    nui::FontManager& fonts = *app.GetFontManager();

    auto root = std::make_unique<nui::Widget>();

    // ── Titlebar ───────────────────────────────────────────────
    auto titlebar = std::make_unique<nui::TitlebarWidget>();
    titlebar->SetTitle("NUI Frameless Demo");
    titlebar->SetRect(0, 0, desc.width, 36);
    titlebar->SetAnchor(nui::AnchorFlag::Left | nui::AnchorFlag::Top | nui::AnchorFlag::Right);
    titlebar->ShowMinimize(true);
    titlebar->ShowMaximize(true);
    titlebar->ShowClose(true);

    // Wire up window control callbacks
    titlebar->SetOnClose([]() {
        nui::GetApp()->Quit();
    });
    titlebar->SetOnMinimize([]() {
        SDL_MinimizeWindow(nui::GetApp()->GetSDLWindow());
    });
    titlebar->SetOnMaximize([]() {
        nui::GetApp()->ToggleMaximize();
    });

    root->AddChild(std::move(titlebar));

    // ── Content area (draggable background) ────────────────────
    auto content = std::make_unique<nui::Widget>();
    content->SetRect(0, 36, desc.width, desc.height - 36);
    content->SetAnchor(nui::AnchorFlag::Left | nui::AnchorFlag::Top |
                       nui::AnchorFlag::Right | nui::AnchorFlag::Bottom);
    content->SetBgColor(nui::Color(25, 25, 35, 255));
    content->SetWindowDragEnabled(true);  // drag the window by clicking the background

    // ── Info label ─────────────────────────────────────────────
    auto infoLabel = std::make_unique<nui::Label>();
    infoLabel->SetText("Drag anywhere on this background to move the window.\n"
                       "Drag window edges/corners to resize.\n"
                       "Double-click titlebar to maximize/restore.");
    infoLabel->SetRect(40, 40, 500, 80);
    infoLabel->SetFontSize(16);
    infoLabel->SetTextColor(nui::Color(200, 200, 220, 255));
    content->AddChild(std::move(infoLabel));

    // ── Close button (demo) ────────────────────────────────────
    auto btnClose = std::make_unique<nui::Button>();
    btnClose->SetText("Close");
    btnClose->SetRect(40, 140, 120, 36);
    btnClose->SetOnClick([](nui::Widget*) {
        nui::GetApp()->Quit();
    });
    content->AddChild(std::move(btnClose));

    // ── Toggle maximize button (demo) ──────────────────────────
    auto btnMax = std::make_unique<nui::Button>();
    btnMax->SetText("Toggle Maximize");
    btnMax->SetRect(180, 140, 180, 36);
    btnMax->SetOnClick([](nui::Widget*) {
        nui::GetApp()->ToggleMaximize();
    });
    content->AddChild(std::move(btnMax));

    root->AddChild(std::move(content));

    // ── Run ────────────────────────────────────────────────────
    app.SetRoot(std::move(root));
    return app.Run();
}
