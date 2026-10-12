# CHIP-8

A CHIP-8 emulator written in C99 using SDL2 for graphics and input handling, and SDL2_mixer for audio.

The emulator implements the CHIP-8 instruction set, renders the 64×32 monochrome display, handles keyboard input through the standard CHIP-8 keypad mapping, and reproduces the sound timer using a beep sound.

ROMs can be loaded directly from the command line.

```bash
nix develop
cmake -S . -B ./build
make -C ./build && cd ./bin && ./CHIP8 ../rom/tetris.ch8 && cd ..
./bin/CHIP8 rom/tetris.ch8
```

## Demo

https://github.com/user-attachments/assets/e030279a-a15b-4794-87c1-5a15ef11fd6a
