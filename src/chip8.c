#include <assert.h>
#include <chip8.h>
#include <log.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct chip8* chip8_init(void)
{
  struct chip8 *chip = malloc(sizeof(struct chip8));

  if(NULL == chip) 
  {
    LOG_ERROR("chip8 malloc error");
    exit(1);
  }

  memset(chip->mem,   0, sizeof(chip->mem));
  memset(chip->stack, 0, sizeof(chip->stack));
  memset(chip->gfx,   0, sizeof(chip->gfx));
  memset(chip->regV,  0, sizeof(chip->regV));
  memset(chip->key,   0, sizeof(chip->key));

  chip->pc = CHIP8_MEM_ORG;
  chip->I = 0;
  chip->sp = 0;
  chip->opcode = 0;
  chip->delay = 0;
  chip->sound = 0;
  chip->running = 0;
  chip->draw = 0;

  srand(time(0));

  return chip;
}


void chip8_load_rom(struct chip8 *chip, const char *restrict path)
{
  if(NULL == chip) 
  {
    LOG_ERROR("chip8 not initalized to load ROM");
    exit(1);
  }

  if(NULL == path)
  {
    LOG_ERROR_ARGS("ROM path not found at %s", path);
    exit(1);
  }

  /* from bottom to top: we will start with fontmap init in memory */

  if(CHIP8_FONT_ORG + CHIP8_FONT_SIZE > CHIP8_MEM_ORG)
  {
    LOG_ERROR_ARGS("chip8 fontmap is too big: %06x > %06x", 
        CHIP8_FONT_ORG + CHIP8_FONT_SIZE, CHIP8_MEM_ORG);
    exit(1);

  }
  memcpy((chip->mem + CHIP8_FONT_ORG), chip8_fontset, CHIP8_FONT_SIZE);


  /* now we init chip8 rom in memory */

  FILE *fd = fopen(path, "rb");
  if(NULL == fd) 
  {
    LOG_ERROR_ARGS("fopen failed: %s", path);
    exit(1);

  }

  fseek(fd, 0, SEEK_END);
  size_t rom_size = ftell(fd);

  if(rom_size > CHIP8_MEM_SIZE - CHIP8_MEM_ORG)
  {
    LOG_ERROR_ARGS("rom size is too big: %ld", rom_size);
    exit(1);
  }

  fseek(fd, 0, SEEK_SET);

  if (fread(chip->mem + CHIP8_MEM_ORG, 1, rom_size, fd) != rom_size)
  {
    LOG_ERROR("fread: Failed to read ROM");
    exit(1);
  }

  fclose(fd);


}

void chip8_free(struct chip8 *chip) 
{
  if(NULL == chip) 
  {
    LOG_ERROR("chip8 free on null");
    exit(1);
  }

  free(chip);
}


/* HANDLER FUNCTIONS */

#define NNN(chip)  ((chip->opcode) & 0x0FFFu)
#define  NN(chip)  ((chip->opcode) & 0x00FFu)
#define   N(chip)  ((chip->opcode) & 0x000Fu)
#define   X(chip) (((chip)->opcode & 0x0F00u) >> 8u)
#define   Y(chip) (((chip)->opcode & 0x00F0u) >> 4u)


static inline void chip8_skip_instr(struct chip8 *chip)
{
  chip->pc += 2;
}


/* STARTING OPCODE */

static inline void chip8_op_00E0(struct chip8 *chip)
{
  memset(chip->gfx, 0, sizeof(chip->gfx));
  chip->draw = 1;
}

static inline void chip8_op_00EE(struct chip8 *chip)
{
  chip->pc = chip->stack[--chip->sp];
}

static inline void chip8_op_1NNN(struct chip8 *chip)
{
  chip->pc = NNN(chip);
}

static inline void chip8_op_2NNN(struct chip8 *chip)
{
  chip->stack[chip->sp++] = chip->pc;
  chip->pc = NNN(chip);
}

static inline void chip8_op_3XNN(struct chip8 *chip)
{
  if(chip->regV[X(chip)] == NN(chip)) chip8_skip_instr(chip);
}

static inline void chip8_op_4XNN(struct chip8 *chip)
{
  if(chip->regV[X(chip)] != NN(chip)) chip8_skip_instr(chip);
}

static inline void chip8_op_5XY0(struct chip8 *chip)
{
  if(chip->regV[X(chip)] == chip->regV[Y(chip)]) chip8_skip_instr(chip);
}

static inline void chip8_op_6XNN(struct chip8 *chip)
{
  chip->regV[X(chip)] = NN(chip);
}

static inline void chip8_op_7XNN(struct chip8 *chip)
{
  chip->regV[X(chip)] += NN(chip);
}

static inline void chip8_op_8XY0(struct chip8 *chip)
{
  chip->regV[X(chip)] = chip->regV[Y(chip)];
}

static inline void chip8_op_8XY1(struct chip8 *chip)
{
  chip->regV[X(chip)] = chip->regV[X(chip)] | chip->regV[Y(chip)];
}

static inline void chip8_op_8XY2(struct chip8 *chip)
{
  chip->regV[X(chip)] = chip->regV[X(chip)] & chip->regV[Y(chip)];
}

static inline void chip8_op_8XY3(struct chip8 *chip)
{
  chip->regV[X(chip)] = chip->regV[X(chip)] ^ chip->regV[Y(chip)];
}

static inline void chip8_op_8XY4(struct chip8 *chip)
{
  // carry flag 
  chip->regV[VF] = (chip->regV[X(chip)] + chip->regV[Y(chip)]) < chip->regV[X(chip)];
  chip->regV[X(chip)] += chip->regV[Y(chip)];
}

static inline void chip8_op_8XY5(struct chip8 *chip)
{
  // carry flag 
  chip->regV[VF] = chip->regV[X(chip)] > chip->regV[Y(chip)];
  chip->regV[X(chip)] -= chip->regV[Y(chip)];
}

static inline void chip8_op_8XY6(struct chip8 *chip)
{
  // carry flag 
  chip->regV[VF] = chip->regV[X(chip)] & 1;
  chip->regV[X(chip)] >>= 1;
}

static inline void chip8_op_8XY7(struct chip8 *chip)
{
  // carry flag 
  chip->regV[VF] = chip->regV[Y(chip)] > chip->regV[X(chip)];
  chip->regV[X(chip)] = chip->regV[Y(chip)] - chip->regV[X(chip)];
}

static inline void chip8_op_8XYE(struct chip8 *chip)
{
  // carry flag 
  // if MSB is 1 then
  chip->regV[VF] = (chip->regV[X(chip)] & 0x80) >> 7u;
  chip->regV[X(chip)] <<= 1;

}

static inline void chip8_op_9XY0(struct chip8 *chip)
{
  if(chip->regV[X(chip)] != chip->regV[Y(chip)]) chip8_skip_instr(chip);
}

static inline void chip8_op_ANNN(struct chip8 *chip)
{
  chip->I = NNN(chip);
}

static inline void chip8_op_BNNN(struct chip8 *chip)
{
  chip->pc = NNN(chip) + chip->regV[V0];
}

static inline void chip8_op_CXNN(struct chip8 *chip)
{
  chip->regV[X(chip)] = (rand() & 255) & NN(chip);
}

static inline void chip8_op_DXYN(struct chip8 *chip)
{
  chip8_t height = N(chip);

  chip8_t xPos = chip->regV[X(chip)] % CHIP8_DISPLAY_WIDTH;
  chip8_t yPos = chip->regV[Y(chip)] % CHIP8_DISPLAY_HEIGHT;

  chip->regV[VF] = 0;

  for (size_t row = 0; row < height; ++row)
  {
    chip8_t spriteByte = chip->mem[chip->I + row];

    for (size_t col = 0; col < 8; ++col)
    {
      chip8_t spritePixel = spriteByte & (0x80u >> col);
      chip8_mem_t *screenPixel = &chip->gfx[
        ((yPos + row) % CHIP8_DISPLAY_HEIGHT) * CHIP8_DISPLAY_WIDTH
          + ((xPos + col) % CHIP8_DISPLAY_WIDTH)
      ];

      if(!spritePixel) continue;

      if (*screenPixel == 0xFFFFFFFF)
      {
        chip->regV[VF] = 1;
      }

      *screenPixel ^= 0xFFFFFFFF;

      
    }
  }

  chip->draw = 1;
}


static inline void chip8_op_EX9E(struct chip8 *chip)
{
  if(chip->key[X(chip)]) chip8_skip_instr(chip);
}

static inline void chip8_op_EXA1(struct chip8 *chip)
{
  if(!chip->key[X(chip)]) chip8_skip_instr(chip);
}

static inline void chip8_op_FX07(struct chip8 *chip)
{
  chip->regV[X(chip)] = chip->delay;
}

static inline void chip8_op_FX0A(struct chip8 *chip)
{
  for(size_t i = 0; i < CHIP8_KEY_SIZE; i++)
  {
    if (chip->key[i])
    {
      chip->regV[X(chip)] = i;
      return;
    }

  }

  chip->pc -= 2;
}

static inline void chip8_op_FX15(struct chip8 *chip)
{
  chip->delay = chip->regV[X(chip)];
}

static inline void chip8_op_FX18(struct chip8 *chip)
{
  chip->sound = chip->regV[X(chip)];
}

static inline void chip8_op_FX1E(struct chip8 *chip)
{
  chip->I += chip->regV[X(chip)];
}

static inline void chip8_op_FX29(struct chip8 *chip)
{
  chip->I = CHIP8_FONT_ORG + chip->regV[X(chip)] * 5;
}

static inline void chip8_op_FX33(struct chip8 *chip)
{
  chip8_t vX = chip->regV[X(chip)];
  chip->mem[chip->I + 2] = vX % 10;
  vX /= 10;
  chip->mem[chip->I + 1] = vX % 10;
  vX /= 10;
  chip->mem[chip->I + 0] = vX % 10;
}

static inline void chip8_op_FX55(struct chip8 *chip)
{
  for(size_t i = 0; i <= X(chip); i++)
  {
    chip->mem[chip->I + i] = chip->regV[i];
  }

}

static inline void chip8_op_FX65(struct chip8 *chip)
{
  for(size_t i = 0; i <= X(chip); i++)
  {
    chip->regV[i] = chip->mem[chip->I + i];
  }

}

static void (*chip8_op_0_table[256])(struct chip8 *chip) = {
  [0xE0] = chip8_op_00E0,
  [0xEE] = chip8_op_00EE
};

static void (*chip8_op_8_table[16])(struct chip8 *chip) = {
  [0x0] = chip8_op_8XY0,
  [0x1] = chip8_op_8XY1,
  [0x2] = chip8_op_8XY2,
  [0x3] = chip8_op_8XY3,
  [0x4] = chip8_op_8XY4,
  [0x5] = chip8_op_8XY5,
  [0x6] = chip8_op_8XY6,
  [0x7] = chip8_op_8XY7,
  [0xE] = chip8_op_8XYE
};

static void (*chip8_op_E_table[256])(struct chip8 *chip) = {
  [0x9E] = chip8_op_EX9E,
  [0xA1] = chip8_op_EXA1
};

static void (*chip8_op_F_table[256])(struct chip8 *chip) = {
  [0x07] = chip8_op_FX07,
  [0x0A] = chip8_op_FX0A,
  [0x15] = chip8_op_FX15,
  [0x18] = chip8_op_FX18,
  [0x1E] = chip8_op_FX1E,
  [0x29] = chip8_op_FX29,
  [0x33] = chip8_op_FX33,
  [0x55] = chip8_op_FX55,
  [0x65] = chip8_op_FX65
};


static inline void chip8_op_E(struct chip8 *chip)
{
  void (*op)(struct chip8 *) = chip8_op_E_table[NN(chip)];

  if (op == NULL) {
    LOG_ERROR_ARGS("Unknown opcode: 0x%04X", chip->opcode);
    chip->running = 0;
    return;
  }

  op(chip);
}

static inline void chip8_op_F(struct chip8 *chip)
{
  void (*op)(struct chip8 *) = chip8_op_F_table[NN(chip)];

  if (op == NULL) {
    LOG_ERROR_ARGS("Unknown opcode: 0x%04X", chip->opcode);
    chip->running = 0;
    return;
  }

  op(chip);
}

static inline void chip8_op_8(struct chip8 *chip)
{
  void (*op)(struct chip8 *) = chip8_op_8_table[N(chip)];

  if (op == NULL) {
    LOG_ERROR_ARGS("Unknown opcode: 0x%04X", chip->opcode);
    chip->running = 0;
    return;
  }

  op(chip);
}

static inline void chip8_op_0(struct chip8 *chip) 
{
  void (*op)(struct chip8 *) = chip8_op_0_table[NN(chip)];

  if (op == NULL) 
  {
    LOG_ERROR_ARGS("Unknown opcode: 0x%04X", chip->opcode);
    chip->running = 0;
    return;
  }

  op(chip);

}

static void (*chip8_op_table[16])(struct chip8 *chip) = 
{
  [0x0] = chip8_op_0,
  [0x1] = chip8_op_1NNN,
  [0x2] = chip8_op_2NNN,
  [0x3] = chip8_op_3XNN,
  [0x4] = chip8_op_4XNN,
  [0x5] = chip8_op_5XY0,
  [0x6] = chip8_op_6XNN,
  [0x7] = chip8_op_7XNN,
  [0x8] = chip8_op_8,
  [0x9] = chip8_op_9XY0,
  [0xA] = chip8_op_ANNN,
  [0xB] = chip8_op_BNNN,
  [0xC] = chip8_op_CXNN,
  [0xD] = chip8_op_DXYN,
  [0xE] = chip8_op_E,
  [0xF] = chip8_op_F
};


void chip8_update_timers(struct chip8 *chip) 
{
  if (chip->delay > 0)
    chip->delay--;

  if (chip->sound > 0)
    chip->sound--;

}


void chip8_cycle(struct chip8 *chip)
{
  if (chip->pc >= CHIP8_MEM_SIZE - 1) 
  {
    LOG_ERROR_ARGS("PC out of bounds: 0x%04X", chip->pc);
    chip->running = 0;
    return;
  }

  chip->opcode = ((uint16_t)chip->mem[chip->pc] << 8u) | (uint16_t)chip->mem[chip->pc + 1];

  chip8_skip_instr(chip);

  uint8_t instruction = (chip->opcode >> 12u) & 0x0Fu;
  void (*op)(struct chip8 *) = chip8_op_table[instruction];

  if (op == NULL) 
  {
    LOG_ERROR_ARGS("Unknown opcode: 0x%04X", chip->opcode);
    chip->running = 0;
    return;
  }

  op(chip);


}
