# NUI Documentation

Лёгкий кроссплатформенный UI toolkit для десктоп-приложений.

## Содержание

1. [Быстрый старт](getting-started.md) — установка, сборка, первое приложение
2. [Виджеты](widgets.md) — все доступные виджеты и их свойства
3. [XML Layout](xml-layout.md) — описание UI в XML файлах
4. [Async API](async.md) — асинхронные задачи без блокировки UI
5. [Ресурсы](resources.md) — управление ресурсами и вшивание в бинарник
6. [API Reference](api.md) — классы, методы, структуры

## Краткий обзор

```
SDL2 (окно + ввод + CPU software renderer)
  └─ Canvas (2D рендеринг на CPU)
  └─ UI Kit (виджеты)
       ├─ Widget → Label, Button, Image, EditBox, ProgressBar, ScrollView
       └─ LayoutLoader (XML → дерево виджетов)
  └─ Async (ThreadPool + Future + main thread dispatch)
  └─ ResourceManager (embedded + filesystem)
```

## Особенности

- **Только CPU** — не требует GPU или драйверов
- **Один бинарник** — ресурсы вшиваются в exe
- **Кроссплатформенность** — Windows, Linux, macOS
- **~3-5 МБ** — итоговый размер бинарника
- **XML layouts** — описание UI декларативно
- **Async** — фоновые задачи без заморозки UI
- **UTF-8** — поддержка кириллицы и мультибайт символов
