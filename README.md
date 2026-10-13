# cuicui

**A lightweight, "Qt-like" native UI toolkit for C++.**
Write your app's UI once in C++, run it on **Android**, **iOS**, **Windows** (UWP) and the **Web** —
with **real native views** on every platform.

> [!WARNING]
> **Status: functional, not production-ready (for you).**
> cuicui has been used for years to build and ship real apps on the stores
> (Android, iOS, Windows) and on the web. It works.
> But it was built for in-house use: the API is **not stable** (expect breaking changes
> before 1.0), documentation is thin, some widgets are not available on every platform,
> and there is no automated test suite yet.
> Use it if you like the approach and are ready to read the source. Don't bet a
> product on it without that.

---

## Why cuicui?

Qt is great, but it is big. Flutter and React Native bring their own runtime, language
or rendering engine. cuicui takes a different, much smaller route:

- **Plain C++** — your whole app (logic *and* UI) is standard C++. No DSL, no code
  generator, no VM.
- **Native views, not a canvas** — every widget that needs to be visible becomes a real
  platform view (`android.view.View`, `UIView`, XAML element, DOM element). You get native
  text rendering, native text input, native scrolling, accessibility basics and the
  platform look for free. Pure layout containers stay virtual and cost nothing.
- **One layout engine** — sizes are expressed in `em` (relative to the font size),
  pixels, `FILL` or `MIN` (wrap content). Layout is computed once in C++ and pushed
  to the native views, so the result is the same everywhere.
- **Tiny** — the core is a handful of `.cpp` files plus a thin per-platform bridge
  (one Java activity on Android, a JS/CSS file on the Web). No dependency besides
  each platform's SDK (and emscripten for the Web).
- **Modules** — anything platform-specific that not every app needs (biometrics,
  OpenGL view, maps, …) lives in an optional module, in its own repo.

## What it looks like

```cpp
#include "cuicui_core/cuicui.h"
using namespace cui;

// The only function your app must provide.
void InitMyApp()
{
    master = new wEmpty();
    master->Xem(FILL)->Yem(FILL)->BgColor(0x60A0FF);

    master->Content([](Children& dc) {
        dc += wRows::Create(FILL, FILL)->Content([](Children& dc) {
            dc += wText::Create(FILL, ems(2))->SetText("Hello cuicui", 0x000000, true);

            dc += wText::Create(MIN, MIN)->SetText("Tap me")->Center()
                    ->Clickable(1, [](wWidget*) {
                        Log("tapped!");
                    });

            dc += wTextInput::Create(FILL);   // height is automatic
        });
    });
}
```

> Everything lives in the `cui` namespace; platform macros are `CUICUI_*`; widgets keep their
> `w` prefix (`wText`, `wRows`, `wButton`, …). The API is not frozen yet: see
> the [CHANGELOG](CHANGELOG.md).

## Concepts

| Concept | In short |
|---|---|
| `InitMyApp()` | Your entry point. Create `master` (the root widget) and fill it. |
| Widgets | `wText`, `wImage`, `wTextInput`, `wScrollView`, `wButton…`, `wSwitch…`, `wSlider`, `wWebView`, `wGradient`, `wCircle`, … plus native-control samples (`wNativeSwitch`, `wNativeButton`, calendar, time picker, activity indicator). |
| Containers / layout | `wEmpty` (free area / stack), `wRows`, `wColumns`, `wPadding`, `wBorder`, `wSafeZone`, `wStackPanel`, `wTabPanel`, gaps, alignment helpers (`Center`, `AlignRight`, `AlignBottom`). |
| Sizes | `ems(n)`, `rems(n)`, `pixels(n)`, `FILL`, `MIN`, `SAME_AS_X`. `ChangeEmRatio()` scales a whole subtree. |
| Children | Declared in a lambda: `->Content([](Children& dc){ dc += child; })`. |
| Events | `std::function` callbacks: `Clickable`, `OnGesture_Tap / Pan / Pinch / Swipe / LongPress`, pointer events, `onSizeChanged`, hardware back button. |
| Threading | All UI code runs on the UI thread; use `RunOnUiThread(fn)` to come back to it. |
| Services | `central` (app state, file picker, camera, clipboard, safe area, keyboard, …), `timerManager`, `animationManager` (with easing), `WebRequest` (HTTP), `LocalStorage` (key/value). |

## Platforms

| Platform | Native layer | Status |
|---|---|---|
| Android | Java `View`s through JNI | ✅ used in production |
| iOS | `UIView` (Objective-C++) | ✅ used in production |
| Windows (UWP) | XAML | ✅ used in production (UWP itself is legacy) |
| Web | DOM via emscripten | ✅ works; a few widgets/features missing (e.g. `wGradient`, some `central` features) |
| macOS / Linux / Win32 desktop | — | ❌ not supported |

Details per widget and feature: [docs/SUPPORT.md](docs/SUPPORT.md).

## Modules

cuicui keeps the core small. Optional features are **modules**: separate repos you
drop into a `modules/` folder next to `cuicui_core/`, then add to your platform projects.

| Module | What | Status |
|---|---|---|
| [cuicui_biometricAuth](https://github.com/mkdotcom/cuicui_biometricAuth) | Fingerprint / face prompt | Android only |
| [cuicui_wOpenGL](https://github.com/mkdotcom/cuicui_wOpenGL) | OpenGL ES view widget | Experimental: Android (manual loop), iOS, UWP |
| [cuicui_wMap](https://github.com/mkdotcom/cuicui_wMap) | Map widget | Experimental |

The module contract (what a module provides, how it plugs into each platform) is
described in [docs/MODULES.md](docs/MODULES.md).

## Getting started

Start from the project template, [cuicui_project](https://github.com/mkdotcom/cuicui_project):
it contains ready-made Android Studio / Xcode / Visual Studio / emscripten projects.

```text
my_app/
├── app/            your .cpp/.h files and assets
├── cuicui_core/    this repo
├── modules/        optional modules
└── projects/       android/ ios/ uwp/ web/
```

Then add your `.cpp` files to the project of each platform you target and build with
the usual tool (Android Studio, Xcode, Visual Studio, or the `_compile` script for the Web).

More in the developer guide: [docs/GUIDE.md](docs/GUIDE.md).

## Roadmap to 1.0

- English-only, consistent naming (`cui::` namespace, `CUICUI_*` macros)
- A standalone CMake build and an example gallery app
- A documented, automatic module integration
- CI builds for every platform
- Per-widget platform support matrix

Known issues: [docs/KNOWN_ISSUES.md](docs/KNOWN_ISSUES.md) · in-code TODOs: [docs/TODO_INVENTORY.md](docs/TODO_INVENTORY.md)

## License

[MIT](LICENSE). Bundled third-party code keeps its own license, see
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
