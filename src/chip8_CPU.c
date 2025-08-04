//
// Created by Benji on 7/23/2025.
//

#include "../include/chip8_CPU.h"
#include "../include/logging.h"
#include <time.h>
#include <stdio.h>

#define START_PROGRAM_ADDRESS 0x200
#define START_PROGRAM_ADDRESS_ETI 0x600
#define FONT_START_ADDRESS 0x050 // convention
#define FONT_SIZE 80 // 5bytes x 16 sprites
#define MAX_PROGRAM_SIZE 0xE00

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

    uint8_t keyboard_interrupt; // stores value of key in interrupt
    // flags
    uint8_t f_waiting_for_input;
    uint8_t f_keyboard_interrupt;

    // display reference to send things like clear signal
    display_t *display;

    uint8_t keyboard[16]; // 0-> not pressed !0-> pressed
    uint8_t memory[4096]; // 0x000 - 0xFFF
    uint16_t stack[16];
};

/*  keymap
    30, 31, 32, 33, // 1, 2, 3 ,4
    20, 26, 8,  21, // q, w, e, r
    4,  22, 7,  9,  // a, s, d, f
    29, 27, 6,  25  // z, x, c, v
*/

uint8_t map_scancode_to_key(uint8_t scancode)
{
    switch (scancode)
    {
        case 30:
            return 1;
        case 31:
            return 2;
        case 32:
            return 3;
        case 33:
            return 0xC;
        case 20:
            return 4;
        case 26:
            return 5;
        case 8:
            return 6;
        case 21:
            return 0xD;
        case 4:
            return 7;
        case 22:
            return 8;
        case 7:
            return 9;
        case 9:
            return 0xE;
        case 29:
            return 0xA;
        case 27:
            return 0;
        case 6:
            return 0xB;
        case 25:
            return 0xF;
        default:
            return 0;
    }
}

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

/*
 *  initialization
 *      init chip8
 *      set display: this is separate so that the emulator can run headless
 */

chip8_cpu_t *p_Init_CHIP8(void)
{
    chip8_cpu_t *toRet_chip8_cpu = NULL;
    toRet_chip8_cpu = calloc(sizeof(chip8_cpu_t), 1);
    if (!toRet_chip8_cpu)
        return NULL;

    // load fonts into memory
    for (int i = 0; i < FONT_SIZE; i++)
    {
        toRet_chip8_cpu->memory[FONT_START_ADDRESS + i] = fonts[i];
    }

    // set Program Counter to where program will be loaded
    toRet_chip8_cpu->PC = START_PROGRAM_ADDRESS;

    // setting a random seed for 0xC--- instruction (suspect)
    srand(time(NULL));
    return toRet_chip8_cpu;
}

void vDestroy_CHIP8(chip8_cpu_t *emulator)
{
    // MIGHT NEED TO FREE DISPLAY
    free(emulator);
}

int set_display(chip8_cpu_t *emulator, display_t *display)
{
    if (emulator == NULL)
        return -1;
    emulator->display = display;
    return 0;
}

/*
 *   Keyboard stuff!
 *       handle interrupts (used by SDL event loop in main)
 *       emulator waiting (used in instruction 0xFx0A, halting execution
 *
 */

void v_handle_keyboard_interrupt(chip8_cpu_t *emulator, SDL_KeyboardEvent *event)
{
    const uint8_t key = map_scancode_to_key(event->keysym.scancode);
    emulator->keyboard[key] = event->state == SDL_KEYDOWN ? 1 : 0;
    emulator-> keyboard_interrupt = key;
    if (emulator->f_waiting_for_input)
    {
        emulator->f_keyboard_interrupt = 1; // we have successfully been interrupted
        emulator->f_waiting_for_input = 0;
    }
}

uint8_t i_emulator_is_waiting(const chip8_cpu_t *emulator)
{
    return emulator->f_waiting_for_input;
}

// loading data

struct gamefile *p_load_game_from_file(FILE *infile)
{
    if (infile == NULL)
        return NULL;
    struct gamefile *p_game = malloc (sizeof(struct gamefile));
    if (!p_game)
        return NULL;
    p_game->size = 0;
    p_game->data = NULL;

    // get file length, reset file pointer to beginning
    fseek(infile, 0, SEEK_END);
    const long length = ftell(infile);
    if (length > MAX_PROGRAM_SIZE)
        return NULL;
    rewind(infile);

    p_game->data = malloc(sizeof(uint8_t) * length);
    if (!p_game->data)
        return NULL;

    const size_t read_code = fread(p_game->data, sizeof(uint8_t), length, infile);
    if (read_code == length)
        printf("successfully read game data");
    p_game->size = length;

    return p_game;
}

void v_load_rom(chip8_cpu_t *emulator, const struct gamefile *game)
{
    if (emulator == NULL || game == NULL)
        return;
    emulator->PC = START_PROGRAM_ADDRESS;
    for (int o = 0; o < game->size; o++)
    {
        emulator->memory[START_PROGRAM_ADDRESS + o] = game->data[o];
    }
}

/*
 *  functional section
 *      fetch & decode,
 *      decode basically all the work of the system!
 */
uint16_t i_fetch_instruction(chip8_cpu_t *emulator)
{
    uint16_t instruction = 0;
    instruction |= emulator->memory[emulator->PC] << 8;
    instruction |= emulator->memory[emulator->PC + 1];
    emulator->PC += 2;
    return instruction;
}

void draw_thru_emulator(const chip8_cpu_t *emulator)
{
    draw(emulator->display);
}

void decode(chip8_cpu_t *emulator, const uint16_t instruction)
{
    if (get_log_level(INFO))
    {
        printf("\ninstruction: %.4x", instruction);
        printf("\n  instru first 4 bits: %x", (instruction & 0xF000) >> 12);
        fflush(stdout);
    }

    const uint16_t nnn = (instruction & 0x0FFF); // addr; lowest 12 bits.
    const uint8_t n = (instruction & 0x000F); // nibble; lowest 4 bits
    const uint8_t x = (instruction & 0x0F00) >> 8; // 4 lower bits of high byte
    const uint8_t y = (instruction & 0x00F0) >> 4; // 4 upper bits of low byte
    const uint8_t kk = (instruction & 0x00FF); // byte; lowest 8 bits of instr
    uint16_t total = 0;

    switch ((instruction & 0xF000) >> 12) // switch off of first 4 bits
    {
        case 0:
            switch (nnn)
            {
                case 0x0E0:
                    if (get_log_level(DEBUG))
                    {
                        printf("\nclearing display");
                    }
                    if (emulator->display != NULL)
                        clear_screen(emulator->display);
                    break;
                case 0x00EE:
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     fetching program counter from stack %.4x", emulator->stack[emulator->SP]);
                        printf("\n     decrementing stack counter: %d -> %d", emulator->SP, emulator->SP -1);
                    }
                    emulator->PC = emulator->stack[emulator->SP];
                    emulator->SP -= 1;
                    break;
                default:
                    break;
            }
            break;
        case 1:
            if (get_log_level(DEBUG))
            {
                printf("\n     setting PC to %.4x", nnn);
            }
            emulator->PC = nnn;
            break;
        case 2:
            if (get_log_level(DEBUG))
            {
                printf("\n     storing program counter on stack");
            }
            emulator->SP += 1;
            emulator->stack[emulator->SP] = emulator->PC;
            emulator->PC = nnn;
            break;
        case 3:
            if (get_log_level(DEBUG))
            {
                printf("\n     skipping instruction if V[%d] (%.4x) = %.4x", x, emulator->V[x], kk);
            }
            if (emulator->V[x] == kk)
                emulator->PC += 2;
            break;
        case 4:
            if (get_log_level(DEBUG))
            {
                printf("\n     skipping instruction if V[%d] (%.4x) != %.4x", x, emulator->V[x], kk);
            }
            if (emulator->V[x] != kk)
                emulator->PC += 2;
            break;
        case 5:
            if (get_log_level(DEBUG))
            {
                printf("\n     skipping instruction if V[%d] (%.4x) = V[%d] (%.4x)", x, emulator->V[x], y, emulator->V[y]);
            }
            if (emulator->V[x] == emulator->V[y])
                emulator->PC += 2;
            break;
        case 6:
            if (get_log_level(DEBUG))
            {
                printf("\n     Setting V[%d] to %.4x", x, kk);
            }
            emulator->V[x] = kk;
            break;
        case 7:
            if (get_log_level(DEBUG))
            {
                printf("\n     adding %.4x to V[%d] (%.4x)", kk, x, emulator->V[x]);
            }
            emulator->V[x] += kk;
            break;
        case 8:
            switch (n)
            {
                case 0:
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     Setting V[%d] equal to value in V[%d] (%.4x)", x, y, emulator->V[y]);
                    }
                    emulator->V[x] = emulator->V[y];
                    break;
                case 1:
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     boolean OR'ing V[%d] (%.4x) and V[%d] (%.4x)", x, emulator->V[x], y, emulator->V[y]);
                    }
                    emulator->V[x] |= emulator->V[y];
                    break;
                case 2:
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     boolean AND'ing V[%d] (%.4x) and V[%d] (%.4x)", x, emulator->V[x], y, emulator->V[y]);
                    }
                    emulator->V[x] &= emulator->V[y];
                    break;
                case 3:
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     boolean XOR'ing V[%d] (%.4x) and V[%d] (%.4x)", x, emulator->V[x], y, emulator->V[y]);
                    }
                    emulator->V[x] ^= emulator->V[y];
                    break;
                case 4: // ADD
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     Adding V[%d] (%.4x) and V[%d] (%.4x)", x, emulator->V[x], y, emulator->V[y]);
                    }
                    total = emulator->V[x] + emulator->V[y];
                    emulator->V[x] += emulator->V[y];
                    emulator->V[0xF] = total > 255 ? 1 : 0;
                    break;
                case 5: // SUB Vx, Vy
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     Subtracting V[%d] (%.4x) and V[%d] (%.4x)", x, emulator->V[x], y, emulator->V[y]);
                    }
                    emulator->V[0xF] = emulator->V[x] > emulator->V[y] ? 1 : 0;
                    emulator->V[x] -= emulator->V[y];
                    break;
                case 6: // shift right
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     Shifting V[%d] (%.4x) 1 bit to the right", x, emulator->V[x]);
                    }
                    emulator->V[0xF] = emulator->V[x] & 0x1 ? 1 : 0;
                    emulator->V[x] >>= 1;
                    break;
                case 7: // SUBN Vx, Vy
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     Subtracting V[%d] (%.4x) and V[%d] (%.4x)", y, emulator->V[y], x, emulator->V[x]);
                    }
                    emulator->V[0xF] = emulator->V[y] > emulator->V[x] ? 1 : 0;
                    emulator->V[x] = emulator->V[y] - emulator->V[x];
                    break;
                case 0xE:
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     Shifting V[%d] (%.4x) 1 bit to the left", x, emulator->V[x]);
                    }
                    emulator->V[0xF] = emulator->V[x] & 0x80 ? 1 : 0;
                    emulator->V[x] <<= 1;
                    break;
                default:
                    break;
            }
            break;
        case 9:
            if (get_log_level(DEBUG))
            {
                printf("\n     skipping instruction if V[%d] (%.4x) != V[%d] (%.4x)", x, emulator->V[x], y, emulator->V[y]);
            }
            if (emulator->V[x] != emulator->V[y])
                emulator->PC += 2;
            break;
        case 0xA:
            if (get_log_level(DEBUG))
            {
                printf("\n     Setting I register to %.4x", nnn);
            }
            emulator->I = nnn;
            break;
        case 0xB:
            if (get_log_level(DEBUG))
            {
                printf("\n     Setting PC to nnn (%.4d) + V[0] (%.4x)", nnn, emulator->V[0]);
            }
            emulator->PC = nnn + emulator->V[0];
            break;
        case 0xC:
            if (get_log_level(DEBUG))
            {
                printf("\n     setting V[%d] to random number", x);
            }
            emulator->V[x] = rand() & kk;
            break;
        case 0xD: // Dxyn
            if (get_log_level(DEBUG))
            {
                printf("\n     Draw!");
            }
            if (emulator->display != NULL)
            {
                emulator->V[0xF] = 0;
                const uint8_t sprite_x = emulator->V[x] & 63;
                const uint8_t sprite_y = emulator->V[y] & 31;
                for (int j = 0; j < n; j++)
                {
                    if (sprite_y + j >= 32)
                        break;
                    const uint8_t byte = emulator->memory[emulator->I + j];
                    for (int i = 0; i < 8; i++)
                    {
                        if (sprite_x + i >= 64)
                            break;
                        const uint8_t pixel = byte & 0x80 >> i;
                        if (pixel)
                        {
                            uint8_t flipped = draw_internal(emulator->display, sprite_x + i, sprite_y + j);
                            if (!emulator->V[0xF] && flipped)
                                emulator->V[0xF] = 1;
                        }
                    }
                }
            }
            break;
        case 0xE:
            switch (kk)
            {
                case 0x9E: // SKP Vx
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     check if %d is pressed", emulator->V[x]);
                    }
                    if (emulator->keyboard[emulator->V[x]] != 0)
                        emulator->PC += 2;
                    break;
                case 0xA1: // SKNP Vx
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     check if %d is NOT pressed", emulator->V[x]);
                    }
                    if (emulator->keyboard[emulator->V[x]] == 0)
                        emulator->PC += 2;
                    break;
                default:
                    break;
            }
            break;
        case 0xF:
            switch (kk)
            {
                case 0x07:
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     store delay timer (%d) in V[%d]", emulator->delay_timer, x);
                    }
                    emulator->V[x] = emulator->delay_timer;
                    break;
                case 0x0A:
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     keyboard interrupt request");
                    }
                    if (emulator->f_keyboard_interrupt)
                    {
                        emulator->V[x] = emulator->keyboard_interrupt;
                        emulator->f_keyboard_interrupt = 0;
                    }
                    else
                    {
                        emulator->f_waiting_for_input = 1;
                    }
                    break;
                case 0x15:
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     set delay timer to V[%d] (%.4x)", x, emulator->V[x]);
                    }
                    emulator->delay_timer = emulator->V[x];
                    break;
                case 0x18:
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     set sound timer to V[%d] (%.4x)", x, emulator->V[x]);
                    }
                    emulator->sound_timer = emulator->V[x];
                    break;
                case 0x1E:
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     add V[%d] (%.4x) to I register (%.4x)", x, emulator->V[x], emulator->I);
                    }
                    emulator->I += emulator->V[x];
                    break;
                case 0x29:
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     | I reg to mem addr (%.4x)", emulator->memory[FONT_START_ADDRESS + emulator->V[x]]);
                    }
                    emulator->I |= emulator->memory[FONT_START_ADDRESS + emulator->V[x]];
                // emulator->I |= emulator->memory[FONT_START_ADDRESS + emulator->V[x] + 1];
                    break;
                case 0x33: // LD B, Vx
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     store BCD rep of value in V[%d]", x);
                    }
                    int val = emulator->V[x];
                    emulator->memory[emulator->I] = val % 1000 / 100;
                    emulator->memory[emulator->I + 1] = val % 100 / 10;
                    emulator->memory[emulator->I + 2] = val % 10;
                    break;
                case 0x55:
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     Store memory!");
                    }
                    for (int i = 0; i <= 0xF; i++)
                    {
                        emulator->memory[emulator->I + i] = emulator->V[i];
                    }
                    break;
                case 0x65:
                    if (get_log_level(DEBUG))
                    {
                        printf("\n     load memory!");
                    }
                    for (int i = 0; i <= 0xF; i++)
                    {
                        emulator->V[i] = emulator->memory[emulator->I + i];
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    if (get_log_level(DEBUG))
    {
        fflush(stdout);
    }
}

