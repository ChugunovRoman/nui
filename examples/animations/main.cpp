// NUI: Animation Demo
// Demonstrates all available animation types and easing functions.

#include "core/application.h"
#include "core/input.h"
#include "core/log.h"
#include "renderer/resource.h"
#include "renderer/canvas.h"
#include "renderer/font.h"
#include "animation/animator.h"
#include "animation/easing.h"
#include "ui/widget.h"
#include "ui/label.h"
#include "ui/button.h"
#include "ui/editbox.h"
#include "ui/progressbar.h"

#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

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

// ── Animated box that moves along X ─────────────────────────────
struct AnimBox {
    std::unique_ptr<Widget> widget;
    float startX, endX;
};

// ── Build the demo UI ───────────────────────────────────────────
std::unique_ptr<Widget> BuildAnimDemo(FontManager& fonts) {
    auto root = std::make_unique<Widget>();
    root->SetRect(0, 0, 1200, 800);
    root->SetBgColor(Color(18, 18, 28));

    // ── Title ───────────────────────────────────────────────────
    auto title = std::make_unique<Label>();
    title->SetRect(0, 8, 1200, 36);
    title->SetText("NUI Animation Demo");
    title->SetFontSize(28);
    title->SetTextColor(Color(200, 210, 255));
    title->SetAlignH(AlignH::Center);
    root->AddChild(std::move(title));

    // ── Section 1: Easing Functions (top-left) ──────────────────
    auto sec1 = std::make_unique<Label>();
    sec1->SetRect(20, 55, 380, 22);
    sec1->SetText("1. Easing Functions Comparison");
    sec1->SetFontSize(16);
    sec1->SetTextColor(Color(170, 180, 210));
    root->AddChild(std::move(sec1));

    struct EaseDemo { const char* name; EaseType type; };
    EaseDemo eases[] = {
        {"Linear",       EaseType::Linear},
        {"InQuad",       EaseType::InQuad},
        {"OutQuad",      EaseType::OutQuad},
        {"InOutQuad",    EaseType::InOutQuad},
        {"InCubic",      EaseType::InCubic},
        {"OutCubic",     EaseType::OutCubic},
        {"InOutCubic",   EaseType::InOutCubic},
        {"InElastic",    EaseType::InElastic},
        {"OutElastic",   EaseType::OutElastic},
        {"InOutElastic", EaseType::InOutElastic},
        {"InBack",       EaseType::InBack},
        {"OutBack",      EaseType::OutBack},
    };
    int numEases = sizeof(eases) / sizeof(eases[0]);

    for (int i = 0; i < numEases; ++i) {
        int row = i;
        int y = 85 + row * 26;

        // Label
        auto lbl = std::make_unique<Label>();
        lbl->SetRect(20, y, 100, 20);
        lbl->SetText(eases[i].name);
        lbl->SetFontSize(11);
        lbl->SetTextColor(Color(160, 165, 190));
        root->AddChild(std::move(lbl));

        // Track (background)
        auto track = std::make_unique<Widget>();
        track->SetRect(130, y + 3, 260, 14);
        track->SetBgColor(Color(40, 40, 55));
        root->AddChild(std::move(track));

        // Animated dot
        auto dot = std::make_unique<Widget>();
        dot->SetRect(130, y + 1, 18, 18);
        dot->SetBgColor(Color(80, 160, 255));
        Widget* dotPtr = dot.get();
        root->AddChild(std::move(dot));

        // Animate the dot X position (ping-pong)
        Animator::Animate(130.0f, 375.0f, 2.0f, eases[i].type)
            ->OnUpdate([dotPtr](float v) {
                dotPtr->SetPos(static_cast<int>(v), dotPtr->GetY());
            })
            .SetPingPong(true)
            .SetLoop(true)
            .SetDelay(static_cast<float>(i) * 0.15f);
    }

    // ── Section 2: Widget Animations (top-right) ────────────────
    auto sec2 = std::make_unique<Label>();
    sec2->SetRect(420, 55, 380, 22);
    sec2->SetText("2. Widget Property Animations");
    sec2->SetFontSize(16);
    sec2->SetTextColor(Color(170, 180, 210));
    root->AddChild(std::move(sec2));

    // -- Slide in from left --
    auto slideBox = std::make_unique<Widget>();
    slideBox->SetRect(420, 90, 160, 40);
    slideBox->SetBgColor(Color(60, 140, 80));
    slideBox->SetBorderColor(Color(80, 180, 100));
    Widget* slidePtr = slideBox.get();
    root->AddChild(std::move(slideBox));

    auto slideLabel = std::make_unique<Label>();
    slideLabel->SetRect(420, 95, 160, 30);
    slideLabel->SetText("Slide In");
    slideLabel->SetFontSize(13);
    slideLabel->SetTextColor(Color::White());
    slideLabel->SetAlignH(AlignH::Center);
    root->AddChild(std::move(slideLabel));

    Animator::AnimateX(slidePtr, 420.0f, 1.0f, EaseType::OutBack)
        ->SetLoop(true).SetPingPong(true).SetDelay(0.5f);
    // Start off-screen
    slidePtr->SetPos(200, 90);

    // -- Grow width --
    auto growBox = std::make_unique<Widget>();
    growBox->SetRect(420, 140, 30, 40);
    growBox->SetBgColor(Color(180, 100, 40));
    growBox->SetBorderColor(Color(220, 130, 60));
    Widget* growPtr = growBox.get();
    root->AddChild(std::move(growBox));

    auto growLabel = std::make_unique<Label>();
    growLabel->SetRect(420, 145, 160, 30);
    growLabel->SetText("Grow Width");
    growLabel->SetFontSize(13);
    growLabel->SetTextColor(Color::White());
    root->AddChild(std::move(growLabel));

    Animator::AnimateWidth(growPtr, 200.0f, 1.5f, EaseType::InOutCubic)
        ->SetLoop(true).SetPingPong(true);

    // -- Bounce Y --
    auto bounceBox = std::make_unique<Widget>();
    bounceBox->SetRect(650, 90, 40, 40);
    bounceBox->SetBgColor(Color(200, 60, 60));
    bounceBox->SetBorderColor(Color(240, 80, 80));
    Widget* bouncePtr = bounceBox.get();
    root->AddChild(std::move(bounceBox));

    auto bounceLabel = std::make_unique<Label>();
    bounceLabel->SetRect(650, 80, 120, 15);
    bounceLabel->SetText("Bounce");
    bounceLabel->SetFontSize(11);
    bounceLabel->SetTextColor(Color(160, 165, 190));
    root->AddChild(std::move(bounceLabel));

    Animator::AnimateY(bouncePtr, 200.0f, 0.8f, EaseType::OutBounce)
        ->SetLoop(true).SetPingPong(true);

    // -- Pulse (scale via width+height) --
    auto pulseBox = std::make_unique<Widget>();
    pulseBox->SetRect(730, 90, 50, 50);
    pulseBox->SetBgColor(Color(160, 60, 200));
    pulseBox->SetBorderColor(Color(190, 80, 240));
    Widget* pulsePtr = pulseBox.get();
    root->AddChild(std::move(pulseBox));

    auto pulseLabel = std::make_unique<Label>();
    pulseLabel->SetRect(730, 80, 120, 15);
    pulseLabel->SetText("Pulse");
    pulseLabel->SetFontSize(11);
    pulseLabel->SetTextColor(Color(160, 165, 190));
    root->AddChild(std::move(pulseLabel));

    Animator::Animate(50.0f, 70.0f, 0.6f, EaseType::InOutSine)
        ->OnUpdate([pulsePtr](float v) {
            int s = static_cast<int>(v);
            pulsePtr->SetSize(s, s);
        })
        .SetLoop(true).SetPingPong(true);

    // -- Rotate color (hue shift) --
    auto colorBox = std::make_unique<Widget>();
    colorBox->SetRect(820, 90, 60, 60);
    colorBox->SetBgColor(Color(255, 0, 0));
    Widget* colorPtr = colorBox.get();
    root->AddChild(std::move(colorBox));

    auto colorLabel = std::make_unique<Label>();
    colorLabel->SetRect(820, 80, 120, 15);
    colorLabel->SetText("Color Cycle");
    colorLabel->SetFontSize(11);
    colorLabel->SetTextColor(Color(160, 165, 190));
    root->AddChild(std::move(colorLabel));

    Animator::Animate(0.0f, 360.0f, 3.0f, EaseType::Linear)
        ->OnUpdate([colorPtr](float v) {
            // HSV to RGB (simplified)
            float h = v / 60.0f;
            int hi = static_cast<int>(h) % 6;
            float f = h - static_cast<int>(h);
            uint8_t v255 = 255;
            uint8_t q = static_cast<uint8_t>(255 * (1 - f));
            uint8_t t = static_cast<uint8_t>(255 * f);
            uint8_t r, g, b;
            switch (hi) {
                case 0: r = v255; g = t;     b = 0;     break;
                case 1: r = q;    g = v255;  b = 0;     break;
                case 2: r = 0;    g = v255;  b = t;     break;
                case 3: r = 0;    g = q;     b = v255;  break;
                case 4: r = t;    g = 0;     b = v255;  break;
                default:r = v255; g = 0;     b = q;     break;
            }
            colorPtr->SetBgColor(Color(r, g, b));
        })
        .SetLoop(true);

    // ── Section 3: Button-triggered animations ──────────────────
    auto sec3 = std::make_unique<Label>();
    sec3->SetRect(420, 190, 380, 22);
    sec3->SetText("3. Button-Triggered Animations");
    sec3->SetFontSize(16);
    sec3->SetTextColor(Color(170, 180, 210));
    root->AddChild(std::move(sec3));

    // -- Target box for button animations --
    auto targetBox = std::make_unique<Widget>();
    targetBox->SetRect(420, 225, 100, 100);
    targetBox->SetBgColor(Color(50, 120, 180));
    targetBox->SetBorderColor(Color(70, 150, 220));
    Widget* targetPtr = targetBox.get();
    root->AddChild(std::move(targetBox));

    // -- Fly In button --
    auto btnFly = std::make_unique<Button>();
    btnFly->SetRect(540, 225, 120, 30);
    btnFly->SetText("Fly In");
    btnFly->SetFontSize(12);
    btnFly->SetBgColor(Color(40, 100, 160));
    btnFly->SetHoverColor(Color(50, 120, 190));
    btnFly->SetOnClick([targetPtr](Widget*) {
        targetPtr->SetPos(-100, 225);
        Animator::AnimateX(targetPtr, 420.0f, 0.5f, EaseType::OutCubic);
    });
    root->AddChild(std::move(btnFly));

    // -- Bounce In button --
    auto btnBounce = std::make_unique<Button>();
    btnBounce->SetRect(540, 260, 120, 30);
    btnBounce->SetText("Bounce In");
    btnBounce->SetFontSize(12);
    btnBounce->SetBgColor(Color(160, 80, 40));
    btnBounce->SetHoverColor(Color(190, 100, 50));
    btnBounce->SetOnClick([targetPtr](Widget*) {
        targetPtr->SetPos(420, -100);
        Animator::AnimateY(targetPtr, 225.0f, 0.8f, EaseType::OutBounce);
    });
    root->AddChild(std::move(btnBounce));

    // -- Fade In button --
    auto btnFade = std::make_unique<Button>();
    btnFade->SetRect(540, 295, 120, 30);
    btnFade->SetText("Fade In");
    btnFade->SetFontSize(12);
    btnFade->SetBgColor(Color(100, 50, 150));
    btnFade->SetHoverColor(Color(130, 70, 180));
    btnFade->SetOnClick([targetPtr](Widget*) {
        // Start invisible, fade to visible
        targetPtr->SetBgColor(Color(50, 120, 180, 0));
        Animator::AnimateAlpha(targetPtr, 1.0f, 0.8f, EaseType::OutCubic);
    });
    root->AddChild(std::move(btnFade));

    // -- Fade Out button --
    auto btnFadeOut = std::make_unique<Button>();
    btnFadeOut->SetRect(540, 330, 120, 30);
    btnFadeOut->SetText("Fade Out");
    btnFadeOut->SetFontSize(12);
    btnFadeOut->SetBgColor(Color(80, 60, 120));
    btnFadeOut->SetHoverColor(Color(100, 80, 150));
    btnFadeOut->SetOnClick([targetPtr](Widget*) {
        // Fade to invisible, then reset
        Animator::AnimateAlpha(targetPtr, 0.0f, 0.8f, EaseType::InCubic)
            ->OnComplete([targetPtr]() {
                targetPtr->SetRect(420, 225, 100, 100);
                targetPtr->SetBgColor(Color(50, 120, 180));
            });
    });
    root->AddChild(std::move(btnFadeOut));

    // -- Elastic Width button --
    auto btnElastic = std::make_unique<Button>();
    btnElastic->SetRect(670, 225, 120, 30);
    btnElastic->SetText("Elastic");
    btnElastic->SetFontSize(12);
    btnElastic->SetBgColor(Color(60, 140, 60));
    btnElastic->SetHoverColor(Color(80, 170, 80));
    btnElastic->SetOnClick([targetPtr](Widget*) {
        targetPtr->SetSize(10, 100);
        Animator::AnimateWidth(targetPtr, 300.0f, 1.2f, EaseType::OutElastic);
    });
    root->AddChild(std::move(btnElastic));

    // -- Reset button --
    auto btnReset = std::make_unique<Button>();
    btnReset->SetRect(670, 260, 120, 30);
    btnReset->SetText("Reset");
    btnReset->SetFontSize(12);
    btnReset->SetBgColor(Color(80, 80, 80));
    btnReset->SetHoverColor(Color(100, 100, 100));
    btnReset->SetOnClick([targetPtr](Widget*) {
        Animator::CancelAll();
        targetPtr->SetRect(420, 225, 100, 100);
        targetPtr->SetBgColor(Color(50, 120, 180));
    });
    root->AddChild(std::move(btnReset));

    // ── Section 4: Progress Bar Animation ───────────────────────
    auto sec4 = std::make_unique<Label>();
    sec4->SetRect(420, 340, 380, 22);
    sec4->SetText("4. Progress Bar Animation");
    sec4->SetFontSize(16);
    sec4->SetTextColor(Color(170, 180, 210));
    root->AddChild(std::move(sec4));

    auto prog = std::make_unique<ProgressBar>();
    prog->SetRect(420, 370, 300, 24);
    prog->SetValue(0.0f);
    prog->SetFillColor(Color(40, 160, 80));
    prog->SetShowPercent(true);
    prog->SetBorderColor(Color(60, 60, 80));
    ProgressBar* progPtr = prog.get();
    root->AddChild(std::move(prog));

    float progressVal = 0.0f;
    Animator::Animate(0.0f, 1.0f, 4.0f, EaseType::InOutCubic)
        ->OnUpdate([progPtr](float v) { progPtr->SetValue(v); })
        .SetLoop(true).SetDelay(1.0f);

    // ── Section 5: Transparency demo ────────────────────────────
    auto sec5 = std::make_unique<Label>();
    sec5->SetRect(420, 405, 380, 22);
    sec5->SetText("5. Transparency (alpha blending)");
    sec5->SetFontSize(16);
    sec5->SetTextColor(Color(170, 180, 210));
    root->AddChild(std::move(sec5));

    // Background pattern (checkerboard) to show transparency
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 8; ++col) {
            auto checker = std::make_unique<Widget>();
            int cx = 420 + col * 50;
            int cy = 435 + row * 50;
            checker->SetRect(cx, cy, 50, 50);
            bool dark = (row + col) % 2 == 0;
            checker->SetBgColor(dark ? Color(60, 60, 70) : Color(80, 80, 90));
            root->AddChild(std::move(checker));
        }
    }

    // Transparent boxes with different alpha levels
    struct AlphaDemo { int alpha; const char* label; };
    AlphaDemo alphas[] = {
        {255, "100%"},
        {192, "75%"},
        {128, "50%"},
        {64,  "25%"},
        {32,  "12%"},
        {0,   "0%"},
    };
    for (int i = 0; i < 6; ++i) {
        auto box = std::make_unique<Widget>();
        box->SetRect(425 + i * 50, 440, 40, 40);
        box->SetBgColor(Color(80, 160, 255, alphas[i].alpha));
        box->SetBorderColor(Color(120, 190, 255, alphas[i].alpha));
        root->AddChild(std::move(box));

        auto lbl = std::make_unique<Label>();
        lbl->SetRect(425 + i * 50, 482, 40, 15);
        lbl->SetText(alphas[i].label);
        lbl->SetFontSize(10);
        lbl->SetTextColor(Color(150, 150, 170));
        lbl->SetAlignH(AlignH::Center);
        root->AddChild(std::move(lbl));
    }

    // Manual alpha control
    auto manualLabel = std::make_unique<Label>();
    manualLabel->SetRect(420, 505, 200, 18);
    manualLabel->SetText("Manual alpha (0-100):");
    manualLabel->SetFontSize(12);
    manualLabel->SetTextColor(Color(160, 165, 190));
    root->AddChild(std::move(manualLabel));

    // Sample widget to demonstrate manual alpha
    auto sampleBox = std::make_unique<Widget>();
    sampleBox->SetRect(720, 440, 60, 60);
    sampleBox->SetBgColor(Color(80, 200, 120, 255));
    sampleBox->SetBorderColor(Color(100, 240, 150, 255));
    Widget* samplePtr = sampleBox.get();
    root->AddChild(std::move(sampleBox));

    // EditBox for alpha value
    auto alphaEdit = std::make_unique<EditBox>();
    alphaEdit->SetRect(420, 528, 100, 28);
    alphaEdit->SetText("100");
    alphaEdit->SetPlaceholder("0-100");
    alphaEdit->SetFontSize(14);
    alphaEdit->SetMaxLength(3);
    EditBox* alphaEditPtr = alphaEdit.get();
    root->AddChild(std::move(alphaEdit));

    // Set button
    auto btnSetAlpha = std::make_unique<Button>();
    btnSetAlpha->SetRect(530, 528, 70, 28);
    btnSetAlpha->SetText("Set");
    btnSetAlpha->SetFontSize(13);
    btnSetAlpha->SetBgColor(Color(60, 130, 80));
    btnSetAlpha->SetHoverColor(Color(80, 160, 100));
    btnSetAlpha->SetOnClick([alphaEditPtr, samplePtr](Widget*) {
        const std::string& text = alphaEditPtr->GetText();
        int val = 100;
        if (!text.empty()) {
            val = std::max(0, std::min(100, std::atoi(text.c_str())));
        }
        uint8_t a = static_cast<uint8_t>(val * 255 / 100);
        samplePtr->SetBgColor(Color(80, 200, 120, a));
        samplePtr->SetBorderColor(Color(100, 240, 150, a));
    });
    root->AddChild(std::move(btnSetAlpha));

    // Preset buttons: 0%, 25%, 50%, 75%, 100%
    const char* presets[] = {"0", "25", "50", "75", "100"};
    for (int i = 0; i < 5; ++i) {
        auto btn = std::make_unique<Button>();
        btn->SetRect(610 + i * 35, 528, 30, 28);
        btn->SetText(presets[i]);
        btn->SetFontSize(10);
        btn->SetBgColor(Color(50, 50, 70));
        btn->SetHoverColor(Color(70, 70, 100));
        int presetVal = std::atoi(presets[i]);
        btn->SetOnClick([alphaEditPtr, samplePtr, presetVal](Widget*) {
            alphaEditPtr->SetText(std::to_string(presetVal));
            uint8_t a = static_cast<uint8_t>(presetVal * 255 / 100);
            samplePtr->SetBgColor(Color(80, 200, 120, a));
            samplePtr->SetBorderColor(Color(100, 240, 150, a));
        });
        root->AddChild(std::move(btn));
    }

    // Fading box animation
    auto fadeBox = std::make_unique<Widget>();
    fadeBox->SetRect(420, 570, 290, 40);
    fadeBox->SetBgColor(Color(200, 80, 80, 255));
    Widget* fadePtr = fadeBox.get();
    root->AddChild(std::move(fadeBox));

    auto fadeLabel = std::make_unique<Label>();
    fadeLabel->SetRect(420, 575, 290, 30);
    fadeLabel->SetText("Pulsing Alpha");
    fadeLabel->SetFontSize(14);
    fadeLabel->SetTextColor(Color::White());
    fadeLabel->SetAlignH(AlignH::Center);
    root->AddChild(std::move(fadeLabel));

    // Animate alpha: 255 → 32 → 255 (ping-pong)
    Animator::Animate(1.0f, 0.12f, 2.0f, EaseType::InOutSine)
        ->OnUpdate([fadePtr](float v) {
            uint8_t a = static_cast<uint8_t>(v * 255);
            fadePtr->SetBgColor(Color(200, 80, 80, a));
        })
        .SetLoop(true).SetPingPong(true);

    // ── Section 6: Sequential animation chain ───────────────────
    auto sec6 = std::make_unique<Label>();
    sec6->SetRect(20, 420, 380, 22);
    sec6->SetText("6. Sequential Chain (Y → X → Width)");
    sec6->SetFontSize(16);
    sec6->SetTextColor(Color(170, 180, 210));
    root->AddChild(std::move(sec6));

    auto chainBox = std::make_unique<Widget>();
    chainBox->SetRect(20, 455, 60, 60);
    chainBox->SetBgColor(Color(200, 180, 40));
    chainBox->SetBorderColor(Color(240, 220, 60));
    Widget* chainPtr = chainBox.get();
    root->AddChild(std::move(chainBox));

    // Chain: move Y → move X → grow width → shrink back → loop
    auto t1 = Animator::AnimateY(chainPtr, 550.0f, 0.6f, EaseType::OutCubic);
    t1->SetTag("chain");
    t1->OnComplete([chainPtr]() {
        auto t2 = Animator::AnimateX(chainPtr, 300.0f, 0.6f, EaseType::InOutQuad);
        t2->SetTag("chain");
        t2->OnComplete([chainPtr]() {
            auto t3 = Animator::AnimateWidth(chainPtr, 200.0f, 0.5f, EaseType::OutBack);
            t3->SetTag("chain");
            t3->OnComplete([chainPtr]() {
                auto t4 = Animator::AnimateWidth(chainPtr, 60.0f, 0.4f, EaseType::InQuad);
                t4->SetTag("chain");
                t4->OnComplete([chainPtr]() {
                    auto t5 = Animator::AnimateX(chainPtr, 20.0f, 0.6f, EaseType::InOutCubic);
                    t5->SetTag("chain");
                    t5->OnComplete([chainPtr]() {
                        auto t6 = Animator::AnimateY(chainPtr, 455.0f, 0.6f, EaseType::InBack);
                        t6->SetTag("chain");
                    });
                });
            });
        });
    });

    // ── Section 7: Delayed staggered entrance ───────────────────
    auto sec7 = std::make_unique<Label>();
    sec7->SetRect(20, 530, 380, 22);
    sec7->SetText("7. Staggered Entrance (delay)");
    sec7->SetFontSize(16);
    sec7->SetTextColor(Color(170, 180, 210));
    root->AddChild(std::move(sec7));

    for (int i = 0; i < 8; ++i) {
        auto box = std::make_unique<Widget>();
        box->SetRect(20 + i * 45, 570, 35, 35);
        box->SetBgColor(Color(40 + i * 20, 100, 200 - i * 15));
        Widget* boxPtr = box.get();
        root->AddChild(std::move(box));

        boxPtr->SetPos(20 + i * 45, 700);
        Animator::AnimateY(boxPtr, 570.0f, 0.5f, EaseType::OutBack)
            ->SetDelay(static_cast<float>(i) * 0.1f);
    }

    // ── Footer ──────────────────────────────────────────────────
    auto footer = std::make_unique<Label>();
    footer->SetRect(0, 770, 1200, 24);
    footer->SetText("All animations run on CPU · No GPU required · Click buttons to trigger · Auto-looping demos above");
    footer->SetFontSize(11);
    footer->SetTextColor(Color(80, 80, 100));
    footer->SetAlignH(AlignH::Center);
    root->AddChild(std::move(footer));

    return root;
}

// ── Main ────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    SetWorkDirToExe();
    ResourceManager::Initialize();

    Application app;
    AppDesc desc;
    desc.title = "NUI Animation Demo";
    desc.width = 1200;
    desc.height = 800;
    desc.resizable = true;

    if (!app.Initialize(desc)) {
        NUI_LOG_ERROR("Failed to initialize application\n");
        return 1;
    }

    auto root = BuildAnimDemo(*app.GetFontManager());
    if (root) {
        root->SetRect(0, 0, desc.width, desc.height);
        app.SetRoot(std::move(root));
    }

    NUI_LOG("[Animations] Starting demo...\n");
    int code = app.Run();
    NUI_LOG("[Animations] Exited with code %d\n", code);
    return code;
}
