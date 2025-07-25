//
// Created by Benji on 7/23/2025.
//

#include "../include/chip8_CPU.h"
#include <stdlib.h>
#define START_PROGRAM_ADDRESS 0x200
#define START_PROGRAM_ADDRESS_ETI 0x600
#define FONT_START_ADDRESS 0x050 // convention
#define FONT_SIZE 80 // 5bytes x 16 sprites

struct chip8_cpu
{
    // registers
    uint16_t I; // for memory addresses, 0x000-0xFFF (first 12 bits) will only be used
    uint8_t V[16]; // variable registers 0x0 - 0xF; referred to as Vx; VF should not be used by any program
    // special timer registers; decrements when non-zero
    uint8_t delay_timer;
    uint8_t sound_timer;
    // other
    uint16_t PC; // Program Counter
    uint8_t SP; // Stack Pointer 0x0 - 0xF

    uint8_t memory[4096]; // 0x000 - 0xFFF
    uint16_t stack[16];
};

// 80 = 5 bytes per sprite * 16 sprites
uint8_t fonts[FONT_SIZE] =
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

chip8_cpu_t *p_Init_CHIP8(void)
{
    chip8_cpu_t *toRet_chip8_cpu = NULL;
    toRet_chip8_cpu = malloc(sizeof(chip8_cpu_t));
    if (!toRet_chip8_cpu)
        return NULL;

    // load fonts into memory
    for (int i = 0; i < FONT_SIZE; i++)
    {
        toRet_chip8_cpu->memory[FONT_START_ADDRESS + i] = fonts[i];
    }
    return toRet_chip8_cpu;
}

uint8_t test_get_byte(const chip8_cpu_t *emulator, const int offset)
{
    return emulator->memory[FONT_START_ADDRESS + offset];
}