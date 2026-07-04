# Быстрый старт

## Требования

| Инструмент | Версия | Назначение |
|------------|--------|-----------|
| Git | любая | Клонирование с submodule'ами |
| CMake | 3.16+ | Сборка зависимостей и проекта |
| Visual Studio | 2019/2022 | Компилятор (Windows) |
| GCC | 9+ | Компилятор (Linux) |
| Clang | 10+ | Компилятор (macOS) |

## Установка

### 1. Клонировать репозиторий

```bash
git clone --recursive https://github.com/your-org/nui.git
cd nui
```

Или если уже клонировали:

```bash
git submodule update --init --recursive
```

### 2. Первичная настройка (один раз)

| ОС | Команда |
|----|---------|
| **Windows** | `setup.bat` |
| **Linux / macOS** | `chmod +x setup.sh && ./setup.sh` |

Скрипт:
- Инициализирует git submodules (SDL2, SDL_ttf, pugixml)
- Переключает на теги релизов
- Скачивает stb_image.h
- Генерирует `build/nui.sln` (Windows) или build файлы (Linux/macOS)

### 3. Сборка

**Windows (Visual Studio) — рекомендуется:**
1. Открыть `nui.sln` в корне проекта
2. Выбрать **Release | x64**
3. **Build → Build Solution** (Ctrl+Shift+B)

Все зависимости (SDL2, SDL_ttf, pugixml) собираются автоматически через .vcxproj в Externals/.

**Windows (CMake):**
```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

> **Важно:** Всегда указывайте `-G "Visual Studio 17 2022"` — без этого CMake может выбрать GCC из Strawberry Perl.

**Linux / macOS (CMake):**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

> **Windows:** Всегда указывайте `-G "Visual Studio 17 2022"` — без этого CMake может выбрать GCC из Strawberry Perl.

### 4. Запуск

```bash
# Windows
build\bin\Release\nui-example.exe

# Linux/macOS
./build/bin/nui-example
```

## Первое приложение

### main.cpp

```cpp
#include "core/application.h"
#include "renderer/resource.h"
#include "xml/layout_loader.h"
#include "ui/button.h"

int main() {
    // 1. Инициализировать ResourceManager
    nui::ResourceManager::Initialize();

    // 2. Создать приложение
    nui::Application app;
    app.Initialize({"My App", 800, 600});

    // 3. Загрузить UI из XML
    nui::TextureCache textures;
    nui::LayoutLoader loader;
    nui::FontManager fonts;
    fonts.Initialize();

    auto root = loader.LoadFromFile("resources/layouts/main.xml",
                                     textures, fonts);
    if (!root) {
        // Fallback: программное создание UI
        root = std::make_unique<nui::Widget>();
        root->SetRect(0, 0, 800, 600);
        root->SetBgColor(nui::Color(20, 20, 30));

        auto btn = std::make_unique<nui::Button>();
        btn->SetRect(300, 250, 200, 50);
        btn->SetText("Click Me");
        btn->SetOnClick([](nui::Widget*) {
            nui::NUI_LOG("Clicked!\n");
        });
        root->AddChild(std::move(btn));
    }

    // 4. Запустить
    app.SetRoot(std::move(root));
    return app.Run();
}
```

### layout.xml

```xml
<layout width="800" height="600" bg_color="20,20,30">
    <label x="250" y="150" width="300" height="40"
           text="Hello NUI!" font_size="28"
           text_color="200,200,255" align_h="center"/>
    <button name="btn" x="300" y="250" width="200" height="50"
            text="Click Me" font_size="18"
            bg_color="40,120,200" hover_color="55,140,230"/>
</layout>
```

## Структура проекта приложения

```
my-app/
├── nui/                        ← UI toolkit (submodule)
├── resources/
│   ├── layouts/main.xml        ← UI в XML
│   ├── images/logo.png         ← Картинки
│   └── fonts/Roboto.ttf        ← Шрифты
├── src/main.cpp                ← Логика приложения
└── CMakeLists.txt
```

### CMakeLists.txt приложения

```cmake
cmake_minimum_required(VERSION 3.16)
project(my-app LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(nui)

add_executable(${PROJECT_NAME} WIN32 src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE nui)
```

## Шрифты

NUI ищет шрифты в следующем порядке:

1. `resources/fonts/` — рядом с exe
2. Системные шрифты:
   - Windows: `C:/Windows/Fonts/arial.ttf`
   - Linux: `/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf`
   - macOS: `/System/Library/Fonts/Helvetica.ttc`

Для портативности положите `.ttf` файл в `resources/fonts/`.
