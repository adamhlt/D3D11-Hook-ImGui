<div align="center">

# D3D11 Hook ImGui

**D3D11 hook in C++ (x86 / x64) to draw an ImGui menu inside a DirectX 11 process**

![Language](https://img.shields.io/badge/C%2B%2B-16171C?logo=cplusplus)
![Platform](https://img.shields.io/badge/Windows-16171C)
![Arch](https://img.shields.io/badge/x86%20%7C%20x64-16171C)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-16171C)](LICENSE)

</div>

---

D3D11 Hook ImGui is a DLL that hooks the DirectX 11 swap chain of the process it is loaded in, and renders an [ImGui](https://github.com/ocornut/imgui) menu on top of the game.

The hook survives window resize and fullscreen changes, and the DLL can unload itself without closing the process. The same code base builds for x86 and x64.

### DEMO
https://github.com/adamhlt/D3D11-Hook-ImGui/assets/48086737/5c6cf425-6758-4773-aa1c-9ce4b1c8b41d

## Features

**Hooks**
- `Present` hook to render the ImGui menu every frame
- `ResizeBuffers` hook, ImGui and the D3D11 objects are released and created again when the window is resized
- `SetFullscreenState` hook, same handling when the fullscreen state changes
- Window procedure hook to forward the inputs to ImGui

**Unload**
- Press `END` and the DLL removes its hooks and unloads itself
- Unload from the outside (for example `FreeLibrary` from an injector) is handled too
- Closing the window removes the hooks before the process terminates

**Menu**
- Press `INSERT` to show or hide the menu
- The menu position and size are kept after a resize or a fullscreen change
- Add your own widgets in `DrawMenu` in `src/Drawing.cpp`

## Usage

1. Build the DLL (see below).
2. Inject it in a process that uses DirectX 11, with the injector of your choice.
3. Use `INSERT` to toggle the menu and `END` to unload the DLL.

> **Note**
> The DLL must have the same architecture as the target process (x86 or x64).

## Build

1. Open `D3D11 Hook ImGui.sln` in Visual Studio (C++17).
2. Build in Release (x86 or x64).

- **Output:** every configuration is already set up, the DLL is named `D3D11 Hook ImGui.dll`
- **SDK:** the Windows SDK is enough, the legacy DirectX SDK and the `DXSDK_DIR` variable are no longer needed
- **Dependencies:** ImGui and MS Detours (headers and x86 / x64 libraries) are included in the repository

## Libraries

- [MS Detours](https://github.com/microsoft/Detours)
- [ImGui](https://github.com/ocornut/imgui)

## License

This project is licensed under the [GNU GPLv3](LICENSE).
