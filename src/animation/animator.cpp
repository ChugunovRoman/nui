// NUI: Animator implementation

#include "animation/animator.h"
#include "ui/widget.h"
#include <algorithm>

namespace nui {

std::vector<std::unique_ptr<Tween>>& Animator::GetTweens() {
    static std::vector<std::unique_ptr<Tween>> tweens;
    return tweens;
}

Tween* Animator::Animate(float* target, float from, float to,
                          float duration, EaseType ease) {
    auto tween = std::make_unique<Tween>(target, from, to, duration, ease);
    Tween* ptr = tween.get();
    GetTweens().push_back(std::move(tween));
    return ptr;
}

Tween* Animator::Animate(float from, float to, float duration,
                          EaseType ease) {
    auto tween = std::make_unique<Tween>(from, to, duration, ease);
    Tween* ptr = tween.get();
    GetTweens().push_back(std::move(tween));
    return ptr;
}

Tween* Animator::AnimateX(Widget* widget, float to, float duration, EaseType ease) {
    if (!widget) return nullptr;
    float from = static_cast<float>(widget->GetX());
    Tween* t = Animate(from, to, duration, ease);
    t->OnUpdate([widget](float v) { widget->SetPos(static_cast<int>(v), widget->GetY()); });
    return t;
}

Tween* Animator::AnimateY(Widget* widget, float to, float duration, EaseType ease) {
    if (!widget) return nullptr;
    float from = static_cast<float>(widget->GetY());
    Tween* t = Animate(from, to, duration, ease);
    t->OnUpdate([widget](float v) { widget->SetPos(widget->GetX(), static_cast<int>(v)); });
    return t;
}

Tween* Animator::AnimateWidth(Widget* widget, float to, float duration, EaseType ease) {
    if (!widget) return nullptr;
    float from = static_cast<float>(widget->GetWidth());
    Tween* t = Animate(from, to, duration, ease);
    t->OnUpdate([widget](float v) { widget->SetSize(static_cast<int>(v), widget->GetHeight()); });
    return t;
}

Tween* Animator::AnimateHeight(Widget* widget, float to, float duration, EaseType ease) {
    if (!widget) return nullptr;
    float from = static_cast<float>(widget->GetHeight());
    Tween* t = Animate(from, to, duration, ease);
    t->OnUpdate([widget](float v) { widget->SetSize(widget->GetWidth(), static_cast<int>(v)); });
    return t;
}

Tween* Animator::AnimateAlpha(Widget* widget, float to, float duration, EaseType ease) {
    if (!widget) return nullptr;
    float from = static_cast<float>(widget->GetBgColor().a) / 255.0f;
    Tween* t = Animate(from, to, duration, ease);
    t->OnUpdate([widget](float v) {
        Color c = widget->GetBgColor();
        c.a = static_cast<uint8_t>(v * 255);
        widget->SetBgColor(c);
    });
    return t;
}

void Animator::CancelByTag(const std::string& tag) {
    auto& tweens = GetTweens();
    for (auto& t : tweens) {
        if (t && t->GetTag() == tag) {
            t->Cancel();
        }
    }
}

void Animator::CancelAll() {
    auto& tweens = GetTweens();
    for (auto& t : tweens) {
        if (t) t->Cancel();
    }
}

void Animator::UpdateAll(float dt) {
    auto& tweens = GetTweens();
    for (auto& t : tweens) {
        if (t && t->IsActive()) {
            t->Update(dt);
        }
    }
    // Remove finished tweens
    tweens.erase(
        std::remove_if(tweens.begin(), tweens.end(),
            [](const std::unique_ptr<Tween>& t) {
                return !t || t->IsFinished();
            }),
        tweens.end()
    );
}

size_t Animator::GetActiveCount() {
    auto& tweens = GetTweens();
    size_t count = 0;
    for (auto& t : tweens) {
        if (t && t->IsActive()) count++;
    }
    return count;
}

} // namespace nui
