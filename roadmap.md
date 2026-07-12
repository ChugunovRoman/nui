# NUI Roadmap

Планы развития UI toolkit.

## Статус

| Фича | Статус | Версия |
|------|--------|--------|
| CPU software rendering | ✅ Готово | 0.1.0 |
| Базовые виджеты (Label, Button, Image, EditBox, ProgressBar, ScrollView) | ✅ Готово | 0.1.0 |
| XML layouts | ✅ Готово | 0.1.0 |
| Async (ThreadPool + Future) | ✅ Готово | 0.1.0 |
| Вшивание ресурсов в бинарник | ✅ Готово | 0.1.0 |
| Кроссплатформенность (Windows, Linux, macOS) | ✅ Готово | 0.1.0 |
| Анимации (Tween + Easing + Rotation) | ✅ Готово | 0.2.0 |
| Адаптивные Layouts (Anchor System) | 📋 Планируется | 0.3.0 |

---

## Планы

### 1. Расширение виджетов

Добавить новые UI компоненты для полноценного лаунчера.

| Виджет | Описание | Приоритет | Статус |
|--------|----------|-----------|--------|
| **CheckBox** | Чекбокс с текстом | Высокий | ✅ Готово |
| **RadioButton** | Радиокнопка (группа) | Высокий | ✅ Готово |
| **Slider** | Ползунок (0-1, int/float) | Высокий | ✅ Готово |
| **Dropdown / ComboBox** | Выпадающий список | Высокий | ✅ Готово |
| **TabControl** | Вкладки | Средний |
| **Treeview** | Дерево с вложенностью | Средний |
| **Tooltip** | Всплывающая подсказка | Средний |
| **Menu / ContextMenu** | Контекстное меню | Средний |
| **Dialog / MessageBox** | Модальное диалоговое окно | Средний |
| **RichText** | Текст с форматированием (bold, italic, color) | Низкий |
| **Grid / Table** | Табличное размещение | Низкий |
| **ColorPicker** | Выбор цвета | Низкий |

### 2. Рендер видео и GIF

Поддержка анимированного контента в UI.

| Фича | Описание |
|------|----------|
| **GIF playback** | Декодирование и покадровое отображение GIF |
| **Video playback** | Воспроизведение видео через FFmpeg/libav (CPU decode) |
| **Animated sprite** | Sprite sheet анимации |

**Зависимости:**
- `gif.h` (header-only GIF decoder) или `gifdec`
- FFmpeg/libav для видео (опционально)

### 3. Воспроизведение звуков

Аудио для лаунчеров (музыка, SFX).

| Формат | Описание |
|--------|----------|
| **WAV** | Простой PCM, без сжатия |
| **OGG** | Vorbis сжатие |
| **MP3** | MPEG Layer 3 |

**Подход:**
- Использовать SDL_Audio (уже в SDL2 submodule)
- Или `miniaudio` (header-only, кроссплатформенный)
- Простой API: `Sound::Load()`, `Sound::Play()`, `Sound::Stop()`

### 4. Адаптивные Layouts (Anchor System)

Система позиционирования, при которой виджеты автоматически адаптируются при изменении размера окна.

**Проблема:** сейчас виджеты имеют фиксированные `x, y, width, height`. При ресайзе окна всё остаётся на месте.

**Решение:** система якорей (anchors) + stretch/offset режимы.

| Свойство | Описание |
|----------|----------|
| **anchor_left** | Привязка к левому краю родителя (0.0–1.0) |
| **anchor_right** | Привязка к правому краю родителя |
| **anchor_top** | Привязка к верхнему краю |
| **anchor_bottom** | Привязка к нижнему краю |
| **stretch_w** | Режим ширины: fixed / fill / proportional |
| **stretch_h** | Режим высоты: fixed / fill / proportional |
| **min_width / min_height** | Минимальный размер |
| **max_width / max_height** | Максимальный размер |

**Примеры поведения:**

```
┌──────────────────────────────────────┐
│ [Logo]  anchor: left+top             │  Logo всегда в левом верхнем углу
│         offset: 20, 20               │
│                                      │
│         [Button]  anchor: center     │  Button всегда по центру
│                                      │
│ [Status] anchor: left+bottom         │  Status всегда в левом нижнем углу
│          offset: 20, -40             │
│                                      │
│ [Sidebar] anchor: left+top+bottom    │  Sidebar растягивается по высоте
│           width: 250, stretch_h: fill│
│                                      │
│ [Content] anchor: all                │  Content заполняет оставшееся
│           stretch: fill              │
└──────────────────────────────────────┘
```

**XML синтаксис:**

```xml
<button name="btn_play"
        anchor="center"
        width="200" height="50"
        text="PLAY"/>

<panel name="sidebar"
       anchor="left top bottom"
       width="250"
       stretch_h="fill"/>

<image name="bg"
       anchor="all"
       stretch_w="fill" stretch_h="fill"
       src="resources/images/bg.jpg" scale="fill"/>
```

**API:**

```cpp
btn->SetAnchor(Anchor::Center);
sidebar->SetAnchor(Anchor::Left | Anchor::Top | Anchor::Bottom);
sidebar->SetStretchH(StretchMode::Fill);
content->SetAnchor(Anchor::All);
content->SetStretch(StretchMode::Fill, StretchMode::Fill);
```

### 5. ~~Анимации~~ ✅

Реализовано в v0.2.0:

- 30+ easing функций (Linear, Quad, Cubic, Elastic, Back, Bounce)
- Tween система с callback'ами
- Animator менеджер (AnimateX/Y/Width/Height/Alpha)
- Ping-pong, loop, delay, tag-based cancellation
- Framerate-independent (Update(dt))
- **Вращение виджетов (Spin):**
  - `Widget::SetRotation(deg)` + `SetRotationCenter(0..1, 0..1)` — точка вращения в нормализованных координатах (центр, угол и т.д.)
  - `Animator::AnimateRotation(widget, toDeg, duration, ease)`
  - CPU-ротатор `Canvas::DrawSurfaceRotated` (inverse-mapping, корректное вращение вокруг произвольной точки — фикс деформации при смещённом pivot, напр. "Corner spin")
  - Опциональный билинейный сэмплинг для гладких краёв
  - Рендер-кэш перестраивается только по dirty-флагу — статичные вращаемые виджеты бесплатны, анимация вращения не инвалидирует кэш
  - Ownership tweens: новая анимация свойства отменяет предыдущую (нет конфликта)

### 6. GPU рендер (альтернативный бэкенд)

Возможность переключиться с CPU software rendering на GPU для производительности.

| Бэкенд | Платформа | Статус |
|--------|-----------|--------|
| **CPU Software** (текущий) | Все | ✅ Готово |
| **DirectX 11** | Windows | Планируется |
| **OpenGL 3.3+** | Windows, Linux, macOS | Планируется |
| **Vulkan** | Windows, Linux | Планируется |

**Архитектура:**
```
Canvas (абстракция)
  ├─ CPUCanvas     ← текущий (SDL_Surface)
  ├─ DX11Canvas    ← DirectX 11
  ├─ GLCanvas      ← OpenGL
  └─ VulkanCanvas  ← Vulkan
```

UI Kit не зависит от способа рендера — виджеты работают через Canvas API.

**Приоритет:** OpenGL > DirectX > Vulkan
