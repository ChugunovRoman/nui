# XML Layout

UI описывается в XML файлах. LayoutLoader парсит XML и строит дерево виджетов.

## Синтаксис

### Корневой элемент

```xml
<layout width="1024" height="768" bg_color="20,20,30">
    <!-- виджеты -->
</layout>
```

### Вложенность

Виджеты вкладываются друг в друга. Позиция дочерних виджетов — относительно родителя:

```xml
<panel name="sidebar" x="0" y="0" width="250" height="768" bg_color="30,30,45">
    <!-- x=20, y=20 относительно sidebar -->
    <button name="btn" x="20" y="20" width="210" height="40" text="Click"/>
</panel>
```

### Пример полного layout

```xml
<?xml version="1.0" encoding="UTF-8"?>
<layout name="launcher" width="1024" height="768" bg_color="20,20,30">

    <!-- Header -->
    <panel name="header" x="0" y="0" width="1024" height="60" bg_color="35,35,50">
        <label x="20" y="10" width="400" height="40"
               text="My Launcher" font_size="24" text_color="200,200,255"/>
        <label x="800" y="20" width="200" height="30"
               text="v1.0.0" font_size="14" text_color="120,120,140"
               align_h="right"/>
    </panel>

    <!-- Sidebar -->
    <panel name="sidebar" x="0" y="60" width="250" height="708" bg_color="28,28,42">
        <button name="btn_play" x="20" y="20" width="210" height="50"
                text="PLAY" font_size="20"
                bg_color="40,120,40" hover_color="50,150,50"
                pressed_color="30,100,30"/>
        <button name="btn_settings" x="20" y="80" width="210" height="40"
                text="Settings" font_size="16"/>
        <button name="btn_exit" x="20" y="130" width="210" height="40"
                text="Exit" font_size="16"
                bg_color="120,40,40" hover_color="150,50,50"
                pressed_color="100,30,30"/>

        <label x="20" y="200" width="210" height="20"
               text="Download:" font_size="12" text_color="150,150,170"/>
        <progressbar name="progress" x="20" y="225" width="210" height="25"
                     value="0.65" show_percent="true"
                     fill_color="40,120,200" border_color="60,60,80"/>

        <editbox name="search" x="20" y="280" width="210" height="32"
                 placeholder="Search..." font_size="14"/>
    </panel>

    <!-- Content -->
    <panel name="content" x="250" y="60" width="774" height="708" bg_color="24,24,34">
        <image name="cover" x="20" y="20" width="400" height="250"
               src="resources/images/cover.png" scale="fit"/>

        <label x="440" y="20" width="310" height="35"
               text="Game Title" font_size="28" text_color="220,220,255"/>

        <label x="440" y="60" width="310" height="200"
               text="Game description with word wrap enabled..."
               font_size="14" text_color="180,180,190" word_wrap="true"/>

        <scrollview name="news" x="20" y="320" width="734" height="360"
                    border_color="60,60,80">
            <panel x="8" y="8" width="710" height="50"
                   bg_color="35,35,50" border_color="50,50,65">
                <label x="10" y="5" width="690" height="20"
                       text="News item 1" font_size="14" text_color="200,200,220"/>
                <label x="10" y="28" width="690" height="15"
                       text="2026-07-04" font_size="11" text_color="100,100,120"/>
            </panel>
        </scrollview>
    </panel>
</layout>
```

## Справочник тегов

| XML тег | Класс | Описание |
|---------|-------|----------|
| `<layout>` | Widget | Корневой контейнер |
| `<panel>` | Widget | Контейнер |
| `<label>`, `<text>` | Label | Текстовая метка |
| `<button>` | Button | Кнопка |
| `<image>`, `<sprite>` | Image | Картинка |
| `<editbox>`, `<input>` | EditBox | Поле ввода |
| `<progressbar>`, `<progress>` | ProgressBar | Индикатор прогресса |
| `<scrollview>`, `<scroll>` | ScrollView | Прокручиваемый контейнер |

## Общие атрибуты

Все виджеты поддерживают:

| Атрибут | Тип | По умолчанию | Описание |
|---------|-----|-------------|----------|
| `name` | string | — | Идентификатор |
| `x`, `y` | int | 0 | Позиция |
| `width`, `height` | int | 0 | Размер |
| `visible` | bool | true | Видимость |
| `enabled` | bool | true | Активность |
| `bg_color` | color | transparent | Цвет фона |
| `border_color` | color | transparent | Цвет рамки |
| `color` | color | white | Основной цвет |
| `align_h` | enum | left | left / center / right |
| `align_v` | enum | top | top / center / bottom |
| `tooltip` | string | — | Подсказка |

## Цвета

Формат: `r,g,b` или `r,g,b,a`, значения 0-255:

```xml
bg_color="255,0,0"          <!-- красный -->
bg_color="0,0,0,128"        <!-- полупрозрачный чёрный -->
text_color="200,200,255"    <!-- светло-синий -->
```

## Загрузка layout

### C++

```cpp
nui::LayoutLoader loader;
nui::TextureCache textures;
nui::FontManager fonts;
fonts.Initialize();

auto root = loader.LoadFromFile("resources/layouts/main.xml",
                                 textures, fonts);
app.SetRoot(std::move(root));
```

### Из строки

```cpp
std::string xml = R"(
<layout width="800" height="600">
    <label x="10" y="10" width="200" height="30"
           text="From string" font_size="18"/>
</layout>
)";

auto root = loader.LoadFromString(xml, textures, fonts);
```

### Цветовые определения

Загрузка именованных цветов из XML:

```xml
<!-- colors.xml -->
<colors>
    <color name="primary" value="40,120,200"/>
    <color name="danger" value="200,40,40"/>
</colors>
```

```cpp
loader.LoadColorDefs("resources/layouts/colors.xml");
```

Использование в layout:

```xml
<button bg_color="primary" text_color="danger"/>
```

## Получение виджетов из layout

```cpp
auto root = loader.LoadFromFile("layout.xml", textures, fonts);

// Поиск по имени
Button* btn = static_cast<Button*>(root->GetChild("btn_play"));
if (btn) {
    btn->SetOnClick([](Widget*) {
        NUI_LOG("Play!\n");
    });
}

Label* title = static_cast<Label*>(root->GetChild("title"));
if (title) {
    title->SetText("New Title");
}

EditBox* search = static_cast<EditBox*>(root->GetChild("search"));
if (search) {
    search->SetOnTextChanged([](EditBox* eb, const std::string& text) {
        NUI_LOG("Search: %s\n", text.c_str());
    });
}
```
