# NUI

Лёгкий кроссплатформенный UI toolkit для десктоп-приложений, вдохновлённый архитектурой xrUICore.

## Особенности

- **Только CPU рендеринг** — не требует GPU, дискретной видеокарты или драйверов
- **Один бинарник** — статическая линковка всех зависимостей
- **Кроссплатформенность** — Windows, Linux, macOS
- **Маленький размер** — ~3-5 МБ итоговый бинарник
- **XML layouts** — описание UI в XML файлах (аналогично xrUICore)
- **Поддержка изображений** — PNG, JPG, BMP, TGA, GIF через stb_image

## Архитектура

```
SDL2 (окно + ввод + software renderer)
  └─ Canvas (CPU 2D рендеринг)
       ├─ FillRect, DrawRect, DrawLine, DrawPixel
       ├─ DrawTexture (stretch, fit, fill, center, tile)
       └─ DrawText, DrawTextWrapped (SDL_ttf / FreeType)
  └─ UI Kit (виджеты)
       ├─ Widget (базовый класс, как CUIWindow)
       ├─ Label, Button, Image
       ├─ EditBox, ProgressBar, ScrollView
       └─ LayoutLoader (XML → дерево виджетов)
```

## Зависимости

| Библиотека | Назначение | Размер |
|------------|-----------|--------|
| SDL2 2.30 | Окно, ввод, software renderer | ~1.5 МБ |
| SDL_ttf 2.22 | Рендеринг шрифтов (FreeType) | ~0.5 МБ |
| pugixml 1.14 | XML парсер | ~0.1 МБ |
| stb_image | Загрузка картинок (header-only) | ~0 МБ |

## Сборка

### Способ 1: Visual Studio (рекомендуется для Windows)

```bash
# 1. Запустить setup.bat из корня проекта (скачает vcpkg + зависимости + stb_image)
setup.bat

# 2. Открыть nui.sln в Visual Studio 2019/2022
# 3. Выбрать конфигурацию Release | x64
# 4. Build → Build Solution (Ctrl+Shift+B)
```

Проект включает:
- `nui` — статическая библиотека (.lib)
- `example-app` — пример-приложение (ссылается на nui.lib)

Зависимости (SDL2, SDL_ttf, pugixml) устанавливаются через vcpkg автоматически.

### Способ 2: CMake

#### Требования
- CMake 3.16+
- C++17 компилятор (MSVC 2019+, GCC 9+, Clang 10+)
- Git (для автоматического скачивания зависимостей)

#### Windows (MSVC)
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

#### Linux
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

#### macOS
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(sysctl -n hw.ncpu)
```

## Использование в вашем проекте

### Структура проекта на NUI

```
my-launcher/
├── nui/                        ← библиотека (git submodule или копия)
├── resources/
│   ├── layouts/
│   │   └── main.xml            ← UI в XML
│   ├── images/
│   │   ├── logo.png
│   │   └── background.jpg
│   └── fonts/
│       └── Roboto-Regular.ttf
├── src/
│   └── main.cpp                ← логика приложения
├── CMakeLists.txt
└── vcpkg.json
```

### CMakeLists.txt приложения

```cmake
cmake_minimum_required(VERSION 3.16)
project(my-launcher LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(nui)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE nui)
```

### Пример main.cpp

```cpp
#include "core/application.h"
#include "renderer/resource.h"
#include "xml/layout_loader.h"
#include "ui/button.h"

int main() {
    nui::ResourceManager::Initialize();

    nui::Application app;
    app.Initialize({"My Launcher", 1024, 768});

    nui::TextureCache textures;
    nui::LayoutLoader loader;
    nui::FontManager fonts;
    fonts.Initialize();

    // Загрузка UI из XML (ресурсы ищутся сначала в embedded, потом на диске)
    auto root = loader.LoadFromFile("resources/layouts/main.xml", textures, fonts);

    // Обработка событий
    root->GetChild("btn_play")->SetOnClick([](nui::Widget*) {
        nui::Log("Launching game...\n");
    });

    app.SetRoot(std::move(root));
    return app.Run();
}
```

### Пример layout.xml

```xml
<layout width="1024" height="768" bg_color="20,20,30">
    <image x="0" y="0" width="1024" height="400"
           src="resources/images/background.jpg" scale="fill"/>
    <label x="400" y="420" width="224" height="40"
           text="My Game" font_size="32" text_color="220,220,255" align_h="center"/>
    <button name="btn_play" x="400" y="500" width="224" height="60"
            text="PLAY" font_size="24"
            bg_color="40,160,60" hover_color="50,190,70" pressed_color="30,130,45"/>
    <progressbar x="400" y="580" width="224" height="20"
                 value="0.75" show_percent="true" fill_color="40,120,200"/>
</layout>
```

### Сборка и релиз

```bash
# Debug — ресурсы читаются с диска
cmake --build . --config Debug

# Release — ресурсы вшиваются в exe (pre-build автоматически запускает embed_resources.py)
cmake --build . --config Release
```

На выходе — **один exe без внешних зависимостей**. Работает без GPU.

## Структура проекта

```
nui/
├── CMakeLists.txt              # Сборка с FetchContent
├── nui.sln               # Visual Studio Solution
├── nui.vcxproj           # VS: статическая библиотека
├── nui.vcxproj.filters
├── vcpkg.json                  # Манифест зависимостей (vcpkg)
├── setup.bat                   # Установка vcpkg + зависимости
├── README.md
├── .gitignore
├── examples/
│   └── app/
│       ├── CMakeLists.txt      # CMake для примера
│       ├── example-app.vcxproj # VS: пример-приложение
│       ├── example-app.vcxproj.filters
│       ├── main.cpp            # Hello World со всеми виджетами
│       └── layout.xml          # XML layout для примера
├── src/
│   ├── main.cpp                # Точка входа, демо-интерфейс
│   ├── core/
│   │   ├── application.h/cpp   # Главное окно, event loop
│   │   ├── input.h/cpp         # Состояние мыши/клавиатуры
│   │   └── log.h               # Логирование (stdout + OutputDebugString)
│   ├── renderer/
│   │   ├── canvas.h/cpp        # CPU 2D рендеринг
│   │   ├── color.h             # RGBA цвет
│   │   ├── texture.h/cpp       # Загрузка изображений (stb_image)
│   │   ├── font.h/cpp          # Рендеринг шрифтов (SDL_ttf)
│   │   └── resource.h/cpp      # ResourceManager (embedded + filesystem)
│   ├── ui/
│   │   ├── widget.h/cpp        # Базовый виджет (как CUIWindow)
│   │   ├── label.h/cpp         # Текстовая метка
│   │   ├── button.h/cpp        # Кнопка с hover/press состояниями
│   │   ├── image.h/cpp         # Отображение картинок
│   │   ├── editbox.h/cpp       # Поле ввода текста
│   │   ├── progressbar.h/cpp   # Индикатор прогресса
│   │   └── scrollview.h/cpp    # Прокручиваемый контейнер
│   ├── xml/
│   │   └── layout_loader.h/cpp # XML → дерево виджетов
│   └── generated/
│       └── embedded_resources.* # Автогенерированные embedded ресурсы
├── tools/
│   └── embed_resources.py      # Скрипт для вшивания ресурсов в бинарник
└── resources/
    ├── fonts/                  # Шрифты (TTF/OTF)
    ├── images/                 # Картинки для UI
    └── layouts/                # XML файлы разметки
```

## XML Layout

UI описывается в XML файлах, аналогично xrUICore:

```xml
<layout name="launcher" width="1024" height="768" bg_color="20,20,30">
    <panel name="header" x="0" y="0" width="1024" height="60" bg_color="35,35,50">
        <label text="Hello" font_size="24" text_color="200,200,255"/>
    </panel>
    <button name="btn_play" x="20" y="20" width="200" height="50"
            text="PLAY" bg_color="40,120,40"/>
    <image name="cover" x="20" y="100" width="400" height="250"
           src="resources/images/cover.png" scale="fit"/>
    <progressbar x="20" y="370" width="200" height="25"
                 value="0.75" show_percent="true"/>
    <editbox x="20" y="410" width="200" height="32"
             placeholder="Search..."/>
</layout>
```

### Поддерживаемые виджеты

| XML тег | Виджет | Описание |
|---------|--------|----------|
| `<layout>`, `<panel>` | Widget | Контейнер |
| `<label>`, `<text>` | Label | Текстовая метка |
| `<button>` | Button | Кнопка |
| `<image>`, `<sprite>` | Image | Картинка |
| `<editbox>`, `<input>` | EditBox | Поле ввода |
| `<progressbar>`, `<progress>` | ProgressBar | Индикатор прогресса |
| `<scrollview>`, `<scroll>` | ScrollView | Прокручиваемый контейнер |

### Общие свойства

Все виджеты поддерживают:
- `name`, `x`, `y`, `width`, `height`
- `visible`, `enabled`
- `bg_color`, `border_color`, `color`
- `align_h` (left/center/right), `align_v` (top/center/bottom)

## Вшивание ресурсов в бинарник

Ресурсы (картинки, XML, шрифты) можно вшить прямо в exe:

```
resources/                  embed_resources.py           binary
  ├── layouts/main.xml    ──────────────────────►   embedded byte arrays
  ├── images/cover.png                              + lookup registry
  └── fonts/Roboto.ttf
```

- На **pre-build** этапе скрипт `tools/embed_resources.py` конвертирует файлы в C++ массивы
- `ResourceManager::Load(path)` ищет сначала в embedded, потом на диске
- Texture, Font, LayoutLoader автоматически используют ResourceManager
- Если ресурс не найден ни там, ни там — fallback на файловую систему

```bash
# Ручной запуск (обычно автоматически при сборке)
python tools/embed_resources.py <папка_ресурсов> <папка_вывода> [префикс]
python tools/embed_resources.py resources/ src/generated/ resources
```

## Программное создание UI

```cpp
using namespace nui;

auto btn = std::make_unique<Button>();
btn->SetRect(20, 20, 200, 50);
btn->SetText("Click Me");
btn->SetOnClick([](Widget* w) {
    printf("Clicked!\n");
});
root->AddChild(std::move(btn));
```

## Сравнение с другими решениями

| Решение | Размер | CPU-only | Один бинарник | Cross-platform |
|---------|--------|----------|---------------|----------------|
| **NUI** | ~3-5 МБ | ✅ | ✅ | ✅ |
| Electron | ~150 МБ | ❌ | ❌ | ✅ |
| Tauri | ~5 МБ + WebView | ❌ | ❌ | ✅ |
| Qt | ~30-50 МБ | ❌ | ❌ | ✅ |
| FLTK | ~1-3 МБ | ✅ | ✅ | ✅ |
| Dear ImGui + SW | ~1-3 МБ | ✅ | ✅ | ✅ |
