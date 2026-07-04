# Async API

Асинхронная система позволяет запускать тяжёлые задачи в фоне, не блокируя UI.

## Архитектура

```
Async::Run(func)  ─────►  ThreadPool (N-1 воркеров)
        │                        │
        ▼                        ▼
  Future<T>               Задача выполняется
        │                        │
        ▼                        │
  ->Then(callback)  ◄── результат ┘
        │
        ▼
  Main thread queue
        │
        ▼
  Application::Run() — drain каждый кадр
```

## ThreadPool

- Автоматически создаёт `N-1` воркеров (где N = `hardware_concurrency()`)
- Задачи выполняются параллельно на фоновых потоках
- Один экземпляр на всё приложение (синглтон)

## Async::Run()

Запускает функцию на фоновом потоке. Возвращает `Future<T>`.

```cpp
#include "core/async.h"

// Простой вызов
nui::Async::Run([]() {
    // Выполняется на воркере
    return 42;
})->Then([](int result) {
    // Выполняется на main thread
    NUI_LOG("Result: %d\n", result);
});

// Без возврата значения
nui::Async::Run([]() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    // ...
})->Then([]() {
    NUI_LOG("Done!\n");
});
```

## Future<T>

Результат асинхронной операции.

### Методы

| Метод | Описание |
|-------|----------|
| `Then(callback)` | Callback на main thread когда результат готов |
| `IsReady()` | Проверить готовность |
| `Get()` | Блокирующее ожидание результата |

### Примеры

```cpp
// With callback (non-blocking)
auto future = nui::Async::Run([]() -> std::string {
    return loadData();
});

future->Then([](const std::string& data) {
    label->SetText(data);
});

// Blocking wait (не рекомендуется в UI потоке!)
auto future = nui::Async::Run([]() {
    return heavyComputation();
});
int result = future->Get(); // Блокирует直到完成
```

## Application::DispatchOnMainThread()

Планирует callback на выполнение в main thread (следующий кадр).

```cpp
// Из любого потока
nui::Application::DispatchOnMainThread([]() {
    // Выполнится на main thread
    progressBar->SetValue(1.0f);
});

// Или через Async
nui::Async::DispatchOnMainThread([]() {
    // Тоже на main thread
});
```

## Типичные паттерны

### Загрузка данных с прогрессом

```cpp
// Кнопка загрузки
btnLoad->SetOnClick([](Widget*) {
    statusLabel->SetText("Loading...");

    nui::Async::Run([]() -> std::string {
        // Тяжёлая загрузка (5 секунд)
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return "Data loaded successfully!";
    })->Then([](const std::string& result) {
        statusLabel->SetText(result);
    });
});
```

### Прогресс-бар с анимацией

```cpp
// Запуск фоновой задачи
auto future = nui::Async::Run([&progress]() -> bool {
    for (int i = 0; i <= 100; ++i) {
        // Обновление прогресса через main thread
        float val = i / 100.0f;
        nui::Application::DispatchOnMainThread([val, &progress]() {
            progress->SetValue(val);
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return true;
});

future->Then([](bool ok) {
    NUI_LOG("Task completed: %s\n", ok ? "success" : "failed");
});
```

### Сетевой запрос (пример)

```cpp
btnRefresh->SetOnClick([](Widget*) {
    statusLabel->SetText("Fetching data...");

    nui::Async::Run([]() -> std::string {
        // curl / httplib / etc.
        return httpGet("https://api.example.com/data");
    })->Then([](const std::string& response) {
        // Парсинг и обновление UI
        auto data = parseJson(response);
        updateUI(data);
    });
});
```

### Параллельные задачи

```cpp
// Запуск нескольких задач параллельно
auto task1 = nui::Async::Run([]() { return loadTexture("bg.png"); });
auto task2 = nui::Async::Run([]() { return loadTexture("logo.png"); });
auto task3 = nui::Async::Run([]() { return loadFont("arial.ttf"); });

// Каждый результат приходит отдельно
task1->Then([](auto tex) { background->SetTexture(tex); });
task2->Then([](auto tex) { logo->SetTexture(tex); });
task3->Then([](auto font) { title->SetFont(font); });
```

## Важные правила

1. **Не изменяйте UI из фонового потока** — все изменения UI только через `Then()` или `DispatchOnMainThread()`
2. **Не вызывайте `Get()` в main thread** — это заблокирует UI
3. **Future живёт пока есть ссылка** — сохраняйте `shared_ptr<Future>` если нужен long-lived callback
4. **ThreadPool создаётся один раз** — при первом вызове `Async::Run()`
