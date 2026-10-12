# Contributing to cuicui

Thanks for your interest! cuicui is a small project maintained on spare time: issues and pull
requests are welcome, answers may take a while.

## Reporting a bug
Open an issue with:
- the platform(s) (Android / iOS / UWP / Web) and OS / browser version,
- a minimal piece of C++ (ideally an `InitMyApp()` based on
  [cuicui_project](https://github.com/mkdotcom/cuicui_project)) that shows the problem,
- what you expected and what happened (screenshots help).

Check [docs/SUPPORT.md](docs/SUPPORT.md) first: some features are known to be missing on some
platforms.

## Pull requests
- One topic per pull request; describe what you changed and on which platforms you tested it.
- cuicui supports 4 platforms: if your change touches platform code (`#if defined(CUICUI_...)`),
  say which platforms you could build and run. Untested platforms are fine, just say so.
- Code and comments in English.
- Follow the surrounding style: tabs for indentation, widgets named `wSomething`, everything in
  the `cui` namespace, platform macros `CUICUI_*`.
- Don't reformat code you don't change.
- Use `LogDebug` / `LogWarning` / `LogError` for logs, not `printf` / `console.log` / `NSLog`.
- Mark unfinished work with `TODO:` (`PERF:` for performance, `CLEANUP:` for refactoring).

## Modules
New platform-specific features usually belong in a module (a separate repository) rather than in
the core: see [docs/MODULES.md](docs/MODULES.md).

## License
By contributing, you agree that your contributions are licensed under the MIT license.
