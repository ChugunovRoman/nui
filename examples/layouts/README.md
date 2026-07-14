# NUI Example: Adaptive Layouts (Anchor System)

Демонстрация системы адаптивных якорей (v0.4.0) — виджеты автоматически
адаптируются при изменении размера окна.

## Что демонстрируется

| # | Возможность | XML | C++ API |
|---|-------------|-----|---------|
| 1 | **Якоря к краям** (left/top/right/bottom) | `anchor="left top"` | `SetAnchor(AnchorFlag::Left \| AnchorFlag::Top)` |
| 2 | **Центрирование** | `anchor="center"` | `SetAnchor(AnchorFlag::None)` |
| 3 | **Растяжение между краями** (fill) | `anchor="left right"` | `SetAnchor(Left \| Right)` + `SetStretchW(Fill)` |
| 4 | **Режимы stretch** | `stretch_w="fill\|fixed\|proportional"` | `SetStretchW(StretchMode::...)` |
| 5 | **Нормализованные якоря** (Godot-style) | `anchor_left="0.2" anchor_right="0.82"` | `SetAnchor(0.2f, 0.82f, -1, -1)` |
| 6 | **Min / Max размер** | `min_width="100" max_width="300"` | `SetMinSize(100,0)` / `SetMaxSize(300, INT_MAX)` |
| 7 | **Смена якоря в рантайме** | — | кнопки TL/TR/BL/BR/Center/Fill |
| 8 | **Обратная совместимость** | виджеты без `anchor` | `SetRect(...)` как раньше |

## Структура демо

```
┌────────────────────────────────────────────────────────────┐
│  Header  (anchor: left top right, stretch_w: fill)         │
├──────────┬─────────────────────────────────┬───────────────┤
│          │  1. Edge Anchors                │               │
│  Sidebar │     ↖TL    TR↗                  │   Right bar   │
│  (left   │                                 │   (right      │
│   top    │  2. Center                      │    top        │
│   bottom)│     [center box]                │    bottom)    │
│          │                                 │               │
│          │  3. Stretch / Fill              │               │
│          │  4. Proportional                │               │
│          │  5. Min / Max                   │               │
│          │     ↙BL    BR↘                  │               │
│          │  6. [TL][TR][BL][BR][Center][F] │               │
├──────────┴─────────────────────────────────┴───────────────┤
│  Footer  (anchor: left right bottom, stretch_w: fill)      │
└────────────────────────────────────────────────────────────┘
```

- **Content** использует Godot-style нормализованные якоря
  (`anchor_left=0.2 anchor_right=0.82`), занимая пространство между сайдбарами.
- **Sidebar / Right bar** растягиваются по высоте (`left top bottom`).
- **Header / Footer** растягиваются по ширине.
- **Угловые блоки** (TL/TR/BL/BR) прижаты к углам content-области.
- **Кнопки** внизу content меняют якорь target-блока в рантайме.

## Два режима

1. **XML layout** — если `resources/layouts/layouts-demo.xml` существует, UI
   загружается из XML (демонстрирует XML-атрибуты якорей).
2. **Programmatic** — если XML не найден, UI строится кодом (демонстрирует
   C++ API `SetAnchor`, `SetStretch`, `SetMinSize`).

## Сборка

### Windows (Visual Studio)

```bash
# Из корня проекта:
setup.bat             # если ещё не запускали
nui.sln               # открыть в VS → выбрать nui-layouts → Release | x64 → Build
```

### Linux / macOS

```bash
# Из корня проекта:
chmod +x setup.sh && ./setup.sh   # если ещё не запускали
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc) --target nui-layouts
```

## Запуск

```bash
# Windows
build\bin\Release\x64\nui-layouts.exe

# Linux / macOS
./build/bin/nui-layouts
```

Положите шрифт в `resources/fonts/` (например, `Roboto-Regular.ttf` или
`DejaVuSans.ttf`).

## Что попробовать

1. **Ресайз окна** — все регионы (header, footer, sidebar, content) адаптируются.
2. **Кнопки TL/TR/BL/BR/Center/Fill** — меняют якорь синего target-блока.
3. **Min/Max блок** — растягивается по ширине, но не шире 300px и не уже 100px.
4. **Proportional блок** — масштабируется пропорционально размеру content.

## Структура

```
examples/layouts/
├── CMakeLists.txt              # Сборка с зависимостями (Linux/macOS)
├── main.cpp                    # Демо якорей (программно + XML fallback)
├── layouts-demo.xml            # XML layout с anchor/stretch атрибутами
├── nui-layouts.vcxproj         # Проект Visual Studio
├── nui-layouts.vcxproj.filters # Фильтры VS
├── nui-layouts.vcxproj.user    # Настройки пользователя VS
└── README.md
```
