// NUI: Adaptive Layouts Demo (Anchor System)
// Demonstrates every feature of the responsive layout system:
//   1. Edge anchor flags (left/top/right/bottom)
//   2. Center anchoring
//   3. Stretch between two opposite edges (fill)
//   4. stretch_w / stretch_h modes (Fixed / Fill / Proportional)
//   5. Normalized anchor points (Godot-style, 0..1)
//   6. min/max size constraints
//   7. Runtime anchor changes via buttons
//   8. Legacy (non-anchored) widgets coexisting with anchored ones
//
// Resize the window to see every region adapt in real time.

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
#include "xml/layout_loader.h"

#include <cstdio>
#include <cstring>
#include <cmath>
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

using namespace nui;

// ── Small helpers ───────────────────────────────────────────────

// A labelled colored box used as a visible anchor target. The label is a
// child of the box so it moves/scales together with it. The label itself is
// anchored to fill the box, so the text stays centered after a resize.
static std::unique_ptr<Widget> MakeBox(const char* name, const char* text,
                                        Color bg, Color border,
                                        int labelSize = 12) {
    auto box = std::make_unique<Widget>();
    box->SetName(name);
    box->SetBgColor(bg);
    box->SetBorderColor(border);

    auto lbl = std::make_unique<Label>();
    lbl->SetName("lbl");
    lbl->SetText(text);
    lbl->SetFontSize(labelSize);
    lbl->SetTextColor(Color::White());
    lbl->SetAlignH(AlignH::Center);
    lbl->SetAlignV(AlignV::Center);
    // Child fills the parent so the label stays centered after a resize.
    lbl->SetRect(0, 0, 1, 1);
    lbl->SetAnchor(AnchorFlag::Left | AnchorFlag::Top |
                   AnchorFlag::Right | AnchorFlag::Bottom);
    lbl->SetStretch(StretchMode::Fill, StretchMode::Fill);
    box->AddChild(std::move(lbl));

    return box;
}

// A section header label.
static std::unique_ptr<Label> MakeSection(const char* text) {
    auto lbl = std::make_unique<Label>();
    lbl->SetText(text);
    lbl->SetFontSize(15);
    lbl->SetTextColor(Color(170, 180, 210));
    return lbl;
}

// ── Build the layouts demo UI ───────────────────────────────────
std::unique_ptr<Widget> BuildLayoutsDemo(FontManager& fonts) {
    auto root = std::make_unique<Widget>();
    root->SetName("root");
    root->SetBgColor(Color(18, 18, 28));

    // ════════════════════════════════════════════════════════════
    // 1. Header bar — anchored top, fills full width
    // ════════════════════════════════════════════════════════════
    auto header = MakeBox("header", "NUI  ·  Anchor System Demo",
                          Color(30, 30, 46), Color(60, 60, 80), 18);
    header->SetRect(0, 0, 1000, 44);
    header->SetAnchor(AnchorFlag::Left | AnchorFlag::Top | AnchorFlag::Right);
    header->SetStretchW(StretchMode::Fill);
    root->AddChild(std::move(header));

    // ════════════════════════════════════════════════════════════
    // 2. Footer bar — anchored bottom, fills full width
    // ════════════════════════════════════════════════════════════
    auto footer = MakeBox("footer",
        "Resize the window  ·  Every colored region adapts",
        Color(25, 25, 38), Color(50, 50, 65));
    footer->SetRect(0, 0, 1000, 28);
    footer->SetAnchor(AnchorFlag::Left | AnchorFlag::Right | AnchorFlag::Bottom);
    footer->SetStretchW(StretchMode::Fill);
    root->AddChild(std::move(footer));

    // ════════════════════════════════════════════════════════════
    // 3. Left sidebar — anchored left + top + bottom (full height stretch)
    //    Fixed width of 200px. Design rect uses the 1000×720 baseline.
    // ════════════════════════════════════════════════════════════
    auto sidebar = MakeBox("sidebar", "Sidebar\nleft top bottom",
                           Color(34, 40, 56), Color(55, 65, 90));
    sidebar->SetRect(0, 44, 200, 648);
    sidebar->SetAnchor(AnchorFlag::Left | AnchorFlag::Top | AnchorFlag::Bottom);
    sidebar->SetStretchH(StretchMode::Fill);
    root->AddChild(std::move(sidebar));

    // ════════════════════════════════════════════════════════════
    // 4. Right sidebar — anchored right + top + bottom, fixed width 180px
    // ════════════════════════════════════════════════════════════
    auto rightbar = MakeBox("rightbar", "Right\nright top bottom",
                            Color(40, 34, 50), Color(70, 55, 85));
    rightbar->SetRect(820, 44, 180, 648);
    rightbar->SetAnchor(AnchorFlag::Right | AnchorFlag::Top | AnchorFlag::Bottom);
    rightbar->SetStretchH(StretchMode::Fill);
    root->AddChild(std::move(rightbar));

    // ════════════════════════════════════════════════════════════
    // 5. Content area — anchored to all four sides of the space between the
    //    header/footer and the two sidebars. Flag-style "all" anchors give a
    //    clean fill that stays between the sidebars on any window size.
    //    (Content local size at the 1000×720 baseline: 620×648.)
    // ════════════════════════════════════════════════════════════
    auto content = std::make_unique<Widget>();
    content->SetName("content");
    content->SetBgColor(Color(22, 22, 32));
    content->SetBorderColor(Color(50, 50, 65));
    // Design rect: x=200 (after sidebar), y=44 (below header),
    // w=620 (to x=820, before rightbar), h=648 (to y=692, above footer).
    content->SetRect(200, 44, 620, 648);
    content->SetAnchor(AnchorFlag::Left | AnchorFlag::Top |
                       AnchorFlag::Right | AnchorFlag::Bottom);
    content->SetStretch(StretchMode::Fill, StretchMode::Fill);

    // ── 5a. Section: "Edge Anchors" ─────────────────────────────
    // All coords below are LOCAL to the content area (0,0 = content origin).
    auto sec1 = MakeSection("1. Edge Anchors (single edge)");
    sec1->SetRect(12, 8, 400, 20);
    sec1->SetAnchor(AnchorFlag::Left | AnchorFlag::Top);
    content->AddChild(std::move(sec1));

    // Corner boxes inside content — each pinned to one corner.
    auto tl = MakeBox("corner_tl", "left -> top", Color(40, 90, 160), Color(70, 130, 200));
    tl->SetRect(12, 32, 110, 60);
    tl->SetAnchor(AnchorFlag::Left | AnchorFlag::Top);
    content->AddChild(std::move(tl));

    auto tr = MakeBox("corner_tr", "right -> top", Color(160, 90, 40), Color(200, 130, 70));
    tr->SetRect(498, 32, 110, 60);
    tr->SetAnchor(AnchorFlag::Right | AnchorFlag::Top);
    content->AddChild(std::move(tr));

    auto bl = MakeBox("corner_bl", "left -> bottom", Color(40, 130, 70), Color(70, 180, 100));
    bl->SetRect(12, 556, 110, 60);
    bl->SetAnchor(AnchorFlag::Left | AnchorFlag::Bottom);
    content->AddChild(std::move(bl));

    auto br = MakeBox("corner_br", "right -> bottom", Color(130, 40, 120), Color(180, 70, 170));
    br->SetRect(498, 556, 110, 60);
    br->SetAnchor(AnchorFlag::Right | AnchorFlag::Bottom);
    content->AddChild(std::move(br));

    // ── 5b. Section: "Center" ───────────────────────────────────
    auto sec2 = MakeSection("2. Center (keeps size, centered)");
    sec2->SetRect(12, 110, 400, 20);
    sec2->SetAnchor(AnchorFlag::Left | AnchorFlag::Top);
    content->AddChild(std::move(sec2));

    auto centerBox = MakeBox("center_box", "anchor=\"center\"",
                              Color(80, 160, 255), Color(120, 190, 255));
    centerBox->SetRect(230, 132, 160, 50);
    centerBox->SetAnchor(AnchorFlag::None); // center mode
    content->AddChild(std::move(centerBox));

    // ── 5c. Section: "Stretch / Fill" ───────────────────────────
    auto sec3 = MakeSection("3. Stretch between two edges (Fill)");
    sec3->SetRect(12, 200, 400, 20);
    sec3->SetAnchor(AnchorFlag::Left | AnchorFlag::Top);
    content->AddChild(std::move(sec3));

    // A bar that stretches horizontally across the content area.
    auto stretchH = MakeBox("stretch_h", "left + right  →  fill width",
                             Color(180, 100, 40), Color(220, 130, 60));
    stretchH->SetRect(12, 222, 596, 36);
    stretchH->SetAnchor(AnchorFlag::Left | AnchorFlag::Right | AnchorFlag::Top);
    stretchH->SetStretchW(StretchMode::Fill);
    content->AddChild(std::move(stretchH));

    // A bar that stretches vertically (left + top + bottom).
    auto stretchV = MakeBox("stretch_v", "top + bottom",
                             Color(60, 140, 80), Color(90, 180, 110));
    stretchV->SetRect(12, 268, 60, 348);
    stretchV->SetAnchor(AnchorFlag::Left | AnchorFlag::Top | AnchorFlag::Bottom);
    stretchV->SetStretchH(StretchMode::Fill);
    content->AddChild(std::move(stretchV));

    // ── 5d. Section: "Proportional" ─────────────────────────────
    auto sec4 = MakeSection("4. Proportional stretch (scales with parent)");
    sec4->SetRect(80, 268, 400, 20);
    sec4->SetAnchor(AnchorFlag::Left | AnchorFlag::Top);
    content->AddChild(std::move(sec4));

    auto propBox = MakeBox("prop_box", "stretch_w=\"proportional\"",
                           Color(160, 60, 200), Color(200, 90, 240));
    propBox->SetRect(80, 292, 200, 50);
    propBox->SetAnchor(AnchorFlag::Left | AnchorFlag::Top);
    propBox->SetStretchW(StretchMode::Proportional);
    content->AddChild(std::move(propBox));

    // ── 5e. Section: "Min / Max size" ───────────────────────────
    auto sec5 = MakeSection("5. Min / Max size constraints");
    sec5->SetRect(12, 360, 400, 20);
    sec5->SetAnchor(AnchorFlag::Left | AnchorFlag::Top);
    content->AddChild(std::move(sec5));

    // A bar that stretches to fill width but is clamped to [100 .. 300].
    auto minMaxBox = MakeBox("minmax_box", "stretch=fill, min=100, max=300",
                              Color(200, 170, 30), Color(240, 220, 60));
    minMaxBox->SetRect(12, 382, 596, 36);
    minMaxBox->SetAnchor(AnchorFlag::Left | AnchorFlag::Right | AnchorFlag::Top);
    minMaxBox->SetStretchW(StretchMode::Fill);
    minMaxBox->SetMinSize(100, 0);
    minMaxBox->SetMaxSize(300, INT_MAX);
    content->AddChild(std::move(minMaxBox));

    // ── 5f. Section: "Runtime changes" ──────────────────────────
    auto sec6 = MakeSection("6. Runtime anchor changes (click buttons)");
    sec6->SetRect(12, 440, 400, 20);
    sec6->SetAnchor(AnchorFlag::Left | AnchorFlag::Top);
    content->AddChild(std::move(sec6));

    // Target box whose anchor is changed by buttons below.
    auto target = MakeBox("target", "target",
                          Color(50, 120, 180), Color(80, 160, 220));
    target->SetRect(12, 462, 120, 50);
    target->SetAnchor(AnchorFlag::Left | AnchorFlag::Top);
    content->AddChild(std::move(target));

    // Buttons row: a fixed-height panel pinned to the bottom of the content
    // area. Buttons inside it use absolute coords (the panel keeps its height,
    // so the design snapshot is stable regardless of the content size).
    auto btnRow = std::make_unique<Widget>();
    btnRow->SetName("btn_row");
    btnRow->SetRect(12, 604, 596, 32);
    btnRow->SetAnchor(AnchorFlag::Left | AnchorFlag::Right | AnchorFlag::Bottom);
    btnRow->SetStretchW(StretchMode::Fill);

    struct BtnDef { const char* name; const char* text; };
    BtnDef btns[] = {
        {"btn_tl", "TL"},
        {"btn_tr", "TR"},
        {"btn_bl", "BL"},
        {"btn_br", "BR"},
        {"btn_ct", "Center"},
        {"btn_al", "Fill"},
    };

    for (int i = 0; i < 6; ++i) {
        auto btn = std::make_unique<Button>();
        btn->SetName(btns[i].name);
        btn->SetText(btns[i].text);
        btn->SetFontSize(11);
        btn->SetBgColor(Color(45, 55, 75));
        btn->SetHoverColor(Color(65, 80, 110));
        btn->SetRect(0 + i * 98, 2, 90, 28);
        // Buttons are NOT anchored: they keep their position inside the
        // fixed-height button row (demonstrates legacy widgets coexisting
        // with anchored parents).
        btnRow->AddChild(std::move(btn));
    }
    content->AddChild(std::move(btnRow));

    root->AddChild(std::move(content));

    return root;
}

// ── Main ────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    SetWorkDirToExe();
    ResourceManager::Initialize();

    Application app;
    AppDesc desc;
    desc.title    = "NUI · Adaptive Layouts (Anchor System)";
    desc.width    = 1000;
    desc.height   = 720;
    desc.resizable = true;

    if (!app.Initialize(desc)) {
        NUI_LOG_ERROR("Failed to initialize application\n");
        return 1;
    }

    TextureCache textures;
    LayoutLoader loader;

    // Try XML layout first, fall back to programmatic.
    std::unique_ptr<Widget> root;
    root = loader.LoadFromFile("resources/layouts/layouts-demo.xml",
                                textures, *app.GetFontManager());
    if (!root) {
        NUI_LOG("[Layouts] No XML layout found, building programmatically\n");
        root = BuildLayoutsDemo(*app.GetFontManager());
    }
    if (root) {
        root->SetRect(0, 0, desc.width, desc.height);
        app.SetRoot(std::move(root));
    }

    // ── Wire up the runtime anchor-change buttons ───────────────
    // Each button re-anchors the "target" box and logs the change.
    {
        nui::Widget* r = app.GetRoot();
        nui::Widget* content = r ? r->GetChild("content") : nullptr;
        nui::Widget* target  = content ? content->GetChild("target") : nullptr;

        auto setAnchor = [target](AnchorFlag f, const char* label) {
            if (!target) return;
            // Reset to a known size before re-anchoring so the design snapshot
            // captured by SetAnchor is predictable.
            target->SetRect(12, 0, 120, 50);
            target->SetAnchor(f);
            if (f == AnchorFlag::None) {
                // center mode: keep the box at a fixed size
                target->SetStretch(StretchMode::Fixed, StretchMode::Fixed);
            } else if ((f & AnchorFlag::Left)   != AnchorFlag::None &&
                       (f & AnchorFlag::Right)  != AnchorFlag::None &&
                       (f & AnchorFlag::Top)    != AnchorFlag::None &&
                       (f & AnchorFlag::Bottom) != AnchorFlag::None) {
                target->SetStretch(StretchMode::Fill, StretchMode::Fill);
            } else {
                target->SetStretch(StretchMode::Fixed, StretchMode::Fixed);
            }
            NUI_LOG("[Layouts] target anchor → %s\n", label);
        };

        struct Wire { const char* name; AnchorFlag f; const char* label; };
        Wire wires[] = {
            {"btn_tl", AnchorFlag::Left  | AnchorFlag::Top,                    "TopLeft"},
            {"btn_tr", AnchorFlag::Right | AnchorFlag::Top,                    "TopRight"},
            {"btn_bl", AnchorFlag::Left  | AnchorFlag::Bottom,                 "BotLeft"},
            {"btn_br", AnchorFlag::Right | AnchorFlag::Bottom,                 "BotRight"},
            {"btn_ct", AnchorFlag::None,                                      "Center"},
            {"btn_al", AnchorFlag::Left | AnchorFlag::Top |
                       AnchorFlag::Right | AnchorFlag::Bottom,                 "FillAll"},
        };
        for (const auto& w : wires) {
            if (nui::Widget* btn = content ? content->GetChild(w.name) : nullptr) {
                AnchorFlag f = w.f;
                const char* label = w.label;
                btn->SetOnClick([setAnchor, f, label](nui::Widget*) {
                    setAnchor(f, label);
                });
            }
        }
    }

    NUI_LOG("[Layouts] Starting Anchor System demo...\n");
    NUI_LOG("[Layouts] Resize the window to see regions adapt.\n");
    int code = app.Run();
    NUI_LOG("[Layouts] Exited with code %d\n", code);
    return code;
}
