// NUI: Hello World example
// Demonstrates every supported widget type:
//   Widget, Label, Button, Image, EditBox, ProgressBar, ScrollView,
//   Slider, CheckBox, RadioButton, Dropdown,
//   TabControl, Treeview, Tooltip, Menu/ContextMenu, Dialog/MessageBox
//
// Two modes:
//   1. XML layout  — loads demo.xml if present
//   2. Programmatic — builds the same UI in code as a fallback

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
#include "ui/slider.h"
#include "ui/checkbox.h"
#include "ui/radiobutton.h"
#include "ui/dropdown.h"
#include "ui/tabcontrol.h"
#include "ui/treeview.h"
#include "ui/tooltip.h"
#include "ui/menu.h"
#include "ui/dialog.h"
#include "xml/layout_loader.h"
#include "renderer/resource.h"
#include "core/async.h"

#include <cstdio>
#include <cmath>
#include <cstring>
#include <thread>
#include <chrono>
#include "core/log.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

// ── Build every widget programmatically ─────────────────────────
std::unique_ptr<nui::Widget> BuildHelloWorldUI(nui::FontManager& fonts,
                                                  nui::TextureCache& textures) {
    using namespace nui;

    // ── Root panel ──────────────────────────────────────────────
    auto root = std::make_unique<Widget>();
    root->SetName("root");
    root->SetBgColor(Color(18, 18, 28, 255));

    // ── 1. Title label ──────────────────────────────────────────
    auto title = std::make_unique<Label>();
    title->SetName("title");
    title->SetRect(0, 10, 1200, 40);
    title->SetAlignH(AlignH::Center);
    title->SetAlignV(AlignV::Center);
    title->SetText("NUI  ·  Hello World");
    title->SetFontSize(28);
    title->SetTextColor(Color(200, 210, 255, 255));
    root->AddChild(std::move(title));

    // ── 2. Subtitle label ───────────────────────────────────────
    auto subtitle = std::make_unique<Label>();
    subtitle->SetRect(0, 50, 1200, 24);
    subtitle->SetAlignH(AlignH::Center);
    subtitle->SetText("Every supported widget shown below — all rendered on CPU");
    subtitle->SetFontSize(13);
    subtitle->SetTextColor(Color(120, 130, 160, 255));
    root->AddChild(std::move(subtitle));

    // ── 3. Separator line (thin widget used as a divider) ───────
    auto separator = std::make_unique<Widget>();
    separator->SetRect(40, 82, 1120, 1);
    separator->SetBgColor(Color(60, 60, 80, 255));
    root->AddChild(std::move(separator));

    // ── Left column ─────────────────────────────────────────────

    // -- Button examples --
    auto btnHeader = std::make_unique<Label>();
    btnHeader->SetRect(40, 100, 340, 22);
    btnHeader->SetText("Buttons");
    btnHeader->SetFontSize(16);
    btnHeader->SetTextColor(Color(170, 180, 210, 255));
    root->AddChild(std::move(btnHeader));

    // Normal button
    auto btnNormal = std::make_unique<Button>();
    btnNormal->SetName("btn_hello");
    btnNormal->SetRect(40, 130, 160, 40);
    btnNormal->SetText("Say Hello");
    btnNormal->SetFontSize(15);
    btnNormal->SetBgColor(Color(40, 90, 160, 255));
    btnNormal->SetHoverColor(Color(55, 110, 190, 255));
    btnNormal->SetPressedColor(Color(30, 70, 130, 255));
    btnNormal->SetOnClick([](Widget*) {
        NUI_LOG("[Example] Hello from NUI!\n");
    });
    root->AddChild(std::move(btnNormal));

    // Green action button
    auto btnPlay = std::make_unique<Button>();
    btnPlay->SetName("btn_play");
    btnPlay->SetRect(210, 130, 160, 40);
    btnPlay->SetText("▶  Play");
    btnPlay->SetFontSize(15);
    btnPlay->SetBgColor(Color(35, 130, 50, 255));
    btnPlay->SetHoverColor(Color(45, 155, 60, 255));
    btnPlay->SetPressedColor(Color(25, 100, 35, 255));
    btnPlay->SetOnClick([](Widget*) {
        NUI_LOG("[Example] Play clicked!\n");
    });
    root->AddChild(std::move(btnPlay));

    // Async demo button
    auto btnAsync = std::make_unique<Button>();
    btnAsync->SetName("btn_async");
    btnAsync->SetRect(40, 240, 160, 40);
    btnAsync->SetText("Async Task");
    btnAsync->SetFontSize(15);
    btnAsync->SetBgColor(Color(120, 60, 160, 255));
    btnAsync->SetHoverColor(Color(145, 80, 190, 255));
    btnAsync->SetPressedColor(Color(100, 45, 140, 255));
    btnAsync->SetOnClick([](Widget*) {
        NUI_LOG("[Example] Async task started...\n");

        // Find status label and update it
        Widget* root = GetApp()->GetRoot();
        Label* status = static_cast<Label*>(root->GetChild("async_status"));
        if (status) status->SetText("Working...");

        // Run heavy task on background thread
        Async::Run([]() -> std::string {
            // Simulate 5-second heavy work (does NOT block UI)
            std::this_thread::sleep_for(std::chrono::seconds(5));
            return "Async done! Result: 42";
        })->Then([status](const std::string& result) {
            // This runs on main thread — safe to update UI
            NUI_LOG("[Example] %s\n", result.c_str());
            if (status) status->SetText(result);
        });
    });
    root->AddChild(std::move(btnAsync));

    // Status label for async demo
    auto asyncStatus = std::make_unique<Label>();
    asyncStatus->SetName("async_status");
    asyncStatus->SetRect(40, 290, 330, 20);
    asyncStatus->SetText("Click \"Async Task\" to test (5s delay)");
    asyncStatus->SetFontSize(13);
    asyncStatus->SetTextColor(Color(180, 140, 220, 255));
    root->AddChild(std::move(asyncStatus));

    // Red danger button
    auto btnQuit = std::make_unique<Button>();
    btnQuit->SetName("btn_quit");
    btnQuit->SetRect(40, 180, 160, 40);
    btnQuit->SetText("Quit");
    btnQuit->SetFontSize(15);
    btnQuit->SetBgColor(Color(150, 35, 35, 255));
    btnQuit->SetHoverColor(Color(180, 50, 50, 255));
    btnQuit->SetPressedColor(Color(120, 25, 25, 255));
    btnQuit->SetOnClick([](Widget*) {
        nui::GetApp()->Quit();
    });
    root->AddChild(std::move(btnQuit));

    // Disabled button
    auto btnDisabled = std::make_unique<Button>();
    btnDisabled->SetRect(210, 180, 160, 40);
    btnDisabled->SetText("Disabled");
    btnDisabled->SetFontSize(15);
    btnDisabled->SetEnabled(false);
    root->AddChild(std::move(btnDisabled));

    // -- EditBox examples --
    auto editHeader = std::make_unique<Label>();
    editHeader->SetRect(40, 330, 340, 22);
    editHeader->SetText("Text Input");
    editHeader->SetFontSize(16);
    editHeader->SetTextColor(Color(170, 180, 210, 255));
    root->AddChild(std::move(editHeader));

    auto editName = std::make_unique<EditBox>();
    editName->SetName("edit_name");
    editName->SetRect(40, 360, 330, 32);
    editName->SetPlaceholder("Enter your name...");
    editName->SetFontSize(14);
    editName->SetOnTextChanged([](EditBox* eb, const std::string& text) {
        NUI_LOG("[EditBox] Name changed: %s\n", text.c_str());
    });
    root->AddChild(std::move(editName));

    auto editPassword = std::make_unique<EditBox>();
    editPassword->SetName("edit_password");
    editPassword->SetRect(40, 400, 330, 32);
    editPassword->SetPlaceholder("Password...");
    editPassword->SetFontSize(14);
    editPassword->SetPasswordMode(true);
    root->AddChild(std::move(editPassword));

    // -- ProgressBar examples --
    auto progressHeader = std::make_unique<Label>();
    progressHeader->SetRect(40, 450, 340, 22);
    progressHeader->SetText("Progress Bars");
    progressHeader->SetFontSize(16);
    progressHeader->SetTextColor(Color(170, 180, 210, 255));
    root->AddChild(std::move(progressHeader));

    auto progressBlue = std::make_unique<ProgressBar>();
    progressBlue->SetName("progress_blue");
    progressBlue->SetRect(40, 480, 330, 24);
    progressBlue->SetValue(0.45f);
    progressBlue->SetShowPercent(true);
    progressBlue->SetFillColor(Color(40, 120, 200, 255));
    progressBlue->SetBorderColor(Color(60, 60, 80, 255));
    root->AddChild(std::move(progressBlue));

    auto progressGreen = std::make_unique<ProgressBar>();
    progressGreen->SetName("progress_green");
    progressGreen->SetRect(40, 510, 330, 24);
    progressGreen->SetValue(0.78f);
    progressGreen->SetLabel("78 / 100 items");
    progressGreen->SetFillColor(Color(40, 160, 60, 255));
    progressGreen->SetBorderColor(Color(60, 60, 80, 255));
    root->AddChild(std::move(progressGreen));

    auto progressYellow = std::make_unique<ProgressBar>();
    progressYellow->SetName("progress_yellow");
    progressYellow->SetRect(40, 540, 330, 24);
    progressYellow->SetValue(0.15f);
    progressYellow->SetFillColor(Color(200, 170, 30, 255));
    progressYellow->SetBorderColor(Color(60, 60, 80, 255));
    root->AddChild(std::move(progressYellow));

    // -- Slider example --
    auto sliderHeader = std::make_unique<Label>();
    sliderHeader->SetRect(40, 580, 340, 22);
    sliderHeader->SetText("Slider");
    sliderHeader->SetFontSize(16);
    sliderHeader->SetTextColor(Color(170, 180, 210, 255));
    root->AddChild(std::move(sliderHeader));

    auto slider = std::make_unique<Slider>();
    slider->SetName("slider_volume");
    slider->SetRect(40, 610, 200, 24);
    slider->SetValue(0.6f);
    slider->SetFillColor(Color(60, 160, 80, 255));
    slider->SetThumbColor(Color(220, 230, 240, 255));

    auto sliderValue = std::make_unique<Label>();
    sliderValue->SetName("slider_value");
    sliderValue->SetRect(250, 610, 60, 24);
    sliderValue->SetText("60%");
    sliderValue->SetFontSize(14);
    sliderValue->SetTextColor(Color(180, 180, 200, 255));
    sliderValue->SetAlignH(AlignH::Center);
    Label* sliderValPtr = sliderValue.get();

    slider->SetOnValueChanged([sliderValPtr](Widget*, float val) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", static_cast<int>(val * 100));
        sliderValPtr->SetText(buf);
    });
    root->AddChild(std::move(slider));
    root->AddChild(std::move(sliderValue));

    // -- CheckBox examples --
    auto checkHeader = std::make_unique<Label>();
    checkHeader->SetRect(40, 650, 340, 22);
    checkHeader->SetText("Checkboxes");
    checkHeader->SetFontSize(16);
    checkHeader->SetTextColor(Color(170, 180, 210, 255));
    root->AddChild(std::move(checkHeader));

    auto cb1 = std::make_unique<CheckBox>();
    cb1->SetRect(40, 680, 160, 24);
    cb1->SetText("Enable sound");
    cb1->SetChecked(true);
    cb1->SetFontSize(13);
    cb1->SetOnCheckedChanged([](Widget*, bool checked) {
        NUI_LOG("[Example] Sound: %s\n", checked ? "ON" : "OFF");
    });
    root->AddChild(std::move(cb1));

    auto cb2 = std::make_unique<CheckBox>();
    cb2->SetRect(210, 680, 160, 24);
    cb2->SetText("Fullscreen");
    cb2->SetFontSize(13);
    cb2->SetOnCheckedChanged([](Widget*, bool checked) {
        NUI_LOG("[Example] Fullscreen: %s\n", checked ? "ON" : "OFF");
    });
    root->AddChild(std::move(cb2));

    // -- RadioButton examples --
    auto radioHeader = std::make_unique<Label>();
    radioHeader->SetRect(40, 715, 340, 22);
    radioHeader->SetText("Radio Buttons");
    radioHeader->SetFontSize(16);
    radioHeader->SetTextColor(Color(170, 180, 210, 255));
    root->AddChild(std::move(radioHeader));

    auto rb1 = std::make_unique<RadioButton>();
    rb1->SetRect(40, 745, 100, 22);
    rb1->SetText("Low");
    rb1->SetGroup("quality");
    rb1->SetFontSize(13);
    rb1->SetOnSelectedChanged([](Widget*) { NUI_LOG("[Example] Quality: Low\n"); });
    root->AddChild(std::move(rb1));

    auto rb2 = std::make_unique<RadioButton>();
    rb2->SetRect(150, 745, 100, 22);
    rb2->SetText("Medium");
    rb2->SetGroup("quality");
    rb2->SetSelected(true);
    rb2->SetFontSize(13);
    rb2->SetOnSelectedChanged([](Widget*) { NUI_LOG("[Example] Quality: Medium\n"); });
    root->AddChild(std::move(rb2));

    auto rb3 = std::make_unique<RadioButton>();
    rb3->SetRect(260, 745, 100, 22);
    rb3->SetText("High");
    rb3->SetGroup("quality");
    rb3->SetFontSize(13);
    rb3->SetOnSelectedChanged([](Widget*) { NUI_LOG("[Example] Quality: High\n"); });
    root->AddChild(std::move(rb3));

    // ── Right column ────────────────────────────────────────────

    // -- Image widget --
    auto imgHeader = std::make_unique<Label>();
    imgHeader->SetRect(420, 100, 340, 22);
    imgHeader->SetText("Image (Fit mode)");
    imgHeader->SetFontSize(16);
    imgHeader->SetTextColor(Color(170, 180, 210, 255));
    root->AddChild(std::move(imgHeader));

    auto image = std::make_unique<Image>();
    image->SetName("demo_image");
    image->SetRect(420, 130, 340, 200);
    image->SetScaleMode(ScaleMode::Fit);
    image->SetBorderColor(Color(60, 60, 80, 255));
    // Will show placeholder if the file doesn't exist
    image->LoadFromFile("resources/images/cover.png", textures);
    root->AddChild(std::move(image));

    // -- Label with word wrap --
    auto wrapHeader = std::make_unique<Label>();
    wrapHeader->SetRect(420, 345, 340, 22);
    wrapHeader->SetText("Word-Wrapped Text");
    wrapHeader->SetFontSize(16);
    wrapHeader->SetTextColor(Color(170, 180, 210, 255));
    root->AddChild(std::move(wrapHeader));

    auto wrapLabel = std::make_unique<Label>();
    wrapLabel->SetRect(420, 375, 340, 100);
    wrapLabel->SetText(
        "This label demonstrates automatic word wrapping. "
        "Long text is split across multiple lines to fit "
        "inside the widget bounds."
    );
    wrapLabel->SetFontSize(13);
    wrapLabel->SetTextColor(Color(180, 180, 200, 255));
    wrapLabel->SetWordWrap(true);
    wrapLabel->SetBgColor(Color(30, 30, 42, 255));
    wrapLabel->SetBorderColor(Color(50, 50, 65, 255));
    root->AddChild(std::move(wrapLabel));

    // -- ScrollView with news items --
    auto scrollHeader = std::make_unique<Label>();
    scrollHeader->SetRect(420, 490, 340, 22);
    scrollHeader->SetText("Scroll View");
    scrollHeader->SetFontSize(16);
    scrollHeader->SetTextColor(Color(170, 180, 210, 255));
    root->AddChild(std::move(scrollHeader));

    auto scrollView = std::make_unique<ScrollView>();
    scrollView->SetName("scroll_news");
    scrollView->SetRect(420, 518, 340, 220);
    scrollView->SetBorderColor(Color(60, 60, 80, 255));

    const char* items[] = {
        "Welcome to NUI!",
        "CPU-only rendering — no GPU needed",
        "SDL2 software renderer under the hood",
        "XML layouts like xrUICore",
        "Image support: PNG, JPG, BMP, TGA, GIF",
        "FreeType font rendering with anti-aliasing",
        "Single self-contained binary",
        "Cross-platform: Windows, Linux, macOS",
        "Perfect for game launchers",
    };
    for (int i = 0; i < 9; ++i) {
        auto item = std::make_unique<Widget>();
        item->SetRect(8, 8 + i * 42, 320, 36);
        item->SetBgColor(Color(32, 32, 46, 255));
        item->SetBorderColor(Color(48, 48, 62, 255));

        auto itemLabel = std::make_unique<Label>();
        itemLabel->SetRect(8, 8, 300, 20);
        itemLabel->SetText(items[i]);
        itemLabel->SetFontSize(12);
        itemLabel->SetTextColor(Color(190, 195, 215, 255));
        item->AddChild(std::move(itemLabel));

        scrollView->AddChild(std::move(item));
    }
    root->AddChild(std::move(scrollView));

    // -- Dropdown example --
    auto ddHeader = std::make_unique<Label>();
    ddHeader->SetRect(420, 750, 340, 22);
    ddHeader->SetText("Dropdown");
    ddHeader->SetFontSize(16);
    ddHeader->SetTextColor(Color(170, 180, 210, 255));
    root->AddChild(std::move(ddHeader));

    auto dropdown = std::make_unique<Dropdown>();
    dropdown->SetName("dd_server");
    dropdown->SetRect(420, 778, 200, 28);
    dropdown->SetFontSize(13);
    dropdown->AddItem("Server EU-West");
    dropdown->AddItem("Server EU-East");
    dropdown->AddItem("Server US-East");
    dropdown->AddItem("Server US-West");
    dropdown->AddItem("Server Asia");
    dropdown->SetOnItemSelected([](Widget*, int idx, const std::string& text) {
        NUI_LOG("[Example] Selected server: %s (index %d)\n", text.c_str(), idx);
    });
    root->AddChild(std::move(dropdown));

    // ── Footer ──────────────────────────────────────────────────
    auto footer = std::make_unique<Widget>();
    footer->SetRect(0, 820, 800, 28);
    footer->SetBgColor(Color(25, 25, 38, 255));

    auto footerLabel = std::make_unique<Label>();
    footerLabel->SetRect(40, 4, 720, 20);
    footerLabel->SetText("NUI Example  ·  All widgets demo  ·  Click \"Quit\" or close window to exit");
    footerLabel->SetFontSize(11);
    footerLabel->SetTextColor(Color(100, 100, 120, 255));
    footerLabel->SetAlignH(AlignH::Center);
    footer->AddChild(std::move(footerLabel));

    root->AddChild(std::move(footer));

    return root;
}

// ── Change working directory to exe location ────────────────────
// This ensures resources are found regardless of how the exe is launched.
static void SetWorkDirToExe() {
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    // Strip filename, keep directory
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
    desc.title   = "NUI · Hello World";
    desc.width   = 1200;
    desc.height  = 900;
    desc.resizable = true;

    if (!app.Initialize(desc)) {
        NUI_LOG_ERROR( "Failed to initialize application\n");
        return 1;
    }

    TextureCache textures;
    LayoutLoader loader;

    // Try XML layout first, fall back to programmatic
    std::unique_ptr<Widget> root;
    root = loader.LoadFromFile("resources/app/demo.xml",
                                textures, *app.GetFontManager());
    if (!root) {
        NUI_LOG("[Example] No XML layout found, building programmatically\n");
        root = BuildHelloWorldUI(*app.GetFontManager(), textures);
    }

    if (root) {
        root->SetRect(0, 0, desc.width, desc.height);
        app.SetRoot(std::move(root));
    }

    // ── Wire up overlay widgets (Menu / Dialog / ContextMenu / MessageBox) ──
    // These are not declarative in XML (they pop up dynamically), so we find
    // the trigger buttons by name and attach programmatic handlers. Results
    // are reported in the "overlay_result" label.
    {
        nui::Widget* r = app.GetRoot();
        nui::Label* resultLabel = r ? static_cast<nui::Label*>(r->GetChild("overlay_result")) : nullptr;
        int sw = app.GetWidth();
        int sh = app.GetHeight();

        // Helper: a small status string shown next to the overlay buttons.
        auto setResult = [resultLabel](const char* text) {
            if (resultLabel) resultLabel->SetText(text);
        };

        // ── btn_menu: open a popup menu at the button (with a submenu) ──
        if (nui::Widget* btn = r ? r->GetChild("btn_menu") : nullptr) {
            btn->SetOnClick([sw, sh, setResult](nui::Widget* w) {
                auto menu = std::make_unique<nui::Menu>();
                menu->AddItem("New",   [setResult] { setResult("Menu: New"); });
                menu->AddItem("Open",  [setResult] { setResult("Menu: Open"); });
                menu->AddSeparator();
                menu->AddDisabledItem("Save (disabled)");
                // Submenu: build it up front and hand ownership to the parent.
                auto sub = std::make_unique<nui::Menu>();
                sub->AddItem("Desktop", [setResult] { setResult("Sent to Desktop"); });
                sub->AddItem("Archive", [setResult] { setResult("Sent to Archive"); });
                menu->AddSubmenu("Send to", std::move(sub));
                nui::Rect a = w->GetAbsoluteRect();
                nui::Menu::Open(std::move(menu), a.x, a.y + a.h, sw, sh);
            });
        }

        // ── btn_dialog: open a modal dialog with custom content ──
        if (nui::Widget* btn = r ? r->GetChild("btn_dialog") : nullptr) {
            btn->SetOnClick([sw, sh, setResult](nui::Widget*) {
                auto dlg = std::make_unique<nui::Dialog>();
                dlg->SetTitle("Settings");
                dlg->SetMessage("Pick a quality level, then confirm.");
                auto dd = std::make_unique<nui::Dropdown>();
                dd->AddItem("Low");
                dd->AddItem("Medium");
                dd->AddItem("High");
                dd->SetRect(10, 10, 240, 28);
                dlg->AddContent(std::move(dd));
                dlg->SetButtons(nui::DialogButtons::OkCancel);
                dlg->SetOnResult([setResult](nui::DialogResult res) {
                    if (res == nui::DialogResult::Ok)      setResult("Dialog: OK");
                    else if (res == nui::DialogResult::Cancel) setResult("Dialog: Cancel");
                });
                nui::Dialog::Open(std::move(dlg), sw, sh);
            });
        }

        // ── btn_msgbox: a simple Yes/No message box ──
        if (nui::Widget* btn = r ? r->GetChild("btn_msgbox") : nullptr) {
            btn->SetOnClick([sw, sh, setResult](nui::Widget*) {
                nui::Dialog::ShowMessage(
                    "Confirm", "Are you sure you want to continue?",
                    nui::DialogButtons::YesNo,
                    [setResult](nui::DialogResult res) {
                        if (res == nui::DialogResult::Yes) setResult("MessageBox: Yes");
                        else if (res == nui::DialogResult::No) setResult("MessageBox: No");
                        else setResult("MessageBox: dismissed");
                    },
                    sw, sh);
            });
        }

        // ── btn_ctxmenu: open a context menu at the button ──
        if (nui::Widget* btn = r ? r->GetChild("btn_ctxmenu") : nullptr) {
            btn->SetOnClick([sw, sh, setResult](nui::Widget* w) {
                auto menu = std::make_unique<nui::Menu>();
                menu->AddItem("Refresh",    [setResult] { setResult("Ctx: Refresh"); });
                menu->AddItem("Properties", [setResult] { setResult("Ctx: Properties"); });
                menu->AddSeparator();
                // Use the global GetApp() accessor instead of capturing &app:
                // the menu outlives the local scope (it is owned by the overlay
                // stack) and a captured reference could dangle during teardown.
                menu->AddItem("Exit", [] { if (auto* a = nui::GetApp()) a->Quit(); });
                nui::Rect a = w->GetAbsoluteRect();
                nui::Menu::Open(std::move(menu), a.x, a.y + a.h, sw, sh);
            });
        }
    }

    // Animate progress bars
    float t = 0.0f;
    app.SetOnTick([&](float dt) {
        t += dt;

        // Pulse the blue progress bar
        ProgressBar* pb = static_cast<ProgressBar*>(app.GetRoot()->GetChild("progress_blue"));
        if (pb) {
            float v = 0.3f + 0.2f * std::sin(t * 1.5f);
            pb->SetValue(v);
        }
    });

    NUI_LOG("[Example] Starting Hello World demo...\n");
    int code = app.Run();
    NUI_LOG("[Example] Exited with code %d\n", code);
    return code;
}
