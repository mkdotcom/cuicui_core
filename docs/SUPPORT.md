# Platform support matrix

What works where, as of the first public release (v0.1). Established by reading the code
(per-platform `#if CUICUI_*` branches), not by an automated test suite — treat it as a guide and
report anything wrong.

Legend: ✅ supported · ⚠ partial (see note) · ❌ not available · — not applicable.
macOS / Linux / Win32 desktop are not supported at all.

## Widgets

| Widget | Android | iOS | UWP | Web | Notes |
|---|---|---|---|---|---|
| `wEmpty`, `wRows`, `wColumns`, `wHGap`, `wVGap` | ✅ | ✅ | ✅ | ✅ | layout only |
| `wPadding`, `wBorder`, `wStackPanel`, `wTabPanel`, `wAlignRight` | ✅ | ✅ | ✅ | ✅ | borders: see below |
| `wSafeZone` | ✅ | ✅ | ⚠ | ⚠ | safe-area insets are only reported on Android; iOS relies on the host view controller; always 0 on UWP/Web |
| `wText` (rich text, ellipsis) | ✅ | ✅ | ✅ | ✅ | see text features below |
| `wTextInput` | ✅ | ⚠ | ✅ | ⚠ | iOS: multiline mode is broken; Web: Enter can't insert a newline in multiline |
| `wImage`, `wButtonImage(2)` | ✅ | ✅ | ✅ | ✅ | `MIN` size not supported; see image features |
| `wScrollView`, `wVerticalListView` | ✅ | ⚠ | ✅ | ✅ | `MIN` size on the scrolling axis not supported; iOS: scrollbars hidden, `OnUserScrolled` only on drag start |
| `wWebView` | ⚠ | ⚠ | ✅ | ⚠ | `MIN` not supported; Android: no scrolling inside a `wScrollView`, no `BgColor`; iOS: `Source()` not implemented, display unverified; Web: `Html()` only |
| `wGradient` | ⚠ | ⚠ | ✅ | ❌ | vertical only; Android ignores stop offsets; iOS forces alpha to 1 |
| `wButton`, `wSwitch`, `wSlider`, `wSliderVertical`, `wProgressIndicator` | ✅ | ✅ | ⚠ | ⚠ | UWP/Web: minor pixel-rounding issues; UWP: `wSwitch` only reacts to clicks on the dot |
| `native::wCalendar` (date picker) | ✅ | ⚠ | ✅ | ✅ | no setter for the date; iOS sizing to check |
| `native::wTimePicker` | ✅ | ❌ | ✅ | ✅* | *in the template's web build, not tested in a browser yet |
| `native::wNativeSwitch`, `native::wNativeButton` | ❌ | ❌ | ⚠ | ❌ | samples; UWP only, `onClick` not wired |
| `native::wActivityIndicator` | ❌ | ⚠ | ❌ | ❌ | experimental, not built by default |

## Base widget features (`wWidget`)

| Feature | Android | iOS | UWP | Web | Notes |
|---|---|---|---|---|---|
| `Clickable`, `SetOpaqueToTouch`, `Visibility`, `Opacity`, `BgColor` | ✅ | ✅ | ✅ | ✅ | |
| `OnGesture_Tap`, double tap | ✅ | ✅ | ✅ | ✅ | Web: tap emulated (pointer up within 15 px, left button) |
| `OnGesture_LongPress` | ✅ | ❌ | ⚠ | ❌ | UWP: touch only |
| `OnGesture_Pan` (one finger) | ✅ | ✅ | ⚠ | ⚠ | UWP: mouse/pen drag; Web: no Start/End states |
| `OnGesture_Pan` (two fingers), `OnGesture_Pinch` | ✅ | ✅ | ❌ | ❌ | iOS: pinch never sends End |
| `OnGesture_Swipe` | ❌ | ⚠ | ❌ | ❌ | iOS only, direction not filled |
| Pointer pressed / released / moved | ✅ | ✅ | ✅ | ✅ | UWP/Web: left mouse button |
| Pointer entered / exited | ⚠ | ⚠ | ✅ | ✅ | emulated on touch down/up on mobile |
| `OnMouseWheel` | ❌ | ⚠ | ✅ | ✅ | |
| `Border` with corner radius | ⚠ | ✅ | ⚠ | ⚠ | Android ignores border alpha; Web ignores per-corner flags; UWP: not on `wImage` |
| `BlurBackground` | ❌ | ✅ | ❌ | ❌ | |
| `EnableDropFile` | — | — | ✅ | ✅ | needs a mouse |
| `LinkToHardwareBackButton` | ✅ | — | ✅ | ✅ | UWP: Win+Backspace; Web: browser back |

## Text, input, images

| Feature | Android | iOS | UWP | Web |
|---|---|---|---|---|
| Rich text (bold, italic, color) | ✅ | ✅ | ✅ | ✅ |
| Touchable spans: click | ✅ | ✅ | ✅ | ✅ |
| Touchable spans: hover / exit | ❌ | ❌ | ✅ | ✅ |
| `wText::GetCharacterPositionFromLocalPos` | ✅ | ✅ | ✅ | ❌ |
| `wText::AllowTextSelection` | ❌ | ❌ | ✅ | ✅ |
| `wTextInput` password | ✅ | ✅ | ✅ | ✅ |
| `wTextInput` multiline | ✅ | ❌ | ✅ | ⚠ |
| `onEnterKeyPressed` | ✅ | ❌ | ✅ | ✅ |
| `DisableReturnKey` | ✅ | ❌ | ✅ | ❌ |
| `EnablePasteFile` | — | — | ✅ | ✅ |
| `wImage` local / http / base64 sources, Contain / Cover, transforms | ✅ | ✅ | ✅ | ✅ |
| Animated GIF | ✅ (autoplay) | ⚠ (remote only) | ✅ (+ play/stop) | ✅ (autoplay) |
| `wImage::Tint` | ✅ | ❌ | ❌ | ❌ |
| `wImage::Grayscale` | ❌ | ❌ | ❌ | ❌ |
| `cFileReceived::ConvertToJpgAndResize` | ✅ | ⚠ | ❌ | ❌ |

## `central` (app-level services)

| Feature | Android | iOS | UWP | Web |
|---|---|---|---|---|
| `OpenFilePicker` | ✅ | ✅ | ✅ | ✅ |
| Camera (`OpenCamera`) | ✅ | ✅ | ❌ | ❌ |
| Message boxes | ⚠ | ⚠ | ⚠ | ⚠ |
| Clipboard | ✅ | ❌ | ✅ | ❌ |
| Safe area (`safeAreaRect`) | ✅ | via host | 0 | 0 |
| Virtual keyboard mode (resize / pan) | ✅ | ✅ | — | — |
| Screen size / orientation | ✅ | ✅ | ✅ | ✅ |
| Temp / assets / documents folders | ✅ | ✅ | ⚠ (no documents) | ❌ |
| App title, mouse cursor | — | — | ✅ | ✅ |
| Icon badge number | ❌ | ❌ | ✅ | ❌ |
| Open OS app settings | ✅ | ✅ | ✅ | ❌ |
| Exit the app | ✅ | ❌ | ✅ | ⚠ |
| Open an external app / URL | ✅ | ✅ | ✅ | ✅ |
| Receive shares / initial payload | ✅ | ❌ | ⚠ | ❌ |
| App state (foreground / background) | ✅ | ✅ | ✅ | ✅ |
| Volume keys | ✅ | ❌ | ❌ | ❌ |

Message boxes: the `onOk` callback is only honored on iOS.

## Services

| Service | Android | iOS | UWP | Web | Notes |
|---|---|---|---|---|---|
| `WebRequest` POST (+ headers) | ✅ | ✅ | ⚠ | ✅ | GET is not implemented; body sent as `text/plain` |
| `LocalStorage` | ✅ | ✅ | ✅ | ✅ | values ≤ 4095 chars |
| Timers | ✅ | ✅ | ✅ | ✅ | one-shot; no safe cancel |
| Animations (`animationManager`, easing) | ✅ | ✅ | ✅ | ✅ | |
| Threads, `RunOnUiThread` | ✅ | ✅ | ✅ | ⚠ | no threads on the Web (runs synchronously) |
