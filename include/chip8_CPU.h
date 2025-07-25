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

#include <stdint.h>

typedef struct chip8_cpu chip8_cpu_t;

// constructor and destructor
chip8_cpu_t *p_Init_CHIP8(void);
void vDestroy_CHIP8(chip8_cpu_t *emulator);

uint8_t test_get_byte(const chip8_cpu_t *emulator, int offset);

#endif //CHIP8_CPU_H
