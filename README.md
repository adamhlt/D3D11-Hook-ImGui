```
            ____ _____ ____ ______   __  __            __      ____          ______      _ 
           / __ \__  // __ <  <  /  / / / /___  ____  / /__   /  _/___ ___  / ____/_  __(_)
          / / / //_ </ / / / // /  / /_/ / __ \/ __ \/ //_/   / // __ `__ \/ / __/ / / / / 
         / /_/ /__/ / /_/ / // /  / __  / /_/ / /_/ / ,<    _/ // / / / / / /_/ / /_/ / /
        /_____/____/_____/_//_/  /_/ /_/\____/\____/_/|_|  /___/_/ /_/ /_/\____/\__,_/_/
                                                                       
                                                                      
                           D3D11 Hook with ImGui integration (x86 / x64)
                                Make D3D11 internal menu with ImGui
                                   (Resize & Fullscreen support)
```
<p align="center">
    <img src="https://img.shields.io/badge/language-C%2B%2B-%23f34b7d.svg?style=for-the-badge&logo=appveyor" alt="C++">
    <img src="https://img.shields.io/badge/platform-Windows-0078d7.svg?style=for-the-badge&logo=appveyor" alt="Windows">
    <img src="https://img.shields.io/badge/arch-x86-red.svg?style=for-the-badge&logo=appveyor" alt="x86">
    <img src="https://img.shields.io/badge/arch-x64-green.svg?style=for-the-badge&logo=appveyor" alt="x64">
</p>

## :open_book: Project Overview :

D3D11 Hook with [ImGui](https://github.com/ocornut/imgui) integration written in C++, works on x86 and x64 with all librairies and includes.

#### Features :

- Handle window resize
- DLL unload itself
- Handler external unload
- Handle fullscreen change

#### Used librairies :

- [MS Detours](https://www.microsoft.com/en-us/research/project/detours/)
- [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
- [ImGui](https://github.com/ocornut/imgui)

## :rocket: Getting Started

This project use [MS Detours](https://github.com/microsoft/Detours) library for hooking and obviously D3D11 SDK.

MS Detours library is already include in the project folder, but if you don't have DirectX SDK you can download it [here](https://www.microsoft.com/en-us/download/details.aspx?id=6812).

> **Note** <br>
> Make sure that **DXSDK_DIR** is declared in your environment variables.

To see your environment variables :

> **Settings --> System --> About --> System Advanced Settings --> Environment Variables**

### Visual Studio :

1. Open the solution file (.sln).
2. Build the project in Release (x86 or x64)

Every configuration in x86 / x64 (Debug and Realese) are already configured with librairies and includes.

## :test_tube: Demonstration :

[Demo](https://github.com/adamhlt/D3D11-Hook-ImGui/assets/48086737/5c6cf425-6758-4773-aa1c-9ce4b1c8b41d)
