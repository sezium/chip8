# CHIP-8

A CHIP-8 emulator written in **C99**, using **SDL2** for graphics and input handling and **SDL2_mixer** for audio playback.

The emulator implements the CHIP-8 instruction set and features:

- 64×32 monochrome display.
- CHIP-8 hexadecimal keypad mapping.
- Sound timer with beep audio playback.
- ROM loading through command-line arguments.
- 500HZ clock speed

## Technologies

- **C99** — emulator implementation.
- **CMake** — build system and project configuration.
- **SDL2** — window management, graphics, and keyboard input.
- **SDL2_mixer** — audio playback.
- **Nix** — reproducible development environment.

## Requirements

The following dependencies are required to build the project:

- C compiler with C99 support, such as GCC or Clang.
- CMake 3.20 or newer.
- Make.
- SDL2.
- SDL2_mixer.

If you use Nix, the development environment can provide the required dependencies.

## Build and Run

Enter the development environment:

```bash
nix develop
```

Configure and build the project:

```bash
cmake -S . -B build
cmake --build build
```

Run the emulator with a ROM:

```bash
./bin/CHIP8 rom/tetris.ch8
```

You can load any compatible CHIP-8 ROM by passing its path as a command-line argument.

## Preview

https://github.com/user-attachments/assets/e030279a-a15b-4794-87c1-5a15ef11fd6a
