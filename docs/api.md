# API Reference

## Application

Главное окно и event loop.

```cpp
#include "core/application.h"

nui::Application app;
nui::AppDesc desc;
desc.title = "My App";
desc.width = 1024;
desc.height = 768;
desc.resizable = true;

app.Initialize(desc);
app.SetRoot(std::move(root));
int exitCode = app.Run();
app.Shutdown();
```

### AppDesc

| Поле | Тип | По умолчанию | Описание |
|------|-----|-------------|----------|
| title | string | "NUI" | Заголовок окна |
| iconPath | string | "" | Путь к PNG/BMP иконке (окно + таскбар) |
| width | int | 1024 | Ширина |
| height | int | 768 | Высота |
| resizable | bool | true | Изменяемый размер |
| borderless | bool | false | Без системной рамки (кастомный titlebar) |
| resizeBorderWidth | int | 5 | Зона захвата краёв для resize (px, DPI-scaled) |
| minWindowWidth | int | 100 | Минимальная ширина окна при resize |
| minWindowHeight | int | 100 | Минимальная высота окна при resize |
| titlebarButtonSize | int | 28 | Размер кнопок min/max/close (px, DPI-scaled) |
| doubleClickMs | int | 500 | Окно двойного клика для maximize (мс) |
| dragThreshold | int | 4 | Мин. сдвиг мыши до старта drag (px, DPI-scaled) |

### Методы Application

| Метод | Описание |
|-------|----------|
| `Initialize(desc)` | Создать окно |
| `Run()` | Запустить event loop |
| `Shutdown()` | Освободить ресурсы |
| `Quit()` | Завершить event loop |
| `SetRoot(widget)` | Установить корневой виджет |
| `GetRoot()` | Получить корневой виджет |
| `GetCanvas()` | Получить Canvas |
| `GetFontManager()` | Получить FontManager |
| `GetInput()` | Получить InputState |
| `GetWidth()` / `GetHeight()` | Размер окна |
| `GetSDLWindow()` | Внутреннее SDL-окно |
| `IsBorderless()` / `IsMaximized()` | Состояние окна |
| `ToggleMaximize()` | Развернуть/восстановить окно |
| `SetResizeBorderWidth(px)` | Переопределить зону resize |
| `SetMinWindowSize(w, h)` | Переопределить мин. размер |
| `SetDragThreshold(px)` | Переопределить порог drag |
| `SetTitlebarButtonSize(px)` | Размер кнопок titlebar |
| `SetDoubleClickMs(ms)` | Окно двойного клика |
| `GetDpiScale()` / `SetDpiScale(s)` | Content scale дисплея (HiDPI) |
| `ScalePx(logicalPx)` | Логические px → физические (с учётом DPI) |
| `SetOnTick(cb)` | Callback каждый кадр |
| `DispatchOnMainThread(cb)` | Callback на main thread |

### Singleton

```cpp
nui::Application* app = nui::GetApp();
app->Quit();
```

---

## Canvas

CPU 2D рендеринг.

### Методы

| Метод | Описание |
|-------|----------|
| `Clear(color)` | Очистить экран |
| `FillRect(rect, color)` | Залитый прямоугольник |
| `DrawRect(rect, color)` | Контур прямоугольника |
| `DrawLine(x1, y1, x2, y2, color)` | Линия |
| `DrawPixel(x, y, color)` | Пиксель |
| `DrawTexture(texture, dest)` | Текстура (растянуть) |
| `DrawTexture(texture, src, dest)` | Текстура (участок) |
| `DrawText(font, text, x, y, color)` | Текст |
| `DrawTextWrapped(font, text, bounds, color)` | Текст с переносом |
| `PushClip(rect)` | Начать clipping |
| `PopClip()` | Закончить clipping |

---

## Color

```cpp
struct Color {
    uint8_t r, g, b, a;

    Color();                              // Белый
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    uint32_t ToRGBA() const;

    static Color White();
    static Color Black();
    static Color Red();
    static Color Green();
    static Color Blue();
    static Color Gray();
    static Color DarkGray();
    static Color Transparent();
};
```

---

## Rect

```cpp
struct Rect {
    int x, y, w, h;

    Rect();
    Rect(int x, int y, int w, int h);

    bool Contains(int px, int py) const;
};
```

---

## Widget

Базовый класс всех виджетов.

### Методы

| Метод | Описание |
|-------|----------|
| `SetName(name)` | Установить имя |
| `GetName()` | Получить имя |
| `SetRect(x, y, w, h)` | Установить позицию и размер |
| `GetRect()` | Получить Rect |
| `SetPos(x, y)` | Установить позицию |
| `SetSize(w, h)` | Установить размер |
| `SetVisible(bool)` | Видимость |
| `SetEnabled(bool)` | Активность |
| `SetBgColor(color)` | Цвет фона |
| `SetBorderColor(color)` | Цвет рамки |
| `SetAlignH(AlignH)` | Горизонтальное выравнивание |
| `SetAlignV(AlignV)` | Вертикальное выравнивание |
| `SetAnchor(AnchorFlag)` | Якоря к краям родителя (флаги) |
| `SetAnchor(l, r, t, b)` | Нормализованные якоря (Godot-style, 0..1) |
| `SetStretch(w, h)` | Режим размера при ресайзе |
| `SetStretchW(m)` / `SetStretchH(m)` | Режим размера по одной оси |
| `SetMinSize(w, h)` | Минимальный размер |
| `SetMaxSize(w, h)` | Максимальный размер |
| `UpdateLayout()` | Пересчитать геометрию (якоря + дочерние) |
| `MarkLayoutDirty()` | Пометить поддерево как требующее пересчёта |
| `AddChild(widget)` | Добавить дочерний виджет |
| `GetChild(name)` | Найти дочерний по имени |
| `RemoveChild(name)` | Удалить дочерний |
| `ClearChildren()` | Удалить все дочерние |
| `SetOnClick(cb)` | Callback при клике |
| `ClearFocus()` | Снять фокус со всех дочерних |
| `HandleInput(input)` | Обработка ввода |
| `Update(dt)` | Обновление (каждый кадр) |
| `Render(canvas, fonts)` | Отрисовка |

### AlignH / AlignV

```cpp
enum class AlignH { Left, Center, Right };
enum class AlignV { Top,  Center, Bottom };
```

### AnchorFlag / StretchMode

```cpp
enum class AnchorFlag : uint8_t {
    None   = 0,
    Left   = 1 << 0,
    Top    = 1 << 1,
    Right  = 1 << 2,
    Bottom = 1 << 3,
};
AnchorFlag operator|(AnchorFlag, AnchorFlag);
AnchorFlag operator&(AnchorFlag, AnchorFlag);

enum class StretchMode { Fixed, Fill, Proportional };
```

Флаги комбинируются через `|`. Привязка к двум противоположным краям
(`Left|Right` или `Top|Bottom`) растягивает виджет между ними.

---

## Label

```cpp
class Label : public Widget {
    void SetText(const std::string& text);
    void SetFontSize(int size);
    void SetFont(Font* font);
    void SetTextColor(const Color& c);
    void SetWordWrap(bool wrap);
};
```

---

## Button

```cpp
class Button : public Widget {
    void SetText(const std::string& text);
    void SetFontSize(int size);
    void SetFont(Font* font);
    void SetTextColor(const Color& c);
    void SetHoverColor(const Color& c);
    void SetPressedColor(const Color& c);
    void SetOnClick(ClickCallback cb);

    bool IsHovered() const;
    bool IsPressed() const;
};
```

---

## Image

```cpp
enum class ScaleMode { Stretch, Fit, Fill, Center, Tile };

class Image : public Widget {
    bool LoadFromFile(const std::string& path, TextureCache& cache);
    void SetTexture(Texture* tex);
    void SetScaleMode(ScaleMode mode);
};
```

---

## EditBox

```cpp
class EditBox : public Widget {
    void SetText(const std::string& text);
    const std::string& GetText() const;
    void SetPlaceholder(const std::string& text);
    void SetFontSize(int size);
    void SetFont(Font* font);
    void SetTextColor(const Color& c);
    void SetPasswordMode(bool mask);
    void SetMaxLength(int len);

    void SetOnTextChanged(TextCallback cb);
    void SetOnEnter(TextCallback cb);
};
```

---

## ProgressBar

```cpp
class ProgressBar : public Widget {
    void SetValue(float value);    // 0.0 - 1.0
    float GetValue() const;
    void SetFillColor(const Color& c);
    void SetEmptyColor(const Color& c);
    void SetTextColor(const Color& c);
    void SetShowPercent(bool show);
    void SetLabel(const std::string& label);
    void SetFontSize(int size);
    void SetFont(Font* font);
};
```

---

## ScrollView

```cpp
class ScrollView : public Widget {
    void SetScrollY(int y);
    int  GetScrollY() const;
    int  GetMaxScrollY() const;
    void SetScrollbarColor(const Color& c);
    void SetScrollbarBgColor(const Color& c);
};
```

---

## Texture

```cpp
class Texture {
    bool LoadFromFile(const std::string& path);
    bool LoadFromMemory(const uint8_t* data, size_t size, const std::string& name = "");
    bool CreateFromPixels(int width, int height, const uint8_t* rgbaPixels);
    bool CreateSolid(int width, int height, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void Free();

    SDL_Surface* GetSurface() const;
    int GetWidth() const;
    int GetHeight() const;
    bool IsLoaded() const;
};
```

---

## TextureCache

```cpp
class TextureCache {
    Texture* Get(const std::string& path);  // Загрузить или получить из кеша
    void Clear();                            // Очистить кеш
};
```

---

## Font

```cpp
class Font {
    bool Load(const std::string& path, int size);
    bool LoadFromMemory(const uint8_t* data, size_t size, int fontSize);

    SDL_Surface* RenderText(const std::string& text, const Color& color) const;
    void FreeRenderedSurface(SDL_Surface* surface) const;

    int GetHeight() const;
    int GetTextWidth(const std::string& text) const;
    int GetSize() const;
};
```

---

## FontManager

```cpp
class FontManager {
    bool Initialize();
    void Shutdown();

    Font* Get(const std::string& path, int size);
    Font* GetDefault(int size = 16);
};
```

---

## ResourceManager

```cpp
struct ResourceData {
    const uint8_t* data;
    size_t         size;
    bool           valid() const;
};

class ResourceManager {
    static void Initialize();
    static ResourceData Load(const std::string& path);
    static bool Exists(const std::string& path);
};
```

---

## Async

```cpp
// Запуск на фоне
auto future = Async::Run([]() -> T {
    return result;
});

// Callback на main thread
future->Then([](T result) {
    // UI update
});

// Dispatch на main thread
Async::DispatchOnMainThread([]() {
    // UI update
});

// Drain queue (вызывается автоматически в Application::Run)
Async::ProcessMainThreadQueue();
```

---

## InputState

```cpp
class InputState {
    int  GetMouseX() const;
    int  GetMouseY() const;
    bool IsMouseDown(MouseButton btn) const;
    bool IsMouseClicked(MouseButton btn) const;   // Один кадр
    bool IsMouseReleased(MouseButton btn) const;
    int  GetWheelY() const;

    bool IsKeyDown(int keycode) const;
    bool IsKeyClicked(int keycode) const;          // Один кадр
    const std::string& GetTextInput() const;       // UTF-8 ввод
};

enum class MouseButton { Left, Middle, Right };
```

---

## Логирование

```cpp
#include "core/log.h"

NUI_LOG("Info: %s\n", message);
NUI_LOG_ERROR("Error: %d\n", code);
```

Выводит в stdout + `OutputDebugStringA` (VS Output Window на Windows).
