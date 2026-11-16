<div align="center">

# Elysian Shadows

### The engine, toolkit, and tech behind *Elysian Shadows* — now open source.

[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Kickstarter backers](https://img.shields.io/badge/Kickstarter%20backers-no%20strings%20attached-orange.svg)](LICENSE)
[![Status](https://img.shields.io/badge/status-needs%20love-red.svg)](#heads-up)
[![PRs welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](#contributing)

</div>

---

## To our Kickstarter backers

You believed in this project when it was just a dream and a Dreamcast. This release is for you.

We're opening up the full *Elysian Shadows* codebase — engine, toolkit, and the cross-platform libraries underneath them — with its complete development history. It's MIT licensed for everyone, and **if you backed the Kickstarter, you can do whatever you want with it, no license terms attached.** See [LICENSE](LICENSE) for details.

## Heads up

Before you dive in, know what you're getting:

- **Assets are private.** The game assets (art, music, maps, scripts) are a private submodule at `assets/`. Without access, the code still builds, but there is no game to load.
- **Only ESToolkit on Linux is known to build.** It builds cleanly with GCC 15 and Qt 6 (6.2 or newer). The only supported targets are Linux (Debian/Ubuntu) and the Sega Dreamcast; the Dreamcast build hasn't been fixed up yet. Windows, macOS, iOS and Android support has been removed.
- **It could use some love.** Expect rough edges, dead code, and the occasional colorful comment.

## What's inside

| Component | What it is |
|---|---|
| **ESGamma** | The core *Elysian Shadows* game engine |
| **ESToolkit** | The ESTk level editor and content-creation toolkit (Qt) |
| **AssetIO** | Asset objects saved and loaded by the engine and toolkit |
| **game** | Game-specific engine code |
| **libGyro** | Cross-platform multimedia C API powering the engine and ESTk |
| **libGyro-Kinetic** | libGyro backend for the Sega Dreamcast |
| **QESTkUtils** | Shared Qt utilities and widgets |

Third-party libraries (sol2, the Spine runtime, the Dreamcast SDK bits under `libGyro/lib/dc/`, and the Dreamcast tools under `libGyro/tools/dc/`) remain under their own licenses.

## Building

ESToolkit builds with CMake and Qt 6. Linux is the tested platform.

### 1. Get the source

sol2 (the C++/Lua bindings) and the game assets are git submodules, so clone recursively:

```sh
git clone --recursive git@github.com:Bruceleeto/elysian-shadows.git
```

In an existing clone, run `git submodule update --init` instead.

### 2. Install dependencies

On Debian or Ubuntu:

```sh
sudo apt install build-essential cmake pkg-config qt6-base-dev qt6-base-dev-tools liblua5.5-dev \
    qt6-charts-dev qt6-tools-dev \
    libsdl2-dev libsdl2-net-dev libopenal-dev libogg-dev libvorbis-dev \
    zlib1g-dev
```

### 3. Build

From the repository root:

```sh
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

The toolkit ends up at `build/ESToolkit/ESToolkit/ESToolkit` and opens the repo's `assets/` on startup. All build output stays in the build folder, so to start over just delete it.

For the Dreamcast, source KallistiOS's `environ.sh` and run `kos-cmake ..` instead of `cmake ..`. That builds `ESGamma.elf`, but the Dreamcast build doesn't work yet.

Optional extras, passed to `cmake`:

- `-DESTK_DOCS=ON` bundles the Lua API docs (not tested recently).
- `-DCMAKE_BUILD_TYPE=Debug` for a debug build (the default is `RelWithDebInfo`).

### Lua binding checks

Run `ctest` in the build folder for the headless ownership, identity, property,
constructor and Qt conversion tests. Serialization checks also run when the
assets are checked out. For Lua 5.5 syntax checks, run
`tests/check_scripts <file.lua>...` from the build folder. None of these launch the editor.

### Qt Creator

Open `CMakeLists.txt` from the repository root.

## Contributing

**Contributions are very welcome.** Getting things building on modern toolchains, fixing bugs, cleaning up, writing docs, porting — all of it helps. Open an issue or send a pull request.

## License

MIT, with an additional permission letting Elysian Shadows Kickstarter backers use the code without any license conditions. See [LICENSE](LICENSE) and [AUTHORS](AUTHORS).

<div align="center">

*Made with love (and a lot of late nights) by the Elysian Shadows Team.*

</div>
