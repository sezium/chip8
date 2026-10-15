#include <SDL.h>
#include <SDL2/SDL_mixer.h>
#include <log.h>
#include <chip8.h>
#include <stdint.h>
#include <stdlib.h>


struct chip8_window
{
  SDL_Event     event;
  SDL_Renderer *renderer;
  SDL_Texture  *texture;
  SDL_Surface  *frame_buffer;
  SDL_Window   *window;
};

struct chip8_sound
{
  Mix_Chunk *beep;
  int        channel;

};

static void chip8_input(struct chip8 *chip)
{
  SDL_Event event;

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

static void chip8_window_update_framebuffer(SDL_Surface *framebuffer,
                               const struct chip8 *chip)
{
  if(SDL_LockSurface(framebuffer) < 0)
    return;

  uint32_t *pixels = framebuffer->pixels;

  for(uint32_t i = 0; i < CHIP8_DISPLAY_SIZE; i++)
  {
    const uint32_t color = chip->gfx[i]
      ? CHIP8_FOREGROUND_COLOR
      : CHIP8_BACKGROUND_COLOR;

      pixels[i] = SDL_MapRGBA(
        framebuffer->format,
       (color >> 16) & 0xFF,
       (color >> 8)  & 0xFF,
        color        & 0xFF,
        0xFF
    );
  }

  SDL_UnlockSurface(framebuffer);
}


struct chip8_window chip8_window_init(const char *title, const size_t H, const size_t W, const size_t SCALE)
{

  struct chip8_window chip8_win;

  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
  {
    LOG_ERROR_ARGS("SDL_Init failed: %s", SDL_GetError());
    exit(1);
  }

  chip8_win.window = SDL_CreateWindow(
      title,
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      W * SCALE,
      H * SCALE,
      SDL_WINDOW_SHOWN
      );

  if(chip8_win.window == NULL)
  {
    LOG_ERROR_ARGS("SDL_CreateWindow failed: %s", SDL_GetError());
    SDL_Quit();
    exit(1);
  }

  chip8_win.renderer = SDL_CreateRenderer(
      chip8_win.window, -1, SDL_RENDERER_ACCELERATED
      );

  if(NULL == chip8_win.renderer)
  {
    LOG_INFO_ARGS("Accelerated renderer unavailable: %s",
        SDL_GetError());

    chip8_win.renderer = SDL_CreateRenderer(
        chip8_win.window, -1, SDL_RENDERER_SOFTWARE
        );
  }

  if(NULL == chip8_win.renderer)
  {
    LOG_ERROR_ARGS("SDL_CreateRenderer failed: %s", SDL_GetError());
    SDL_DestroyWindow(chip8_win.window);
    SDL_Quit();
    exit(1);
  }

  SDL_RenderSetLogicalSize(
      chip8_win.renderer,
      W,
      H
      );

  chip8_win.texture = SDL_CreateTexture(
      chip8_win.renderer,
      SDL_PIXELFORMAT_RGBA32,
      SDL_TEXTUREACCESS_STREAMING,
      W,
      H
      );

  chip8_win.frame_buffer = SDL_CreateRGBSurfaceWithFormat(
      0,
      W,
      H,
      32,
      SDL_PIXELFORMAT_RGBA32
      );

  if(chip8_win.texture == NULL || chip8_win.frame_buffer == NULL)
  {
    LOG_ERROR_ARGS("Framebuffer creation failed: %s", SDL_GetError());

    if(chip8_win.frame_buffer != NULL)
      SDL_FreeSurface(chip8_win.frame_buffer);

    if(chip8_win.texture != NULL)
      SDL_DestroyTexture(chip8_win.texture);

    SDL_DestroyRenderer(chip8_win.renderer);
    SDL_DestroyWindow(chip8_win.window);
    SDL_Quit();
    exit(1);
  }

  return chip8_win;

}

void chip8_window_update(struct chip8_window *chip8_win, const struct chip8 *chip)
{

  chip8_window_update_framebuffer(chip8_win->frame_buffer, chip);
  SDL_UpdateTexture(
      chip8_win->texture,
      NULL,
      chip8_win->frame_buffer->pixels,
      chip8_win->frame_buffer->pitch
      );

  SDL_RenderCopy(chip8_win->renderer, chip8_win->texture, NULL, NULL);
  SDL_RenderPresent(chip8_win->renderer);
}

void chip8_window_destroy(struct chip8_window *chip8_win)
{
  SDL_FreeSurface(chip8_win->frame_buffer);
  SDL_DestroyTexture(chip8_win->texture);
  SDL_DestroyRenderer(chip8_win->renderer);
  SDL_DestroyWindow(chip8_win->window);
  SDL_Quit();

}

struct chip8_sound chip8_sound_init(const char *path)
{

  struct chip8_sound chip8_snd;

  if(Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048) < 0)
  {
    LOG_ERROR_ARGS("Mix_OpenAudio failed: %s", Mix_GetError());
    SDL_Quit();
    exit(1);
  }

  chip8_snd.beep = Mix_LoadWAV(path);

  if(chip8_snd.beep == NULL)
  {
    LOG_ERROR_ARGS("Mix_LoadWAV failed: %s", Mix_GetError());
    Mix_CloseAudio();
    SDL_Quit();
    exit(1);
  }

  chip8_snd.channel = -1;

  return chip8_snd;

}

void chip8_sound_update(struct chip8_sound *chip8_snd,
                        const struct chip8 *chip)
{
  if(chip->sound > 0)
  {
    if(chip8_snd->channel == -1 || !Mix_Playing(chip8_snd->channel))
    {
      chip8_snd->channel = Mix_PlayChannel(-1, chip8_snd->beep, -1);

      if(chip8_snd->channel < 0)
        LOG_ERROR_ARGS("Mix_PlayChannel failed: %s",
            Mix_GetError());
    }
  }
  else if(chip8_snd->channel != -1)
  {
    Mix_HaltChannel(chip8_snd->channel);
    chip8_snd->channel = -1;
  }
}

void chip8_sound_destroy(struct chip8_sound *chip8_snd)
{
  Mix_HaltChannel(-1);
  Mix_FreeChunk(chip8_snd->beep);
  Mix_CloseAudio();

}

int main(int argc, char *argv[])
{
  if(argc != 2)
  {
    LOG_INFO_ARGS("Usage: %s <rom.ch8>", argv[0]);
    return 1;
  }

  struct chip8_window chip8_win = chip8_window_init("CHIP8", CHIP8_DISPLAY_HEIGHT, CHIP8_DISPLAY_WIDTH, CHIP8_DISPLAY_SCALE);
  struct chip8_sound  chip8_snd = chip8_sound_init("beep.wav");

  struct chip8 *chip = chip8_init();

  chip8_load_rom(chip, argv[1]);
  chip->running = 1;

  uint32_t cycle_accumulator = 0;
  uint32_t timer_accumulator = 0;
  uint32_t last_time = SDL_GetTicks();

  while(chip->running)
  {
    chip8_input(chip);

    uint32_t current_time = SDL_GetTicks();
    uint32_t delta = current_time - last_time;
    last_time = current_time;

    cycle_accumulator += delta;
    timer_accumulator += delta;

    while(timer_accumulator >= CHIP8_SOUND_PERIOD)
    {
      timer_accumulator -= CHIP8_SOUND_PERIOD;
      chip8_update_timers(chip);
      chip8_sound_update(&chip8_snd, chip);
    }

    while(cycle_accumulator >= CHIP8_PERIOD)
    {
      cycle_accumulator -= CHIP8_PERIOD;
      chip8_cycle(chip);
    }

    if(chip->draw)
    {
      chip->draw = 0;
      chip8_window_update(&chip8_win, chip);

    }
  }

  chip8_free(chip);
  chip8_sound_destroy(&chip8_snd);
  chip8_window_destroy(&chip8_win);

  return 0;
}
