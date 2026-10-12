# cuicui modules

cuicui keeps its core small: anything that is platform-specific **and** not needed by
every app (biometrics, an OpenGL surface, maps, payments, …) belongs in a **module**.
This document describes how modules work today, the contract they should follow,
and the state of the existing ones.

> Status: the *principle* is in place and used in production, but integration is
> still manual and a few modules leak into the core. Sections marked **Target** describe
> what we want for 1.0; sections marked **Today** describe what actually exists.

---

## 1. What is a module?

- A **separate git repository** named `cuicui_<name>` (e.g. `cuicui_biometricAuth`).
- Checked out into a `modules/` folder at the root of the app project, next to
  `cuicui_core/`:

  ```text
  my_app/
  ├── app/
  ├── cuicui_core/
  ├── modules/
  │   ├── cuicui_biometricAuth/
  │   └── cuicui_wOpenGL/
  └── projects/
  ```
- It contains C++ code (always) and, when needed, platform glue (Java for Android,
  Objective-C++ for iOS, JS for the Web, NuGet/XAML for UWP).
- It comes in one of two flavours:
  - **Widget module**: a class deriving from the core widget base class
    (`wWidget`), created like any other widget: `dc += wOpenGl::Create(FILL, FILL);`
  - **Service module**: a singleton exposing an async API with `std::function`
    callbacks: `cuicui_biometricAuth::GetInstance()->PromptUser(cb);`

## 2. Today: how a module is integrated (manual)

1. `cd modules && git clone https://github.com/mkdotcom/cuicui_<name>`
2. `#include "modules/cuicui_<name>/<header>.h"` in the app.
3. Add the module's files to **each** platform project by hand:

| Platform | What you edit |
|---|---|
| Android | `CMakeLists.txt` (add `.cpp`, link libs such as `GLESv2`/`EGL`), `build.gradle` (`java.srcDirs += 'modules/…'`, dependencies such as `androidx.biometric`), `AndroidManifest.xml` (permissions/features), sometimes `MainActivity.java` (instantiate the module's Java class in `onCreate`) |
| iOS | Xcode project: add files (type *Objective-C++ source*), frameworks (`.xcframework`) |
| UWP | `.vcxproj` + `.filters`, NuGet packages |
| Web | `your_app_cpp_files.txt`, emscripten flags (e.g. `-s USE_WEBGL2=1`), `index.html` snippets |

Each module's README documents these steps.

### Today's implicit conventions

- Platform code is selected with `#if defined(CUICUI_ANDROID) / CUICUI_IOS / CUICUI_UWP / CUICUI_WEB`,
  and an unsupported platform ends in `#else non_implemented_platform` (a deliberate
  compile error).
- Android: the module's Java class lives in its own sub-package of the core package
  (e.g. `io.github.mkdotcom.cuicui.wopengl`); C++ calls it through JNI by class name, and Java
  calls back through `Java_io_github_mkdotcom_cuicui_<subpackage>_<Class>_<method>` exports.
- Widget modules set a `coreType` from the core's closed `eCoreType` enum.

### Known problems (to fix before 1.0)

- ~~**The core depends on a module.**~~ Fixed: the core no longer references
  `cuicui_wOpenGL` (see the extension points below).
- ~~**Modules depend on the app.**~~ Fixed: `cuicui_biometricAuth` now takes any
  `FragmentActivity`.
- ~~A new widget module needs an `eCoreType` value.~~ Fixed: module widgets use `eCoreType::Custom`.
- Integration is entirely manual and differs per platform; READMEs drift
  (the biometric README has the wrong class name, file name and dependency version).
- No versioning: nothing says which core version a module works with.

## 3. Target contract (1.0)

### Rule #1: the core knows no module
No module name, include or type appears in `cuicui_core`. The core only offers
**generic extension points**:

- **Widget hooks** on the base widget class. Available today:
  `virtual void OnNativeFrameChanged(double width, double height)`, called at the end of
  `SetPos_HW()` once the native view has its new frame (used by `cuicui_wOpenGL` on iOS).
  Possible later: `OnAttachedToNative()` / `OnDetachedFromNative()`.
- **Android element access** (available today, on the core activity): `cuiGetElementView(id)`
  returns the native container view of a widget, `cuiGetElementWidgetPtr(id)` its C++ pointer,
  `cuiNewViewId()` a unique id for a view created by a module. See
  `cuicui_wOpenGL/android/io/github/mkdotcom/cuicui/wopengl/CuicuiWOpenGL.java` for an example.
- **`eCoreType::Custom`** for widgets defined outside the core (available today).
- **Android host interface**: a small Java interface implemented by modules and
  registered by the core activity:

  ```java
  public interface CuicuiModule {
      void onCreate(FragmentActivity activity);
      default void onDestroy() {}
      default boolean onActivityResult(int requestCode, int resultCode, Intent data) { return false; }
      default void onRequestPermissionsResult(int requestCode, String[] p, int[] r) {}
  }
  ```
  Modules receive a `FragmentActivity`, never the app's own `MainActivity` class.
- **JNI**: each module owns its Java class in its own package. Its natives can use the standard
  `Java_<package>_<Class>_<method>` names (what the current modules do) or `RegisterNatives`
  (what the core does); avoid `FindClass` from callback threads (cache a global `jclass`).
- **Lifecycle**: `central.onAppStateChanged` is already available; modules must
  release native resources (global refs, GL contexts) on destroy.

### Rule #2: one manifest per module
Each module ships the integration snippets so that a project adds a module in one line
per platform:

| File | Used by |
|---|---|
| `cuicui_module.cmake` → `add_cuicui_module(<name>)` | Android (and later a standalone CMake build) |
| `cuicui_module.gradle` (srcDirs, dependencies) | Android |
| `AndroidManifest.xml` fragment (merged by Gradle) | Android |
| `sources.txt` | Web (`_compile`), and as the reference list for Xcode / vcxproj |
| `README.md` | Everything that can't be automated (Xcode, NuGet, `index.html`) |

### Rule #3: API conventions

- Header: `#include "cuicui_core/cuicui.h"` (never a relative `../../` path).
- C++ in the `cui` namespace; widget classes keep the `w` prefix (`cui::wOpenGL`).
- Platform macros: `CUICUI_ANDROID`, `CUICUI_IOS`, `CUICUI_UWP`, `CUICUI_WEB`.
- `static bool IsSupported()` at runtime, so apps don't hard-code platform lists.
- Unsupported platforms: compile, return "not supported" at runtime, and say so in the
  README support table (instead of a compile error), unless the module is meaningless
  there.
- Results are documented enums, not raw ints or strings.
- Document on which thread each callback is called (UI thread by default).
- No user-visible string hard-coded in platform code: make it a parameter.
- English only; MIT license; a `LICENSE` file; third-party notices when vendoring.
- Declare the minimum core version in the README (and later in the manifest).
- No large binaries in git (use release assets, Git LFS or a package manager).

### Rule #4: README template

Every module README contains: purpose · status banner · platform support table ·
install (per platform) · usage example · API reference · known issues · license.

## 4. Existing modules

| Module | Kind | Android | iOS | UWP | Web | Notes |
|---|---|---|---|---|---|---|
| `cuicui_biometricAuth` | service | ✅ | ❌ | ❌ | ❌ | Known issues: the callback may report a failed attempt (`0`) before the final result; prompt texts are hard-coded (English) |
| `cuicui_wOpenGL` | widget | ⚠ manual loop only | ✅ | ✅ (ANGLE) | ❌ | **Not published yet** |
| `cuicui_wMap` | widget | ❌ | ❌ | ⚠ Bing Maps (service retired) | ⚠ Google Maps | Experimental |

