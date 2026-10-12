# Changelog

All notable changes to cuicui are listed here. The API is not stable before 1.0: minor versions
may contain breaking changes.

## [0.1.0] — unreleased

First public release. cuicui had been developed and used in-house for years; this release cleans
it up for public use.

### Changed (breaking)
- Everything is in the `cui` namespace; platform macros are `CUICUI_ANDROID`, `CUICUI_IOS`,
  `CUICUI_UWP`, `CUICUI_WEB`, `CUICUI_DEBUG`.
- Main header: `cuicui_core/cuicui.h` (was `mkw.h`); `utils.h` / `platform.h` (were `stuff.h` /
  `ph.h`).
- Android: Java package `io.github.mkdotcom.cuicui`, base activity `CuicuiActivity`.
- Renamed types: `wWidget` (base widget), `Children` (the `dc` of `Content()`), `Length` (sizes),
  `BorderStyle`, `TimerManager` / `Timer`, `RunOnUiThread`, `OnWindowSizeChanged`.
- Widgets: `wButton`, `wSwitch`, `wBorder`, `wPadding`, `wButtonImage`, `wSliderVertical` are the
  former `*2` / `*3` versions; the old deprecated versions were removed.
- `FILL`, `MIN`, `UNSET`, `SAME_AS_X`, `PIXELS` are constants instead of macros.
- Native-control samples are in `cui::native` (was `Nws`).

### Added
- Leveled logging: `LogDebug`, `LogInfo`, `LogWarning`, `LogError`.
- Extension points for modules: `wWidget::OnNativeFrameChanged()`, `eCoreType::Custom`, Android
  `cuiGetElementView()` / `cuiGetElementWidgetPtr()` / `cuiNewViewId()`.
- `cuicui.cmake` to add the core to a CMake build.
- Android: native methods registered with `RegisterNatives` (the C++ side doesn't depend on the
  Java package name anymore).
- Web implementation of `native::wTimePicker` and of `wTextInput::DisableReturnKey()`.
- CI (GitHub Actions): web, Android and UWP builds.
- Documentation: README, developer guide, platform support matrix, modules.

### Removed
- The deprecated MIEM input system, cpp-httplib, the unlicensed easing library (replaced by our
  own implementation), app-specific code, and ~4,500 lines of commented-out code.

### Fixed
- UWP build (pan gesture), web build (`wTimePicker`).
- Android: assets are copied only when the app is installed or updated (was: at every start).
- Android: the placeholder of a `wTextInput` follows its text color (was often invisible).
- UWP: memory leak and missing `await` when receiving a picked / pasted / dropped file.
- Web: crash when pressing Enter in a destroyed `wTextInput`.
- Android: two JNI signature mismatches (`cuiGetBackButtonManagerPushedElementsCount`,
  `cuiSetCentralSafeAreaRect`).

### Changed (behavior)
- Web: Enter in a multiline `wTextInput` now inserts a new line (as on the other platforms), unless
  `DisableReturnKey()` was called.
