# NUI Animations Demo

<img src="animations.gif" alt="NUI Animations Demo" width="800"/>

Демонстрация всех возможностей системы анимаций NUI.

## Виджеты и анимации

| Секция | Что демонстрируется |
|--------|---------------------|
| **1. Easing Functions** | 12 точек с разными easing кривыми (Linear, Quad, Cubic, Elastic, Back, Bounce) — ping-pong loop |
| **2. Widget Properties** | Slide In (X), Grow Width, Bounce Y, Pulse (size), Color Cycle (HSV) |
| **3. Button-Triggered** | Fly In, Bounce In, Fade In, Fade Out, Elastic Width, Reset |
| **4. Progress Bar** | Плавная анимация заполнения 0→100% |
| **5. Transparency** | 6 квадратов с разной прозрачностью (100%→0%) + ручной EditBox + пульсирующий alpha |
| **6. Sequential Chain** | Последовательная цепочка: Y → X → Width → Width → X → Y |
| **7. Staggered Entrance** | 8 блоков с задержкой появления (stagger) |
| **8. Rotation** | Center spin, Corner spin, Oscillate, Rotate button |

## Запуск

### Windows (Visual Studio)

```bash
# Из корня проекта:
setup.bat
# Открыть nui.sln → nui-animations → Set as Startup Project → F5
```

### Linux / macOS (CMake)

```bash
# Из корня проекта:
chmod +x setup.sh && ./setup.sh
cmake --build build --config Release --target nui-animations
./build/bin/nui-animations
```

## API использованные в демо

```cpp
// Анимация позиции
Animator::AnimateX(widget, targetX, duration, EaseType::OutCubic);
Animator::AnimateY(widget, targetY, duration, EaseType::OutBounce);

// Анимация размера
Animator::AnimateWidth(widget, targetW, duration, EaseType::InOutQuad);

// Анимация прозрачности
Animator::AnimateAlpha(widget, 0.0f, duration, EaseType::InCubic);

// Анимация вращения
Animator::AnimateRotation(widget, 360.0f, duration, EaseType::Linear);

// Callback + loop
Animator::Animate(from, to, duration, ease)
    ->OnUpdate([](float v) { /* каждый кадр */ })
    .SetLoop(true)
    .SetPingPong(true)
    .SetDelay(0.5f);

// Последовательные анимации
auto t1 = Animator::AnimateY(widget, 300.0f, 0.5f, EaseType::OutCubic);
t1->OnComplete([]() {
    auto t2 = Animator::AnimateX(widget, 500.0f, 0.5f, EaseType::OutCubic);
    // ...
});
```

## Структура

```
examples/animations/
├── CMakeLists.txt              # CMake сборка
├── nui-animations.vcxproj      # VS проект
├── nui-animations.vcxproj.filters
├── main.cpp                    # Демо всех анимаций
├── animations.gif              # Гифка с демо
└── README.md
```
