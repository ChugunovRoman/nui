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
| Anchor | AnchorFlag | `anchor` | Якоря к краям родителя: `left` `top` `right` `bottom` `center` `all` (через пробел). Включает адаптивную вёрстку |
| Anchor (Godot) | float | `anchor_left`/`anchor_right`/`anchor_top`/`anchor_bottom` | Нормализованные точки (0..1); -1 = наследовать из флагов |
| StretchW/H | StretchMode | `stretch_w`, `stretch_h` | Режим размера при ресайзе: `fixed`, `fill`, `proportional` |
| MinSize | int | `min_width`, `min_height` | Минимальный размер (clamp после layout) |
| MaxSize | int | `max_width`, `max_height` | Максимальный размер (clamp после layout) |

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

## Slider

Ползунок для выбора значения в диапазоне. Внутреннее значение всегда
нормализовано в `[0.0, 1.0]`; `min`/`max` задают отображаемый диапазон, а
`GetRangedValue()` возвращает значение в нём.

| Свойство | Тип | XML атрибут | Описание |
|----------|-----|-------------|----------|
| Value | float | `value` | Значение. Если заданы `min`/`max`, интерпретируется в их единицах; иначе в `[0,1]` |
| MinValue | float | `min` | Минимум диапазона |
| MaxValue | float | `max` | Максимум диапазона |
| TrackColor | Color | `track_color` | Цвет дорожки |
| FillColor | Color | `fill_color` | Цвет заполненной части |
| ThumbColor | Color | `thumb_color` | Цвет ползунка |

### XML

```xml
<slider name="volume" x="10" y="10" width="200" height="24"
        value="0.6" fill_color="60,160,80" thumb_color="220,230,240"/>

<!-- с диапазоном: value="50" при min=0/max=100 → внутреннее 0.5 -->
<slider name="sensitivity" x="10" y="40" width="200" height="24"
        min="0" max="100" value="50"/>
```

### C++

```cpp
auto slider = std::make_unique<Slider>();
slider->SetValue(0.6f);
slider->SetFillColor(Color(60, 160, 80));
slider->SetOnValueChanged([](Widget*, float val) {
    NUI_LOG("Value: %.0f%%\n", val * 100);
});
```

### Особенности

- Клик по дорожке перемещает ползунок к курсору (click-to-jump).
- Drag автоматически отпускается при отпускании кнопки мыши, даже если слайдер стал невидимым/неактивным во время перетаскивания.
- Колёсико мыши над слайдером меняет значение на 5%.

---

## CheckBox

Чекбокс с текстом.

| Свойство | Тип | XML атрибут | Описание |
|----------|-----|-------------|----------|
| Text | string | `text` | Текст рядом с чекбоксом |
| Checked | bool | `checked` | Состояние (по умолчанию false) |
| CheckColor | Color | `check_color` | Цвет галочки |
| FontSize | int | `font_size` | Размер шрифта |

### XML

```xml
<checkbox name="sound" x="10" y="10" text="Enable sound"
          checked="true" check_color="80,200,120"/>
```

### C++

```cpp
auto cb = std::make_unique<CheckBox>();
cb->SetText("Enable sound");
cb->SetChecked(true);
cb->SetOnCheckedChanged([](Widget*, bool checked) {
    NUI_LOG("Sound: %s\n", checked ? "ON" : "OFF");
});
```

---

## RadioButton

Радиокнопка с групповой exclusivity.

| Свойство | Тип | XML атрибут | Описание |
|----------|-----|-------------|----------|
| Text | string | `text` | Текст рядом с кнопкой |
| Group | string | `group` | Имя группы (только одна selected) |
| Selected | bool | `selected` | Выбрана ли |
| DotColor | Color | `dot_color` | Цвет точки |
| FontSize | int | `font_size` | Размер шрифта |

### XML

```xml
<radiobutton name="low" x="10" y="10" text="Low"
             group="quality" dot_color="80,180,255"/>
<radiobutton name="high" x="10" y="40" text="High"
             group="quality" selected="true"/>
```

### C++

```cpp
auto rb = std::make_unique<RadioButton>();
rb->SetText("Option A");
rb->SetGroup("options");
rb->SetSelected(true);
rb->SetOnSelectedChanged([](Widget*) {
    NUI_LOG("Selected!\n");
});
```

### Особенности

- Группы хранятся в глобальном реестре, поэтому радиокнопки в разных родительских контейнерах корректно образуют одну mutually-exclusive группу.
- При выборе одной кнопки callback `OnSelectedChanged` вызывается и у снимаемой с выбора кнопки, и у новой выбранной.
- `SetSelected(true)` в XML/коде корректно снимает предыдущий выбор в группе даже до добавления виджета в дерево.

---

## Dropdown (ComboBox)

Выпадающий список с выбором.

| Свойство | Тип | XML атрибут | Описание |
|----------|-----|-------------|----------|
| ItemHeight | int | `item_height` | Высота элемента (по умолчанию 28) |
| MaxVisibleItems | int | `max_visible` | Макс. видимых элементов (по умолчанию 5) |
| TextColor | Color | `text_color` | Цвет текста |

### XML

```xml
<dropdown name="server" x="10" y="10" width="200" height="28"
          item_height="28" max_visible="5">
    <item>Server EU</item>
    <item>Server US</item>
    <item>Server Asia</item>
</dropdown>
```

### C++

```cpp
auto dd = std::make_unique<Dropdown>();
dd->AddItem("Server EU");
dd->AddItem("Server US");
dd->AddItem("Server Asia");
dd->SetOnItemSelected([](Widget*, int idx, const std::string& text) {
    NUI_LOG("Selected: %s\n", text.c_str());
});
```

### Особенности

- При раскрытии список автоматически открывается вверх, если внизу не хватает места (до низа окна).
- Раскрытый список временно снимает clip-стек родителя, поэтому корректно отображается даже внутри ScrollView.
- Раскрытый dropdown захватывает ввод (input capture): любой клик закрывает список и не проваливается в нижележащие виджеты.

---

## TabControl

Контейнер с полосой вкладок и переключаемыми страницами. Активна только одна
страница за раз.

| Свойство | Тип | XML атрибут | Описание |
|----------|-----|-------------|----------|
| ActiveTab | int | `active_tab` | Индекс активной вкладки |
| FontSize | int | `font_size` | Размер шрифта заголовков |
| TabHeight | int | `tab_height` | Высота полосы вкладок (по умолчанию 28) |
| TextColor | Color | `text_color` | Цвет текста активной вкладки |
| TabBgColor | Color | `tab_color` | Фон неактивной вкладки |
| ActiveTabColor | Color | `active_color` | Фон активной вкладки |
| HoverTabColor | Color | `hover_color` | Фон вкладки под курсором |
| ContentBgColor | Color | `content_color` | Фон области контента |

Вкладки задаются дочерними тегами `<tab title="...">`, внутри которых
размещаются виджеты-страницы.

### XML

```xml
<tabcontrol name="settings" x="10" y="10" width="500" height="400"
            tab_height="30" active_tab="0"
            active_color="60,90,150" content_color="25,25,35">
    <tab title="Graphics">
        <label x="10" y="10" width="200" height="24" text="Quality"/>
        <dropdown x="10" y="40" width="180" height="28">
            <item>Low</item>
            <item>High</item>
        </dropdown>
    </tab>
    <tab title="Audio">
        <label x="10" y="10" width="200" height="24" text="Volume"/>
        <slider x="10" y="40" width="200" height="24" value="0.7"/>
    </tab>
</tabcontrol>
```

### C++

```cpp
auto tc = std::make_unique<TabControl>();
tc->SetTabHeight(30);

auto page1 = std::make_unique<Widget>();
auto lbl = std::make_unique<Label>();
lbl->SetText("Graphics settings");
page1->AddChild(std::move(lbl));
tc->AddTab("Graphics", std::move(page1));

tc->SetOnTabChanged([](Widget*, int idx) {
    NUI_LOG("Active tab: %d\n", idx);
});
```

### Особенности

- Полоса вкладок рисуется сверху; область контента — под ней.
- Активная страница рендерится с clip по области контента.
- Клик по заголовку переключает активную вкладку и стреляет `OnTabChanged`.

---

## Treeview

Сворачиваемое иерархическое дерево с вертикальной прокруткой. Модель данных —
рекурсивная структура `Node` (не виджет-на-узел), разворачивается в плоский
список строк с отступами по уровню вложенности.

| Свойство | Тип | XML атрибут | Описание |
|----------|-----|-------------|----------|
| RowHeight | int | `row_height` | Высота строки (по умолчанию 20) |
| Indent | int | `indent` | Отступ на каждый уровень вложенности (по умолчанию 16) |
| FontSize | int | `font_size` | Размер шрифта |
| TextColor | Color | `text_color` | Цвет текста |
| SelectedColor | Color | `selected_color` | Фон выбранной строки |
| HoverColor | Color | `hover_color` | Фон строки под курсором |

Узлы задаются рекурсивными тегами `<node>`. Атрибуты узла: `text`,
`expanded` (bool), `user_data` (строка).

### XML

```xml
<treeview name="files" x="10" y="10" width="300" height="400"
          row_height="22" indent="18">
    <node text="project" expanded="true">
        <node text="src">
            <node text="main.cpp" user_data="file_main"/>
            <node text="utils.cpp" user_data="file_utils"/>
        </node>
        <node text="resources"/>
    </node>
</treeview>
```

### C++

```cpp
auto tv = std::make_unique<Treeview>();

Treeview::Node root;
Treeview::Node project("project");
project.expanded = true;
Treeview::Node src("src");
src.children.emplace_back("main.cpp").userData = "file_main";
src.children.emplace_back("utils.cpp").userData = "file_utils";
project.children.push_back(std::move(src));
root.children.push_back(std::move(project));
tv->SetRoot(std::move(root));

tv->SetOnNodeSelected([](Widget*, const Treeview::NodePath& path) {
    NUI_LOG("Selected node, depth=%d\n", (int)path.size());
});
```

### Особенности

- Колесо мыши над виджетом прокручивает список.
- Клик по глифу ▶/▼ сворачивает/раскрывает узел (стреляет `OnNodeToggled`).
- Клик по строке выбирает узел (стреляет `OnNodeSelected` с путём из индексов).
- Прокрутка через drag правого скроллбара.
- **Мутация модели:** `GetRoot()` даёт неконстантный доступ к дереву. Любое
  изменение (`push_back`/`erase` в `children`, которое может реаллоцировать
  `std::vector<Node>`) инвалидирует внутренний кэш — после правки вызовите
  `tv->Rebuild()`, иначе клики/рендер будут использовать висячие указатели.
- **Скрытие выбранного узла:** при сворачивании родителя выбранного узла
  выделение визуально исчезает (строки нет в развёрнутом списке), но
  `GetSelectedPath()` по-прежнему возвращает путь.

---

## Tooltip

Всплывающая подсказка. Не имеет отдельного XML-тега — показывается
автоматически при наведении на любой виджет, у которого задан непустой
атрибут `tooltip` (общее свойство всех виджетов).

### Как использовать

Добавьте атрибут `tooltip` к любому виджету:

```xml
<button name="btn_play" x="10" y="10" width="200" height="50"
        text="PLAY" tooltip="Launch the game"/>
```

Подсказка появляется рядом с курсором после задержки наведения (по умолчанию
0.5 с), позиционируется через `PopupGeometry` с clamping к границам окна.

### C++

```cpp
btn->SetTooltip("Launch the game");

// Управление встроенным менеджером подсказок (на уровне Application):
app->SetTooltipEnabled(true);          // включено по умолчанию
app->SetTooltipDelay(0.8f);            // задержка перед показом, секунды
```

### Особенности

- Менеджер (`TooltipManager`, синглтон) находит верхний виджет под курсором с
  непустым `tooltip` и копит hover-таймер.
- При срабатывании таймера `TooltipWidget` пушится в overlay-stack как
  немодальный overlay (`modal=false, closeOnOutsideClick=false`) — он просто
  следует за курсором, не перехватывая клики.
- Отключается целиком через `Application::SetTooltipEnabled(false)`.

---

## Menu / ContextMenu

Вертикальное всплывающее меню (пункты, сепараторы, подменю). Открывается как
overlay-попап и закрывается кликом снаружи.

| Свойство | Тип | XML атрибут | Описание |
|----------|-----|-------------|----------|
| FontSize | int | `font_size` | Размер шрифта |
| ItemHeight | int | `item_height` | Высота пункта (по умолчанию 26) |
| MinPanelWidth | int | `min_panel_width` | Минимальная ширина панели (по умолчанию 120). Для совместимости принимается и устаревший `min_width` |
| TextColor | Color | `text_color` | Цвет текста |
| HoverColor | Color | `hover_color` | Фон пункта под курсором |
| PanelColor | Color | `panel_color` | Фон панели |

Пункты задаются тегами `<item text="..." action="id">` (действие задаётся
строковым id, который разрешается хост-приложением через
`Menu::SetActionResolver`), `<separator/>` и подменю
`<menu text="...">...</menu>`.

### XML

```xml
<contextmenu name="ctx" min_panel_width="160">
    <item text="Open" action="open"/>
    <item text="Rename" action="rename"/>
    <separator/>
    <item text="Delete" action="delete" enabled="false"/>
    <menu text="Send to">
        <item text="Desktop" action="send_desktop"/>
        <item text="Archive" action="send_archive"/>
    </menu>
</contextmenu>
```

### C++

```cpp
// Регистрируем резолвер action id → callback (один раз).
Menu::SetActionResolver([](const std::string& id) -> std::function<void()> {
    if (id == "open")   return [] { NUI_LOG("Open\n"); };
    if (id == "delete") return [] { NUI_LOG("Delete\n"); };
    return nullptr;
});

// Строим и открываем меню при правом клике:
auto menu = std::make_unique<Menu>();
menu->AddItem("Open",   [] { NUI_LOG("Open\n"); });
menu->AddItem("Rename", [] { NUI_LOG("Rename\n"); });
menu->AddSeparator();
menu->AddDisabledItem("Delete");
Menu::Open(std::move(menu), clickX, clickY, screenW, screenH);
```

### Особенности

- `Menu::Open` принимает ownership и пушит меню в overlay-stack как немодальное
  с `closeOnOutsideClick=true` — клик мимо закрывает меню.
- Подменю открываются отдельным overlay справа от родительского пункта.
- Ширина панели измеряется по самому длинному пункту при первом рендере.
- После клика по пункту с действием закрывается вся цепочка меню.

---

## Dialog / MessageBox

Модальное диалоговое окно: заголовок, текст сообщения, опциональные виджеты
контента и строка кнопок. Открывается как модальный overlay — фон затемняется,
ввод в нижележащие виджеты блокируется до закрытия.

| Свойство | Тип | XML атрибут | Описание |
|----------|-----|-------------|----------|
| Title | string | `title` | Заголовок окна |
| Message | string | `message` | Текст сообщения (с переносом строк) |
| Buttons | enum | `buttons` | Набор кнопок: `ok`, `okcancel`, `yesno`, `yesnocancel`, `retrycancel` |
| FontSize | int | `font_size` | Размер шрифта |
| TitleColor | Color | `title_color` | Фон заголовка |
| PanelColor | Color | `panel_color` | Фон панели |

Вложенные виджеты (кроме `<button>`) становятся контентом диалога.

### XML

```xml
<dialog name="confirm" title="Quit?"
        message="Unsaved changes will be lost. Continue?"
        buttons="yesno"
        title_color="160,60,60"/>
```

### C++

```cpp
// Простое сообщение:
Dialog::ShowMessage("Error", "File not found",
                    DialogButtons::Ok,
                    [](DialogResult r) {
                        if (r == DialogResult::Ok) NUI_LOG("Acknowledged\n");
                    },
                    screenW, screenH);

// Полноценный диалог с контентом:
auto dlg = std::make_unique<Dialog>();
dlg->SetTitle("Settings");
dlg->SetMessage("Choose quality:");
auto dd = std::make_unique<Dropdown>();
dd->AddItem("Low");
dd->AddItem("High");
dlg->AddContent(std::move(dd));
dlg->SetButtons(DialogButtons::OkCancel);
dlg->SetOnResult([](DialogResult r) {
    NUI_LOG("Result: %d\n", (int)r);
});
Dialog::Open(std::move(dlg), screenW, screenH);
```

### Особенности

- `Dialog::Open` принимает ownership и пушит диалог в overlay-stack как
  **модальный** (`modal=true`) — overlay-pass рисует полупрозрачный dim-фон.
- Клавиатура: Enter = первичная кнопка, Esc = Cancel (если есть).
- После клика по кнопке стреляет `OnResult` и диалог снимается с overlay.

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
