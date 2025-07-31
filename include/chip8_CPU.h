//
// Created by Benji on 7/23/2025.
//

/* stolen from: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
+---------------+= 0xFFF (4095) End of Chip-8 RAM
|               |
|               |
|               |
|               |
|               |
| 0x200 to 0xFFF|
|     Chip-8    |
| Program / Data|
|     Space     |
|               |
|               |
|               |
+- - - - - - - -+= 0x600 (1536) Start of ETI 660 Chip-8 programs
|               |
|               |
|               |
+---------------+= 0x200 (512) Start of most Chip-8 programs
| 0x000 to 0x1FF|
| Reserved for  |
|  interpreter  |
+---------------+= 0x000 (0) Start of Chip-8 RAM
*/

#ifndef CHIP8_CPU_H
#define CHIP8_CPU_H

#include <SDL_events.h>
#include <stdint.h>
#include "display.h"

typedef struct chip8_cpu chip8_cpu_t;

struct gamefile
{
    uint8_t size;
    uint8_t *data;
};

// constructor and destructor init stuff
chip8_cpu_t *p_Init_CHIP8(void);
void vDestroy_CHIP8(chip8_cpu_t *emulator);
int set_display(chip8_cpu_t *emulator, display_t *display);

// keyboard stuff
void v_handle_keyboard_interrupt(chip8_cpu_t *emulator, SDL_KeyboardEvent *event);
uint8_t i_emulator_is_waiting(const chip8_cpu_t *emulator);

// loading data
struct gamefile *p_load_game_from_file(FILE *infile);
void v_load_rom(chip8_cpu_t *emulator, const struct gamefile *game);

// functional stuff
uint16_t i_fetch_instruction(chip8_cpu_t *emulator);
void decode(chip8_cpu_t *emulator, uint16_t instruction);
void draw_thru_emulator(const chip8_cpu_t *emulator);

#endif //CHIP8_CPU_H
