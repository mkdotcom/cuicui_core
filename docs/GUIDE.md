# cuicui developer guide

Practical notes for building apps with cuicui. For the big picture see the
[README](../README.md); for optional modules see [MODULES.md](MODULES.md).

> All C++ names are in the `cui` namespace (`using namespace cui;` in your app); native-control
> samples are in `cui::native`. The API is not frozen yet — see the [CHANGELOG](../CHANGELOG.md).

---

## Project setup

### Platform defines
Defined at project level (no header needed):

| Define | Platform |
|---|---|
| `CUICUI_ANDROID` | Android |
| `CUICUI_IOS` | iOS |
| `CUICUI_UWP` | Windows (UWP) |
| `CUICUI_WEB` | Web (emscripten) |
| `CUICUI_DEBUG` | debug builds, on every platform |

`assert()` / `cui_assert()` stop in the debugger (or crash) in debug builds and do nothing in
release builds. On UWP, `NDEBUG` must be defined in the release configuration.

### Adding your .cpp files
Each platform project must list your source files:

- **Android**: add them to `CMakeLists.txt` (`app/*.cpp` is picked up automatically in the template).
  The core itself is added with `include(.../cuicui_core/cuicui.cmake)` + `cuicui_add_sources(native-lib)`.
- **iOS**: add them to the Xcode project, then set their type to *Objective-C++ Source*
  (right panel). cuicui headers contain Objective-C code under `CUICUI_IOS`.
- **UWP**: add them to the Visual Studio project.
- **Web**: add them to `your_app_cpp_files.txt`.

### Assets (images, …)
Put them in `app/assets/`. They are copied to each platform project:
Android copies them automatically at build time (Gradle task); the web and UWP projects use
the `deploy_assets_*` scripts; Xcode reads them directly from the app folder.
After adding files, Visual Studio may need a restart to see them.

### Strings and Unicode
cuicui uses `std::string` everywhere, containing UTF-8. That's fine as long as you don't use
`.length()` to count characters or iterate byte by byte expecting one byte per character.

### Icons and store assets
- Android: everything is uploaded to the Play Store.
- Web: put the favicon next to the site and reference it in `index.html`
  (`<link rel="icon" type="image/png" href="fav_icon_png.png">`).
- UWP: `Package.appxmanifest` → *Visual assets* in Visual Studio.
- iOS: asset catalog in Xcode. Tools such as https://appicon.co/ generate all sizes.

### Screen orientation
- Android: `android:screenOrientation="portrait"` (or `landscape`) in the manifest.
- iOS: checkboxes in the target settings.
- UWP: checkboxes in `Package.appxmanifest`.
- Web: only on mobile, through the Screen Orientation API (fullscreen); not on desktop.

---

## Layout

- Sizes are `ems(n)`, `rems(n)`, `pixels(n)`, `FILL`, `MIN` (wrap content), `SAME_AS_X`.
- Padding: use `wPadding` (and its horizontal/vertical variants).
- Borders: use `wBorder`, or `yourWidget->Border(...)`. Rounded borders clip their content on
  every platform (Android uses `setClipToOutline`). Semi-transparent borders are not handled.
- **Refreshing the UI after a change**:
  - `RefreshUI_Fast(widget)`: only when the widget's size does **not** change;
  - `RefreshUI_Everything()`: otherwise (recomputes the whole tree).
- The OS-wide font size / display scale (accessibility settings) is honored: the app is
  re-laid out (UWP, iOS, Web via browser zoom; Android may restart the activity).

## Touch and click events

- Touches go **through** widgets, except when the widget is `Touchable()`, has called
  `SetOpaqueToTouch()`, or is a native control (text input, native button, …).
  A background color does not make a widget opaque to touches — on purpose.
- Gestures: call `Touchable()` then `OnGesture_Tap(...)`, `OnGesture_Pan(...)`,
  `OnGesture_Pinch(...)`, `OnGesture_Swipe(...)`, `OnGesture_LongPress(...)`
  (see `wBase.h` for the full list).
- `Clickable(type, callback)` is the simple way to make a button.
- "Entered" / "exited" pointer events don't exist on touch devices; cuicui simulates them
  around press/release.
- Rich text spans (`cSpan`) can be touchable. On UWP and iOS the `wText` must be `Touchable()`
  for span touches to work. Android and iOS only report the "up" event for spans.

### Hardware back button
`yourButton->LinkToHardwareBackButton()` triggers the button when the user presses back
(Android back button, Windows+Backspace on UWP, browser back on the web; iOS has none).
Only one such button should exist at a time. A `(0,0)`-sized button works if you only want the
action.

## Text

- `wText` without wrapping cuts text that is too long (no "…" by default).
- `wTextInput`: `onEnterKeyPressed` is **not available on iOS** yet.

## Threads

Everything in cuicui runs on the UI thread. Use `RunOnUiThread(fn)` to come back to it
from another thread. `std::thread` works on every platform except the Web, where it requires
`-s USE_PTHREADS=1` and browser support (largely untested).

## Logging

```cpp
LogDebug("only in debug builds");      // also: Log(...), LogD(...)
LogInfo("always written");             // also: LogR(...)
LogWarning("something looks wrong");
LogError("something failed");
```

Output: logcat on Android (tag `cuicui`, with the matching priority), the browser console on the
Web (`console.warn` / `console.error` for warnings and errors), `NSLog` on iOS, the Visual Studio
output window on UWP. `LogDebug` compiles to nothing unless `CUICUI_DEBUG` is defined.

## Local storage

`LocalStorage::GetInstance()->Write(tag, value)` / `Read(tag)` store UTF-8 strings.
Limits: tag ≤ 254 characters, value ≤ 4095 characters (UWP limit, applied everywhere).
Use letters, digits and `_` in tags.

## Network: WebRequest

`WebRequest` wraps each platform's native HTTP stack, so HTTPS works without linking OpenSSL
(Android `HttpURLConnection`, iOS `NSURLSession`, UWP `HttpClient`, Web `fetch`).
iOS refuses plain-HTTP requests unless `NSAppTransportSecurity` is configured in `Info.plist`.

## Receiving files: shares, drops, camera

- Files arrive through the `onFileReceived()` callback, even when the app is in background:
  - **Android**: shares of the types declared in the manifest;
  - **UWP & Web**: drag & drop on a widget that called `EnableDropFile()`, or paste on a
    `wTextInput` that called `EnablePasteFile()` (clipboard images; clipboard files on UWP only);
  - **camera** photos (see below);
  - iOS: not supported yet.
- The callback receives a `std::shared_ptr<cFileReceived>` (`fileName`, `mimeType`,
  `base64Data`, `GetAsRawBytes()`). `base64Data` contains base64 for images, plain text for text.
- The MIME type may be wrong or empty (missing / wrong file extension): check the content.
- **Initial payload**: if the app was *not running* when the share arrived, there is no
  `onFileReceived()` call; check `central.initialPayload` in `InitMyApp()` instead (Android;
  UWP when arguments are passed to the app; never on the Web; not handled on iOS yet).

### Camera
The photo is delivered through `onFileReceived()`.

- **Android**: declare in the manifest
  `<uses-feature android:name="android.hardware.camera" android:required="false" />` and a
  `FileProvider` whose authority is **`${applicationId}.fileprovider`**:
  ```xml
  <provider
      android:name="androidx.core.content.FileProvider"
      android:authorities="${applicationId}.fileprovider"
      android:exported="false"
      android:grantUriPermissions="true">
      <meta-data
          android:name="android.support.FILE_PROVIDER_PATHS"
          android:resource="@xml/file_paths" />
  </provider>
  ```
- **iOS**: nothing to do (optionally customize `NSCameraUsageDescription` in `Info.plist`).
- Other platforms: not available.

## Platform notes

### Android
- The activity must use `android:launchMode="singleTask"` (cuicui relies on a single instance,
  e.g. for receiving shares).
- C++ pointers are stored on the Java side as `long` (64-bit), which works on 32- and 64-bit devices.
- Keyboard resize: the activity sets `SOFT_INPUT_ADJUST_RESIZE`.
- The core's native methods (Java -> C++) are registered in `JNI_OnLoad` with `RegisterNatives`
  (table at the end of `android/native-lib.cpp`): a missing or renamed method makes the library
  fail to load, instead of crashing at its first call.
- Theme: `Theme.Black.NoTitleBar` shows a dark screen at startup, `Theme.Light.NoTitleBar` a
  white one.
- To link a prebuilt library, add it to `target_link_libraries` in `CMakeLists.txt`; to add an
  include folder, add `-I<path>` to `cppFlags` in `build.gradle`.

### Web
- Your own JavaScript can be added with a `<script src="...">` in the `<head>` of `index.html`.
- **PWA**: wait for the `webOnPwaReadyToBeInstalled()` callback (it comes only once the browser
  considers the site installable: HTTPS, some usage time, …; it can come at any time, and never
  again once installed), then call `EM_ASM({ webPwaInstall(); });` to show the native install
  prompt (`webPwaInstall()` is a JS function defined in the project's `index.html`). The service worker file must be served as `application/javascript`.
- Search engines: touchable widgets are not real links (`<a>`), and the content is rendered
  client-side, so crawling is limited.

### UWP
- Event handlers can be C++/CX lambdas, as long as they don't capture the UWP control's own
  `this` (risk of reference cycles).
- To make Visual Studio pick up a whole assets folder, an
  `<Image Include="mkw_assets\*"/>` item can be added to the `.vcxproj`.

### Notifications
Push notifications are not part of the core: apps use the Firebase SDK directly
(Android and iOS). Windows and Web notifications are not covered.

## Updating cuicui in an app
`cuicui_core` is a separate folder (ideally a git submodule) next to your `app/` folder:
update it independently of your code.
