# NUI Example: Hello World

Демонстрация всех поддерживаемых виджетов.

## Виджеты в примере

| Виджет | Где | Описание |
|--------|-----|----------|
| **Label** | Заголовок, подписи | Текст с выравниванием, word-wrap |
| **Button** | Say Hello, Play, Quit, Disabled | Hover/press состояния, onClick |
| **Image** | Правая колонка | PNG с режимом Fit |
| **EditBox** | Имя, пароль | Ввод текста, placeholder, password mode |
| **ProgressBar** | Синий, зелёный, жёлтый | Процент, кастомная метка |
| **ScrollView** | Список новостей | Прокрутка с scrollbar |
| **Widget** | Контейнеры, разделители | Фон, рамки, parent/child дерево |

## Сборка

```bash
cd nui/examples/app
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## Запуск

```bash
# Из папки build/bin (чтобы нашлись ресурсы)
./nui-example
```

Положите шрифт в `resources/fonts/` (например, `Roboto-Regular.ttf` или `DejaVuSans.ttf`).
Картинку для Image виджета — в `resources/images/cover.png`.

## Два режима работы

1. **XML layout** — если `resources/layouts/example.xml` существует, UI загружается из XML
2. **Programmatic** — если XML не найден, UI строится кодом (fallback)

## Структура

```
examples/app/
├── CMakeLists.txt   # Сборка с зависимостями
├── main.cpp         # Демо всех виджетов (программно + XML fallback)
├── layout.xml       # XML layout (скопировать в resources/layouts/)
└── README.md
```
