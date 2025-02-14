# SmartStart Recording

## Introduction

Automatic Scene-Based Recording for OBS

This plugin for OBS Studio allows you to start recordings automatically for each scene after a predefined time. It intelligently considers events, ensuring recordings begin at the right moment without manual intervention. Perfect for workflows that require precise scene-based recording automation.

## Supported Build Environments

| Platform  | Tool   |
|-----------|--------|
| Windows   | Visal Studio 17 2022 |
| macOS     | XCode 16.0 |
| Windows, macOS  | CMake 3.30.5 |
| Ubuntu 24.04 | CMake 3.28.3 |
| Ubuntu 24.04 | `ninja-build` |
| Ubuntu 24.04 | `pkg-config`
| Ubuntu 24.04 | `build-essential` |

## Downolad

[obsproject.com](https://obsproject.com/forum/resources/smartstart-recording.2090/)

## Build

This project uses CMake with build presets defined in `CMakePresets.json`.  
To configure and build the project, use the appropriate preset for your platform.  

For example, on Windows 10/11 (64-bit):  
```sh
cmake --preset windows-x64
```

## Donations

Donation link coming soon