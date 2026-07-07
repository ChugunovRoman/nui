# Animations

Система анимаций NUI позволяет плавно изменять свойства виджетов во времени с использованием easing функций.

## Архитектура

```
Application::Run()
  └─ Animator::UpdateAll(dt)  ← вызывается каждый кадр автоматически
       └─ Tween::Update(dt)   ← обновляет значение по easing кривой
            └─ callback       ← применяет значение к виджету
```

- **Easing** — математическая функция, определяющая форму кривой анимации
- **Tween** — анимация одного float значения от A до B
- **Animator** — менеджер, создаёт и обновляет твины

## Animator

Статический класс — глобальный менеджер анимаций.

### Создание анимаций

```cpp
#include "animation/animator.h"

// Анимация float значения
float myValue = 0.0f;
Animator::Animate(&myValue, 0.0f, 1.0f, 0.5f, EaseType::OutCubic);

// Анимация через callback (без целевого указателя)
Animator::Animate(0.0f, 100.0f, 1.0f, EaseType::Linear)
    ->OnUpdate([](float v) {
        NUI_LOG("Value: %.1f\n", v);
    });
```

### Анимация свойств виджетов

```cpp
// Позиция X
Animator::AnimateX(button, 300.0f, 0.3f, EaseType::OutCubic);

// Позиция Y
Animator::AnimateY(button, 100.0f, 0.3f, EaseType::OutCubic);

// Ширина
Animator::AnimateWidth(panel, 400.0f, 0.5f, EaseType::InOutQuad);

// Высота
Animator::AnimateHeight(panel, 300.0f, 0.5f, EaseType::InOutQuad);

// Прозрачность (0.0 = прозрачный, 1.0 = непрозрачный)
Animator::AnimateAlpha(label, 0.0f, 1.0f, EaseType::Linear);
```

### Управление

```cpp
// Отмена по тегу
Animator::CancelByTag("my_animation");

// Отмена всех анимаций
Animator::CancelAll();

// Количество активных анимаций
size_t count = Animator::GetActiveCount();
```

## Tween

Отдельная анимация. Возвращается методами `Animator::Animate*()`.

### Цепочка настроек (builder pattern)

```cpp
Animator::Animate(0.0f, 1.0f, 2.0f, EaseType::OutCubic)
    ->OnUpdate([](float v) { /* каждый кадр */ })
    ->OnComplete([]() { /* по завершении */ })
    ->SetLoop(true)           // зацикливание
    ->SetPingPong(true)       // туда-обратно
    ->SetDelay(0.5f)          // задержка перед стартом (секунды)
    ->SetTag("my_anim");      // тег для отмены
```

### Управление отдельным твином

```cpp
Tween* t = Animator::AnimateX(widget, 500.0f, 1.0f);

t->Pause();     // пауза
t->Resume();    // продолжить
t->Restart();   // начать сначала
t->Cancel();    // отменить

bool active = t->IsActive();
float progress = t->GetProgress(); // 0.0 - 1.0
```

## Easing Functions

Определяют форму кривой анимации. Все функции принимают `t` в [0, 1] и возвращают значение в [0, 1].

### Линейная

| Функция | Описание |
|---------|----------|
| `Linear` | Постоянная скорость |

### Квадратичная

| Функция | Описание |
|---------|----------|
| `InQuad` | Медленный старт, ускорение |
| `OutQuad` | Быстрый старт, замедление |
| `InOutQuad` | Медленный старт и конец |

### Кубическая

| Функция | Описание |
|---------|----------|
| `InCubic` | Более выраженное ускорение |
| `OutCubic` | Более выраженное замедление |
| `InOutCubic` | Плавный старт и конец |

### Квартичная / Квинтичная

| Функция | Описание |
|---------|----------|
| `InQuart` / `OutQuart` / `InOutQuart` | Ещё более выраженный эффект |
| `InQuint` / `OutQuint` / `InOutQuint` | Максимальный эффект |

### Тригонометрическая

| Функция | Описание |
|---------|----------|
| `InSine` | Плавный старт (синусоида) |
| `OutSine` | Плавный конец |
| `InOutSine` | Плавный старт и конец |

### Экспоненциальная

| Функция | Описание |
|---------|----------|
| `InExpo` | Резкое ускорение |
| `OutExpo` | Резкое замедление |
| `InOutExpo` | Резкий старт и конец |

### Круговая

| Функция | Описание |
|---------|----------|
| `InCirc` | Ускорение по дуге |
| `OutCirc` | Замедление по дуге |
| `InOutCirc` | Плавная дуга |

### Back (с перелётом)

| Функция | Описание |
|---------|----------|
| `InBack` | Оттягивание назад, затем рывок |
| `OutBack` | Перелёт за цель, возврат |
| `InOutBack` | Оттягивание + перелёт |

### Elastic (пружина)

| Функция | Описание |
|---------|----------|
| `InElastic` | Пружинистый старт |
| `OutElastic` | Пружинистый конец |
| `InOutElastic` | Пружинистый старт и конец |

### Bounce (отскок)

| Функция | Описание |
|---------|----------|
| `InBounce` | Отскок в начале |
| `OutBounce` | Отскок в конце |
| `InOutBounce` | Отскок в начале и конце |

## Примеры

### Кнопка с анимацией при наведении

```cpp
auto btn = std::make_unique<Button>();
btn->SetRect(100, 100, 200, 50);
btn->SetText("Hover me");
Widget* btnPtr = btn.get();

btn->SetOnClick([btnPtr](Widget*) {
    // Анимация "bounce" при клике
    Animator::AnimateY(btnPtr, btnPtr->GetY() - 20.0f, 0.15f, EaseType::OutQuad)
        ->OnComplete([btnPtr]() {
            Animator::AnimateY(btnPtr, btnPtr->GetY() + 20.0f, 0.3f, EaseType::OutBounce);
        });
});
```

### Плавное появление элемента

```cpp
auto label = std::make_unique<Label>();
label->SetText("Fade in!");
label->SetColor(Color(255, 255, 255, 0)); // невидимый
Widget* lblPtr = label.get();

// Появление через 0.5 секунды
Animator::AnimateAlpha(lblPtr, 1.0f, 0.5f, EaseType::OutCubic)
    .SetDelay(0.5f);
```

### Зацикленная анимация загрузки

```cpp
auto spinner = std::make_unique<Widget>();
spinner->SetRect(100, 100, 30, 30);
spinner->SetBgColor(Color(80, 160, 255));
Widget* spinPtr = spinner.get();

// Пульсация
Animator::Animate(30.0f, 50.0f, 0.6f, EaseType::InOutSine)
    ->OnUpdate([spinPtr](float v) {
        int s = static_cast<int>(v);
        spinPtr->SetSize(s, s);
    })
    .SetLoop(true).SetPingPong(true);
```

### Слайдер-шоу (последовательные анимации)

```cpp
auto box = std::make_unique<Widget>();
box->SetRect(0, 100, 100, 100);
Widget* boxPtr = box.get();

// Шаг 1: двигаем вправо
auto t1 = Animator::AnimateX(boxPtr, 500.0f, 1.0f, EaseType::InOutCubic);
t1->OnComplete([boxPtr]() {
    // Шаг 2: двигаем вниз
    auto t2 = Animator::AnimateY(boxPtr, 400.0f, 1.0f, EaseType::InOutCubic);
    t2->OnComplete([boxPtr]() {
        // Шаг 3: возвращаемся
        Animator::AnimateX(boxPtr, 0.0f, 1.0f, EaseType::OutBack);
        Animator::AnimateY(boxPtr, 100.0f, 1.0f, EaseType::OutBack);
    });
});
```

### Стагер (задержка между элементами)

```cpp
for (int i = 0; i < 10; ++i) {
    auto item = std::make_unique<Widget>();
    item->SetRect(20 + i * 50, -50, 40, 40);
    Widget* ptr = item.get();
    root->AddChild(std::move(item));

    // Каждый элемент появляется с задержкой
    Animator::AnimateY(ptr, 200.0f, 0.5f, EaseType::OutBack)
        .SetDelay(static_cast<float>(i) * 0.1f);
}
```

## Демонстрация

Запустите `nui-animations` для визуальной демонстрации всех easing функций и типов анимаций.

```bash
# Windows
build\bin\Release\nui-animations.exe

# Linux/macOS
./build/bin/nui-animations
```
