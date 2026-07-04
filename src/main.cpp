#include "core/log.h"
// NUI: Demo application
// Demonstrates the CPU-only UI toolkit with a game launcher-style layout.

#include "core/application.h"
#include "core/input.h"
#include "renderer/canvas.h"
#include "renderer/texture.h"
#include "renderer/font.h"
#include "ui/widget.h"
#include "ui/label.h"
#include "ui/button.h"
#include "ui/image.h"
#include "ui/editbox.h"
#include "ui/progressbar.h"
#include "ui/scrollview.h"
#include "xml/layout_loader.h"
#include "renderer/resource.h"

#include <cstdio>
#include <cstring>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

// ── Build UI programmatically (fallback if no XML layout) ───────
std::unique_ptr<nui::Widget> BuildDemoUI(nui::FontManager& fonts,
                                            nui::TextureCache& textures) {
    using namespace nui;

    auto root = std::make_unique<Widget>();
    root->SetName("root");
    root->SetBgColor(Color(20, 20, 30, 255));

    // ── Header ──────────────────────────────────────────────────
    auto header = std::make_unique<Widget>();
    header->SetName("header");
    header->SetRect(0, 0, 1024, 60);
    header->SetBgColor(Color(35, 35, 50, 255));

    auto titleLabel = std::make_unique<Label>();
    titleLabel->SetName("title");
    titleLabel->SetRect(20, 10, 400, 40);
    titleLabel->SetText("NUI Launcher Demo");
    titleLabel->SetFontSize(24);
    titleLabel->SetTextColor(Color(200, 200, 255, 255));
    header->AddChild(std::move(titleLabel));

    // Version label
    auto versionLabel = std::make_unique<Label>();
    versionLabel->SetName("version");
    versionLabel->SetRect(800, 20, 200, 30);
    versionLabel->SetText("v0.1.0");
    versionLabel->SetFontSize(14);
    versionLabel->SetTextColor(Color(120, 120, 140, 255));
    versionLabel->SetAlignH(AlignH::Right);
    header->AddChild(std::move(versionLabel));

    root->AddChild(std::move(header));

    // ── Sidebar ─────────────────────────────────────────────────
    auto sidebar = std::make_unique<Widget>();
    sidebar->SetName("sidebar");
    sidebar->SetRect(0, 60, 250, 708);
    sidebar->SetBgColor(Color(28, 28, 42, 255));

    // Play button
    auto playBtn = std::make_unique<Button>();
    playBtn->SetName("btn_play");
    playBtn->SetRect(20, 20, 210, 50);
    playBtn->SetText("PLAY");
    playBtn->SetFontSize(20);
    playBtn->SetBgColor(Color(40, 120, 40, 255));
    playBtn->SetHoverColor(Color(50, 150, 50, 255));
    playBtn->SetPressedColor(Color(30, 100, 30, 255));
    playBtn->SetOnClick([](Widget* w) {
        NUI_LOG("[Launcher] Play button clicked!\n");
    });
    sidebar->AddChild(std::move(playBtn));

    // Settings button
    auto settingsBtn = std::make_unique<Button>();
    settingsBtn->SetName("btn_settings");
    settingsBtn->SetRect(20, 80, 210, 40);
    settingsBtn->SetText("Settings");
    settingsBtn->SetFontSize(16);
    settingsBtn->SetOnClick([](Widget* w) {
        NUI_LOG("[Launcher] Settings button clicked!\n");
    });
    sidebar->AddChild(std::move(settingsBtn));

    // Exit button
    auto exitBtn = std::make_unique<Button>();
    exitBtn->SetName("btn_exit");
    exitBtn->SetRect(20, 130, 210, 40);
    exitBtn->SetText("Exit");
    exitBtn->SetFontSize(16);
    exitBtn->SetBgColor(Color(120, 40, 40, 255));
    exitBtn->SetHoverColor(Color(150, 50, 50, 255));
    exitBtn->SetPressedColor(Color(100, 30, 30, 255));
    exitBtn->SetOnClick([](Widget* w) {
        nui::GetApp()->Quit();
    });
    sidebar->AddChild(std::move(exitBtn));

    // Progress bar (e.g., download progress)
    auto progressLabel = std::make_unique<Label>();
    progressLabel->SetRect(20, 200, 210, 20);
    progressLabel->SetText("Download Progress:");
    progressLabel->SetFontSize(12);
    progressLabel->SetTextColor(Color(150, 150, 170, 255));
    sidebar->AddChild(std::move(progressLabel));

    auto progressBar = std::make_unique<ProgressBar>();
    progressBar->SetName("progress");
    progressBar->SetRect(20, 225, 210, 25);
    progressBar->SetValue(0.65f);
    progressBar->SetShowPercent(true);
    progressBar->SetFillColor(Color(40, 120, 200, 255));
    progressBar->SetBorderColor(Color(80, 80, 100, 255));
    sidebar->AddChild(std::move(progressBar));

    // Search box
    auto searchBox = std::make_unique<EditBox>();
    searchBox->SetName("search");
    searchBox->SetRect(20, 280, 210, 32);
    searchBox->SetPlaceholder("Search...");
    searchBox->SetFontSize(14);
    sidebar->AddChild(std::move(searchBox));

    root->AddChild(std::move(sidebar));

    // ── Content area ────────────────────────────────────────────
    auto content = std::make_unique<Widget>();
    content->SetName("content");
    content->SetRect(250, 60, 774, 708);
    content->SetBgColor(Color(24, 24, 34, 255));

    // Game cover image
    auto coverImg = std::make_unique<Image>();
    coverImg->SetName("cover");
    coverImg->SetRect(20, 20, 400, 250);
    coverImg->SetScaleMode(ScaleMode::Fit);
    // Try to load an image (will show placeholder if not found)
    coverImg->LoadFromFile("resources/images/cover.png", textures);
    content->AddChild(std::move(coverImg));

    // Game info
    auto gameTitle = std::make_unique<Label>();
    gameTitle->SetRect(440, 20, 310, 35);
    gameTitle->SetText("Game Title");
    gameTitle->SetFontSize(28);
    gameTitle->SetTextColor(Color(220, 220, 255, 255));
    content->AddChild(std::move(gameTitle));

    auto gameDesc = std::make_unique<Label>();
    gameDesc->SetRect(440, 60, 310, 200);
    gameDesc->SetText("This is a demo of the NUI, "
                      "a lightweight CPU-only cross-platform UI toolkit "
                      "designed for game launchers. All rendering is done "
                      "on the CPU using SDL2 software rendering - no GPU "
                      "or graphics drivers required!");
    gameDesc->SetFontSize(14);
    gameDesc->SetTextColor(Color(180, 180, 190, 255));
    gameDesc->SetWordWrap(true);
    content->AddChild(std::move(gameDesc));

    // News section with scroll view
    auto newsLabel = std::make_unique<Label>();
    newsLabel->SetRect(20, 290, 200, 25);
    newsLabel->SetText("Recent News:");
    newsLabel->SetFontSize(18);
    newsLabel->SetTextColor(Color(200, 200, 220, 255));
    content->AddChild(std::move(newsLabel));

    auto scrollView = std::make_unique<ScrollView>();
    scrollView->SetName("news_scroll");
    scrollView->SetRect(20, 320, 734, 360);
    scrollView->SetBorderColor(Color(60, 60, 80, 255));

    // Add news items to scroll view
    const char* newsItems[] = {
        "Update 1.2.3 - New features added to the launcher",
        "Server maintenance scheduled for next week",
        "New game mode coming soon!",
        "Community event this weekend - join us!",
        "Bug fixes and performance improvements",
        "New map released - check it out!",
    };
    for (int i = 0; i < 6; ++i) {
        auto item = std::make_unique<Widget>();
        item->SetRect(10, 10 + i * 55, 710, 50);
        item->SetBgColor(Color(35, 35, 50, 255));
        item->SetBorderColor(Color(50, 50, 65, 255));

        auto itemLabel = std::make_unique<Label>();
        itemLabel->SetRect(10, 5, 690, 20);
        itemLabel->SetText(newsItems[i]);
        itemLabel->SetFontSize(14);
        itemLabel->SetTextColor(Color(200, 200, 220, 255));
        item->AddChild(std::move(itemLabel));

        auto dateLabel = std::make_unique<Label>();
        dateLabel->SetRect(10, 28, 690, 15);
        dateLabel->SetText("2026-07-03");
        dateLabel->SetFontSize(11);
        dateLabel->SetTextColor(Color(100, 100, 120, 255));
        item->AddChild(std::move(dateLabel));

        scrollView->AddChild(std::move(item));
    }

    content->AddChild(std::move(scrollView));
    root->AddChild(std::move(content));

    return root;
}

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
        char* slash = strrchr(path, '/');
        if (slash) { *slash = '\0'; chdir(path); }
    }
#elif defined(__APPLE__)
    char path[4096];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        char* slash = strrchr(path, '/');
        if (slash) { *slash = '\0'; chdir(path); }
    }
#endif
}

// ── Main ────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    using namespace nui;

    SetWorkDirToExe();
    ResourceManager::Initialize();

    Application app;
    AppDesc desc;
    desc.title = "NUI Launcher";
    desc.width = 1024;
    desc.height = 768;
    desc.resizable = true;

    if (!app.Initialize(desc)) {
        NUI_LOG_ERROR( "Failed to initialize application\n");
        return 1;
    }

    // Initialize texture cache
    TextureCache textures;

    // Try to load XML layout first
    LayoutLoader loader;
    // Uncomment to load color definitions:
    // loader.LoadColorDefs("resources/layouts/colors.xml");

    std::unique_ptr<Widget> root;
    root = loader.LoadFromFile("resources/layouts/main.xml",
                                textures, *app.GetFontManager());
    if (!root) {
        // Fallback: build UI programmatically
        NUI_LOG("[NUI] No XML layout found, using programmatic demo\n");
        root = BuildDemoUI(*app.GetFontManager(), textures);
    }

    if (root) {
        root->SetRect(0, 0, desc.width, desc.height);
        app.SetRoot(std::move(root));
    }

    // Set up a tick callback (for animations, progress, etc.)
    float progressValue = 0.0f;
    app.SetOnTick([&](float dt) {
        // Animate progress bar
        progressValue += dt * 0.1f;
        if (progressValue > 1.0f) progressValue = 0.0f;

        Widget* progressBar = app.GetRoot()->GetChild("progress");
        if (progressBar) {
            // Cast to ProgressBar and update (simplified for demo)
            // In production, use proper type system or visitor pattern
        }
    });

    NUI_LOG("[NUI] Starting main loop (CPU software rendering)...\n");
    int exitCode = app.Run();
    NUI_LOG("[NUI] Application exited with code %d\n", exitCode);

    return exitCode;
}
