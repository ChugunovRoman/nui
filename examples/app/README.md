# NUI Example: Hello World

Демонстрация всех поддерживаемых виджетов.

## Виджеты в примере

| Виджет | Где | Описание |
|--------|-----|----------|
| **Label** | Заголовок, подписи | Текст с выравниванием, word-wrap |
| **Button** | Say Hello, Play, Quit, Disabled, Async | Hover/press состояния, onClick, tooltip |
| **Image** | Правая колонка | PNG с режимом Fit |
| **EditBox** | Имя, пароль | Ввод текста (UTF-8), placeholder, password |
| **ProgressBar** | Синий, зелёный, жёлтый | Процент, кастомная метка |
| **ScrollView** | Список новостей | Прокрутка с scrollbar, clipping |
| **Slider / CheckBox / RadioButton** | Левая колонка | Ползунок, чекбоксы, группа радио |
| **Dropdown** | Список серверов | Выпадающий список |
| **TabControl** | Правая колонка | Вкладки General/Graphics/About |
| **Treeview** | Правая колонка | Иерархическое дерево файлов |
| **Tooltip** | Наведите на кнопки | Всплывающие подсказки (встроены в Application) |
| **Menu / ContextMenu** | Кнопки Menu / Ctx Menu | Всплывающее меню с подменю |
| **Dialog / MessageBox** | Кнопки Dialog / MessageBox | Модальные окна |
| **Widget** | Контейнеры, разделители | Фон, рамки, parent/child дерево |
| **Async** | Кнопка Async Task | Фоновая задача без блокировки UI |

## Сборка

### Windows

```bash
# Из корня проекта:
setup.bat             # если ещё не запускали
nui.sln               # открыть в VS → Release | x64 → Build
```

### Linux / macOS

```bash
# Из корня проекта:
chmod +x setup.sh && ./setup.sh   # если ещё не запускали
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## Запуск

```bash
# Windows
build\bin\Release\nui-example.exe

# Linux / macOS
./build/bin/nui-example
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
├── main.cpp         # Демо всех виджетов + async (программно + XML fallback)
├── example.xml      # XML layout (копируется в OutDir/resources/layouts/)
└── README.md
```
