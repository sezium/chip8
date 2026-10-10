#ifndef CHIP8_H
#define CHIP8_H

typedef unsigned char  chip8_t;
typedef unsigned short chip8_addr_t;
typedef unsigned int   chip8_mem_t;

#define CHIP8_MEM_SIZE       0x1000
#define CHIP8_MEM_ORG        0x200

#define CHIP8_STACK_SIZE     16
#define CHIP8_KEY_SIZE       16

#define CHIP8_DISPLAY_WIDTH  64
#define CHIP8_DISPLAY_HEIGHT 32
#define CHIP8_DISPLAY_SIZE   ((CHIP8_DISPLAY_WIDTH) * (CHIP8_DISPLAY_HEIGHT))
#define CHIP8_DISPLAY_SCALE  (8)
#define CHIP8_CYCLE_DELAY    1

#define CHIP8_FOREGROUND_COLOR 0xFFFFFF
#define CHIP8_BACKGROUND_COLOR 0x000000

#define CHIP8_REG_SIZE       16

enum chip8_register 
{
  V0 = 0x0,
  V1 = 0x1,
  V2 = 0x2,
  V3 = 0x3,
  V4 = 0x4,
  V5 = 0x5,
  V6 = 0x6,
  V7 = 0x7,
  V8 = 0x8,
  V9 = 0x9,
  VA = 0xA,
  VB = 0xB,
  VC = 0xC,
  VD = 0xD,
  VE = 0xE,
  VF = 0xF
};


struct chip8 
{
  chip8_t      mem[CHIP8_MEM_SIZE];
  chip8_addr_t stack[CHIP8_STACK_SIZE];
  chip8_mem_t  gfx[CHIP8_DISPLAY_SIZE];
  
  chip8_t      key[CHIP8_KEY_SIZE];
  chip8_t      regV[CHIP8_REG_SIZE];
  chip8_addr_t I;
  chip8_addr_t pc;
  chip8_t      sp;
  chip8_addr_t opcode;

  chip8_t delay;
  chip8_t sound;

  unsigned char running;

};

#define CHIP8_FONT_SIZE 80
#define CHIP8_FONT_ORG  0x50

static const chip8_t chip8_fontset[CHIP8_FONT_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

struct chip8* chip8_init(void);
void chip8_free(struct chip8 *chip);

void chip8_load_rom(struct chip8 *chip, const char *restrict path);
void chip8_cycle(struct chip8 *chip);
void chip8_update_timers(struct chip8 *chip);

#endif // !CHIP8_H
