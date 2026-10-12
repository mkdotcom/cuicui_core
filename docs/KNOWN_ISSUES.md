# Known issues

Problems known but not fixed yet. Each one should become a GitHub issue on release day
(then this file can be removed, or keep only a link to the issue tracker).

## Bugs

| Platform | Where | Issue |
|---|---|---|
| Android | `central.cpp` `AndroidSimpleToast` | Crashes when called from a `GestureTap` callback. |
| Android | `cuicui.cpp` (wGradient) | Color stop positions are ignored (only evenly spaced colors). |
| iOS | `cuicui.cpp` (wGradient) | The gradient layer is sized at creation: a gradient created before its first layout stays empty. |
| UWP | `cuicui.h` (`wSwitch`) | Only the dot is clickable, and the switch is not pixel-perfect (rounding?). |
| UWP | `wText.cpp` (character position) | A position after the end of the text returns the last character instead of "none". |
| Web | `cuicui.cpp` (layout) | A widget not yet in the DOM is measured wrong: layout before attaching gives wrong sizes. |
| Web | `wTextInput.cpp` (multiline) | Auto-grow works when the text grows, not when it shrinks. |
| All | `wImage.cpp` `Source` | While an image loads, the whole widget is transparent (background color too), and a user-set opacity is lost. |
| All (debug) | `cuicui.cpp` `RefreshUI_Private` | Debug layout statistics are global: several scroll views overwrite each other's values. |

## Missing features

| Platform | Feature |
|---|---|
| Web | `wGradient` |
| All | `eAppState::Background` received when the app is killed: confirmed on UWP only, to validate (and document) elsewhere. |

See [SUPPORT.md](SUPPORT.md) for the per-platform support matrix and
[TODO_INVENTORY.md](TODO_INVENTORY.md) for all the in-code markers.
