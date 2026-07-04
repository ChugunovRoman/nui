# Управление ресурсами

## ResourceManager

Единая точка доступа к ресурсам (картинки, XML, шрифты). Автоматически ищет сначала в embedded, потом на диске.

```cpp
#include "renderer/resource.h"

// Инициализация (вызвать один раз при старте)
nui::ResourceManager::Initialize();

// Загрузка ресурса
nui::ResourceData data = nui::ResourceManager::Load("resources/images/logo.png");
if (data.valid()) {
    // data.data — указатель на байты
    // data.size — размер в байтах
}

// Проверка существования
bool exists = nui::ResourceManager::Exists("resources/layouts/main.xml");
```

### Порядок поиска

1. **Embedded** — вшитые в бинарник ресурсы (Release сборка)
2. **Filesystem** — файлы на диске (Debug сборка, fallback)

## Вшивание ресурсов в бинарник

### Как работает

```
resources/                  tools/embed_resources.py           binary
  ├── layouts/main.xml    ──────────────────────────►     embedded byte arrays
  ├── images/cover.png                                    + lookup registry
  └── fonts/Roboto.ttf
```

1. Скрипт `tools/embed_resources.py` конвертирует файлы в C++ массивы
2. Генерируется `src/generated/embedded_resources.cpp` с byte arrays
3. Компилируется вместе с проектом
4. Ресурсы доступны через `ResourceManager::Load()`

### Автоматический запуск

При Release сборке ресурсы вшиваются автоматически:
- **CMake**: `execute_process()` при configure
- **VS (CMake-generated)**: встроен в CMake
- **VS (manual .vcxproj)**: pre-build event

### Ручной запуск

```bash
python tools/embed_resources.py <папка_ресурсов> <папка_вывода> [префикс]

# Пример:
python tools/embed_resources.py resources/ src/generated/ resources
```

Аргументы:

| Аргумент | Описание |
|----------|----------|
| `<папка_ресурсов>` | Исходная папка с ресурсами |
| `<папка_вывода>` | Куда сгенерировать .cpp/.h |
| `[префикс]` | Префикс путей (опционально) |

### С Generated файлом

`src/generated/embedded_resources.cpp` содержит:

```cpp
// AUTO-GENERATED — DO NOT EDIT

static const uint8_t res_resources_images_cover_png_data[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, // ...
};

static std::unordered_map<std::string_view, EmbeddedResource> s_resources;

void RegisterEmbeddedResources() {
    s_resources["resources/images/cover.png"] = {
        res_resources_images_cover_png_data, 39441
    };
    // ...
}
```

### .gitignore

Generated файлы не коммитятся:

```
# .gitignore
src/generated/
```

## TextureCache

Кеширует загруженные текстуры чтобы не загружать повторно.

```cpp
nui::TextureCache cache;

// Загрузка (кешируется автоматически)
nui::Texture* tex = cache.Get("resources/images/logo.png");
if (tex) {
    canvas.DrawTexture(*tex, destRect);
}

// Повторный вызов возвращает закешированный результат
nui::Texture* same = cache.Get("resources/images/logo.png"); // не перезагружает

// Очистка кеша
cache.Clear();
```

### Поддерживаемые форматы

| Формат | Расширения |
|--------|-----------|
| PNG | `.png` |
| JPEG | `.jpg`, `.jpeg` |
| BMP | `.bmp` |
| TGA | `.tga` |
| GIF | `.gif` |

## FontManager

Кеширует загруженные шрифты.

```cpp
nui::FontManager fonts;
fonts.Initialize();

// Загрузка шрифта
nui::Font* font = fonts.Get("resources/fonts/Roboto.ttf", 18);
if (font) {
    canvas.DrawText(*font, "Hello", x, y, Color::White());
}

// Дефолтный шрифт
nui::Font* defaultFont = fonts.GetDefault(16);

// Повторный вызов с теми же параметрами возвращает закешированный шрифт
nui::Font* same = fonts.Get("resources/fonts/Roboto.ttf", 18); // не перезагружает
```

## XML Layout из ресурсов

LayoutLoader автоматически использует ResourceManager:

```cpp
nui::LayoutLoader loader;

// Ищет сначала embedded, потом на диске
auto root = loader.LoadFromFile("resources/layouts/main.xml",
                                 textures, fonts);
```

## Рекомендации по структуре ресурсов

```
resources/
├── layouts/
│   ├── main.xml              ← Главный layout
│   ├── settings.xml          ← Экран настроек
│   └── colors.xml            ← Цветовые определения
├── images/
│   ├── background.jpg        ← Фон
│   ├── logo.png              ← Логотип
│   ├── button_normal.png     ← Текстуры кнопок
│   ├── button_hover.png
│   └── button_pressed.png
└── fonts/
    ├── Roboto-Regular.ttf    ← Основной шрифт
    └── Roboto-Bold.ttf       ← Жирный шрифт
```

## Вес ресурсов

При вшивании ресурсов в бинарник учитывайте размер:

| Тип | Типичный размер |
|-----|----------------|
| XML layout | 1-10 КБ |
| PNG картинка | 10-500 КБ |
| JPG картинка | 50-2000 КБ |
| TTF шрифт | 100-500 КБ |

Рекомендация: используйте JPG для больших изображений, PNG для маленьких с прозрачностью.
