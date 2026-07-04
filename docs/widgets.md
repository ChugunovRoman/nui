# Виджеты

Все виджеты наследуются от `Widget` и поддерживают общие свойства.

## Общие свойства (Widget)

| Свойство | Тип | XML атрибут | Описание |
|----------|-----|-------------|----------|
| Name | string | `name` | Идентификатор для поиска через `GetChild()` |
| Position | int | `x`, `y` | Позиция относительно родителя |
| Size | int | `width`, `height` | Размер виджета |
| Visible | bool | `visible` | Видимость (по умолчанию `true`) |
| Enabled | bool | `enabled` | Активность (по умолчанию `true`) |
| BgColor | Color | `bg_color` | Цвет фона |
| BorderColor | Color | `border_color` | Цвет рамки |
| AlignH | enum | `align_h` | Горизонтальное выравнивание: `left`, `center`, `right` |
| AlignV | enum | `align_v` | Вертикальное выравнивание: `top`, `center`, `bottom` |
| Tooltip | string | `tooltip` | Подсказка |

### Цвета в XML

Формат: `r,g,b` или `r,g,b,a` (0-255):

```xml
bg_color="255,0,0"          <!-- красный -->
bg_color="0,0,0,128"        <!-- полупрозрачный чёрный -->
```

### C++ API

```cpp
auto widget = std::make_unique<Widget>();
widget->SetName("my_widget");
widget->SetRect(10, 20, 200, 100);
widget->SetVisible(true);
widget->SetBgColor(Color(30, 30, 40, 255));
widget->SetBorderColor(Color(80, 80, 100, 255));
widget->SetAlignH(AlignH::Center);
widget->SetAlignV(AlignV::Center);
```

---

## Label

Текстовая метка.

| Свойство | Тип | XML атрибут | Описание |
|----------|-----|-------------|----------|
| Text | string | `text` | Текст отображения |
| FontSize | int | `font_size` | Размер шрифта (по умолчанию 16) |
| Font | string | `font` | Путь к TTF файлу |
| TextColor | Color | `text_color` | Цвет текста |
| WordWrap | bool | `word_wrap` | Перенос строк |

### XML

```xml
<label x="10" y="10" width="300" height="30"
       text="Hello World" font_size="18"
       text_color="200,200,255" align_h="center"/>

<label x="10" y="50" width="300" height="100"
       text="Long text with word wrap enabled..."
       font_size="14" word_wrap="true"/>
```

### C++

```cpp
auto label = std::make_unique<Label>();
label->SetText("Hello World");
label->SetFontSize(18);
label->SetTextColor(Color(200, 200, 255));
label->SetAlignH(AlignH::Center);
label->SetWordWrap(true);
```

---

## Button

Кнопка с состояниями hover/press.

| Свойство | Тип | XML атрибут | Описание |
|----------|-----|-------------|----------|
| Text | string | `text` | Текст кнопки |
| FontSize | int | `font_size` | Размер шрифта |
| Font | string | `font` | Путь к TTF файлу |
| TextColor | Color | `text_color` | Цвет текста |
| HoverColor | Color | `hover_color` | Цвет фона при наведении |
| PressedColor | Color | `pressed_color` | Цвет фона при нажатии |

### XML

```xml
<button name="btn_play" x="10" y="10" width="200" height="50"
        text="PLAY" font_size="20"
        bg_color="40,120,40"
        hover_color="50,150,50"
        pressed_color="30,100,30"/>
```

### C++

```cpp
auto btn = std::make_unique<Button>();
btn->SetText("Click Me");
btn->SetBgColor(Color(40, 120, 200));
btn->SetHoverColor(Color(55, 140, 230));
btn->SetPressedColor(Color(30, 100, 170));
btn->SetOnClick([](Widget* w) {
    NUI_LOG("Button clicked!\n");
});
root->AddChild(std::move(btn));
```

---

## Image

Отображение картинки.

| Свойство | Тип | XML атрибут | Описание |
|----------|-----|-------------|----------|
| Src | string | `src` | Путь к изображению |
| Scale | enum | `scale` | Режим масштабирования |

### Режимы масштабирования

| Режим | Описание |
|-------|----------|
| `stretch` | Растянуть на весь виджет |
| `fit` | Вписать с сохранением пропорций |
| `fill` | Заполнить с обрезкой |
| `center` | По центру, оригинальный размер |
| `tile` | Замостить |

### XML

```xml
<image x="10" y="10" width="400" height="300"
       src="resources/images/cover.png" scale="fit"/>
```

### C++

```cpp
auto img = std::make_unique<Image>();
img->SetScaleMode(ScaleMode::Fit);
img->LoadFromFile("resources/images/cover.png", textureCache);
```

Поддерживаемые форматы: PNG, JPG, BMP, TGA, GIF.

---

## EditBox

Поле ввода текста.

| Свойство | Тип | XML атрибут | Описание |
|----------|-----|-------------|----------|
| Placeholder | string | `placeholder` | Текст-подсказка |
| Text | string | `text` | Начальный текст |
| FontSize | int | `font_size` | Размер шрифта |
| Font | string | `font` | Путь к TTF файлу |
| MaxLength | int | `max_length` | Макс. длина (0 = без ограничений) |
| Password | bool | `password` | Режим пароля (скрытый текст) |

### XML

```xml
<editbox name="username" x="10" y="10" width="300" height="32"
         placeholder="Enter name..." font_size="14"/>

<editbox name="password" x="10" y="50" width="300" height="32"
         placeholder="Password..." password="true" font_size="14"/>
```

### C++

```cpp
auto edit = std::make_unique<EditBox>();
edit->SetPlaceholder("Enter text...");
edit->SetFontSize(14);
edit->SetMaxLength(100);
edit->SetPasswordMode(true);

edit->SetOnTextChanged([](EditBox* eb, const std::string& text) {
    NUI_LOG("Text changed: %s\n", text.c_str());
});

edit->SetOnEnter([](EditBox* eb, const std::string& text) {
    NUI_LOG("Enter pressed: %s\n", text.c_str());
});
```

### Особенности

- Полная поддержка UTF-8 (кириллица, emoji)
- Backspace/Delete корректно удаляют multi-byte символы
- Стрелки перемещают курсор по символам, а не по байтам
- Курсор мигает с фрейм-независимой частотой
- Фокус: клик = фокус, клик снаружи = сброс

---

## ProgressBar

Индикатор прогресса.

| Свойство | Тип | XML атрибут | Описание |
|----------|-----|-------------|----------|
| Value | float | `value` | Значение 0.0 - 1.0 |
| FillColor | Color | `fill_color` | Цвет заполнения |
| EmptyColor | Color | `empty_color` | Цвет пустой части |
| TextColor | Color | `text_color` | Цвет текста |
| ShowPercent | bool | `show_percent` | Показывать процент |
| Label | string | `label` | Кастомная метка |

### XML

```xml
<progressbar x="10" y="10" width="300" height="24"
             value="0.75" show_percent="true"
             fill_color="40,120,200" border_color="60,60,80"/>

<progressbar x="10" y="50" width="300" height="24"
             value="0.5" label="50 / 100 items"
             fill_color="40,160,60"/>
```

### C++

```cpp
auto pb = std::make_unique<ProgressBar>();
pb->SetValue(0.75f);
pb->SetFillColor(Color(40, 120, 200));
pb->SetShowPercent(true);

// Обновление значения
pb->SetValue(1.0f);
```

---

## ScrollView

Прокручиваемый контейнер с scrollbar.

| Свойство | Тип | XML атрибут | Описание |
|----------|-----|-------------|----------|
| ScrollbarColor | Color | `scrollbar_color` | Цвет ползунка |
| ScrollbarBgColor | Color | `scrollbar_bg_color` | Цвет фона скроллбара |

### XML

```xml
<scrollview name="news" x="10" y="10" width="400" height="300"
            border_color="60,60,80">
    <panel x="5" y="5" width="385" height="50"
           bg_color="35,35,50" border_color="50,50,65">
        <label x="10" y="10" width="360" height="30"
               text="Item 1" font_size="14" text_color="200,200,220"/>
    </panel>
    <panel x="5" y="60" width="385" height="50"
           bg_color="35,35,50" border_color="50,50,65">
        <label x="10" y="10" width="360" height="30"
               text="Item 2" font_size="14" text_color="200,200,220"/>
    </panel>
</scrollview>
```

### C++

```cpp
auto scroll = std::make_unique<ScrollView>();
scroll->SetRect(10, 10, 400, 300);
scroll->SetBorderColor(Color(60, 60, 80));

for (int i = 0; i < 10; ++i) {
    auto item = std::make_unique<Widget>();
    item->SetRect(5, 5 + i * 55, 385, 50);
    item->SetBgColor(Color(35, 35, 50));

    auto label = std::make_unique<Label>();
    label->SetRect(10, 10, 360, 30);
    label->SetText("Item " + std::to_string(i));
    item->AddChild(std::move(label));

    scroll->AddChild(std::move(item));
}
```

### Особенности

- Контент обрезается по границам (scissor clipping)
- Прокрутка колесиком мыши
- Drag-scrollbar для мыши
- Scrollbar автоматически появляется если контент больше области

---

## Программное создание виджетов

### Добавление дочерних виджетов

```cpp
auto parent = std::make_unique<Widget>();
auto child = std::make_unique<Label>();
child->SetText("I'm a child");
parent->AddChild(std::move(child));
```

### Поиск по имени

```cpp
Widget* found = root->GetChild("btn_play");
if (found) {
    // Найден
}
```

### Callbacks

```cpp
// Кнопка
btn->SetOnClick([](Widget* w) {
    NUI_LOG("Clicked!\n");
});

// EditBox
edit->SetOnTextChanged([](EditBox* eb, const std::string& text) {
    NUI_LOG("Changed: %s\n", text.c_str());
});

edit->SetOnEnter([](EditBox* eb, const std::string& text) {
    NUI_LOG("Enter: %s\n", text.c_str());
});
```
