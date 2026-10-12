/*
#include <SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL_timer.h>
#include <log.h>
#include <chip8.h>

static void process_input(struct chip8 *chip)
{
  static SDL_Event event;

  while(SDL_PollEvent(&event))
  {
    if(event.type == SDL_QUIT)
    {
      chip->running = 0;
      continue;
    }

    if(event.type != SDL_KEYDOWN && event.type != SDL_KEYUP)
      continue;

    unsigned char pressed = event.type == SDL_KEYDOWN;
    SDL_Keycode key = event.key.keysym.sym;

    if(key == SDLK_ESCAPE && pressed)
    {
      chip->running = 0;
      continue;
    }

    switch(key)
    {
      case SDLK_1: chip->key[0x1] = pressed; break;
      case SDLK_2: chip->key[0x2] = pressed; break;
      case SDLK_3: chip->key[0x3] = pressed; break;
      case SDLK_4: chip->key[0xC] = pressed; break;

      case SDLK_q: chip->key[0x4] = pressed; break;
      case SDLK_w: chip->key[0x5] = pressed; break;
      case SDLK_e: chip->key[0x6] = pressed; break;
      case SDLK_r: chip->key[0xD] = pressed; break;

      case SDLK_a: chip->key[0x7] = pressed; break;
      case SDLK_s: chip->key[0x8] = pressed; break;
      case SDLK_d: chip->key[0x9] = pressed; break;
      case SDLK_f: chip->key[0xE] = pressed; break;

      case SDLK_z: chip->key[0xA] = pressed; break;
      case SDLK_x: chip->key[0x0] = pressed; break;
      case SDLK_c: chip->key[0xB] = pressed; break;
      case SDLK_v: chip->key[0xF] = pressed; break;
    }
  }
}

static void update_framebuffer(SDL_Surface *framebuffer, const struct chip8 *chip)
{
  if(SDL_LockSurface(framebuffer) < 0)
    return;

  chip8_mem_t *pixels = framebuffer->pixels;

  for(uint32_t i = 0; i < CHIP8_DISPLAY_SIZE; i++)
  {
      const uint32_t color = chip->gfx[i]
        ? CHIP8_FOREGROUND_COLOR
        : CHIP8_BACKGROUND_COLOR;

      pixels[i] =
        SDL_MapRGBA(
            framebuffer->format,
           (color >> 16) & 0xFF,
           (color >> 8)  & 0xFF,
           (color)       & 0xFF,
                           0xFF
            );

  }

  SDL_UnlockSurface(framebuffer);
}

int main(int argc, char *argv[])
{
  if(argc != 2)
  {
    LOG_INFO_ARGS("USAGE: %s <ROM.C8>", argv[0]);
    return 1;
  }

  if(SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    LOG_ERROR_ARGS("SDL_Init failed: %s", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow(
      "CHIP-8",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      CHIP8_DISPLAY_WIDTH * CHIP8_DISPLAY_SCALE,
      CHIP8_DISPLAY_HEIGHT * CHIP8_DISPLAY_SCALE,
      SDL_WINDOW_SHOWN);

  if(window == NULL)
  {
    LOG_ERROR_ARGS("SDL_CreateWindow failed: %s", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_Surface *surface = SDL_GetWindowSurface(window);

  SDL_Surface *framebuffer = SDL_CreateRGBSurfaceWithFormat(
      0,
      CHIP8_DISPLAY_WIDTH,
      CHIP8_DISPLAY_HEIGHT,
      32,
      SDL_PIXELFORMAT_RGBA32);

  if(surface == NULL || framebuffer == NULL)
  {
    LOG_ERROR_ARGS("SDL surface creation failed: %s", SDL_GetError());

    if(framebuffer != NULL)
      SDL_FreeSurface(framebuffer);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  struct chip8 *chip = chip8_init();

  chip8_load_rom(chip, argv[1]);
  chip->running = 1;

  uint32_t last_cycle_time = SDL_GetTicks();

  uint32_t cycle_accumulator = 0;
  uint32_t timer_accumulator = 0;

  if(Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048) < 0)
  {
    LOG_ERROR_ARGS("Mix_OpenAudio failed: %s", Mix_GetError());
    return 1;
  }

  Mix_Chunk *beep = Mix_LoadWAV("beep.wav");
  if(beep == NULL)
  {
    LOG_ERROR_ARGS("Mix_LoadWAV failed: %s", Mix_GetError());
    Mix_CloseAudio();
    return 1;
  }

  int beep_channel = -1;

  while (chip->running)
  {
    process_input(chip);

    uint32_t current_time = SDL_GetTicks();
    uint32_t delta = current_time - last_cycle_time;
    last_cycle_time = current_time;

    cycle_accumulator += delta;
    timer_accumulator += delta;

    while (timer_accumulator >= 16)
    {
      timer_accumulator -= 16;
      chip8_update_timers(chip);

      if (chip->sound > 0)
      {
        if (beep_channel == -1 || !Mix_Playing(beep_channel))
        {
          beep_channel = Mix_PlayChannel(-1, beep, -1);

          if (beep_channel < 0)
            LOG_ERROR_ARGS("Mix_PlayChannel failed: %s", Mix_GetError());
        }
      }
      else if (beep_channel != -1)
      {
        Mix_HaltChannel(beep_channel);
        beep_channel = -1;
      }
    }

    while (cycle_accumulator >= CHIP8_OP_TIME_MS)
    {
      cycle_accumulator -= CHIP8_OP_TIME_MS;
      chip8_cycle(chip);
    }

    if (chip->draw)
    {
      uint32_t now = SDL_GetTicks();
      chip->draw = 0;
      update_framebuffer(framebuffer, chip);
      SDL_BlitScaled(framebuffer, NULL, surface, NULL);
      SDL_UpdateWindowSurface(window);
      printf("%d\n", SDL_GetTicks() - now);
    }
  }

  SDL_FreeSurface(framebuffer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  chip8_free(chip);
  return 0;
}
*/
