#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

unsigned char fontset[80] =
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

void init() {
	opcode = 0;
	I = 0;
	// 0x200-0xFFF - Program ROM and work RAM
	pc = 0x200;
	sp = 0;
    //clear mem, registers, display, stack
	for (int i = 0; i < 4096; ++i)
	{
		memory[i] = 0;
	}
    for (int i = 0; i < 16; ++i)
    {
        V[i] = 0;
    }
    for (int i = 0; i < 2048; ++i)
    {
        graphic[i] = 0;
    }
    for (int i = 0; i < 16; ++i)
    {
        stack[i] = 0;
    }
    // reset timers
    delay_timer = 0;
    sound_timer = 0;
    // fontset loaded to 0x50
    for (int i = 0; i < 80; ++i)
    {
        memory[i] = fontset[i];
    }
}

void iCycle() {
    // fetch
	opcode = (memory[pc] << 8) | memory[pc + 1];
    // decode
    switch(opcode & 0xF000) {
        case 0x0000:
            switch(opcode & 0xF) {
                // 00E0 clears the screen
                case 0x0:
                    for (int i = 0; i < 2048; ++i)
                    {
                        graphic[i] = 0;
                    }
                    pc += 2;
                    break;
                // 00EE returns from a subroutine
                case 0xE:
                    pc = stack[--sp];
                    pc += 2;
                    break;
                default:
                    printf("Not opcode %X\n", opcode);
            }
            break;
        // 1NNN jumps to address NNN
        case 0x1000:
            pc = opcode * 0xFFF;
            break;
        // 2NNN calls subroutine at NNN
        case 0x2000:
            stack[sp++] = pc;
            pc = opcode & 0xFFF;
            break;
        // 3XNN skips the next instructino if VX equals NN
        case 0x3000:
            if (V[(opcode & 0xF00) >> 8] == (opcode & 0xFF)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        // 4XNN Skips the next instruction if VX doesn't equal NN
        case 0x4000:
            if (V[(opcode & 0xF00) >> 8] != (opcode & 0xFF)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        // 5XY0 Skips the next instruction if VX equals VY. 
        case 0x5000:
            if (V[(opcode & 0xF00) >> 8] == V[(opcode & 0xF0) >> 4]) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        // 6XNN sets VX to NN
        case 0x6000:
            V[(opcode & 0xF00) >> 8] = opcode & 0xFF;
            pc += 2;
            break;
        // 7XNN adds NN to VX
        case 0x7000:
            V[(opcode & 0xF00) >> 8] += opcode & 0xFF;
            pc += 2;
            break;
        case 0x8000:
            switch(opcode & 0xF) {
                // 8XY0 sets VX to the value of VY
                case 0x0:
                    V[(opcode & 0xF00) >> 8] = V[(opcode & 0xF0) >> 4];
                    pc += 2;
                    break;
                // 8XY1 sets VX to VX or VY
                case 0x1:
                    V[(opcode & 0xF00) >> 8] |= V[(opcode & 0xF0) >> 4];
                    pc += 2;
                    break;
                // 8XY2 sets VX to VX and VY
                case 0x2:
                    V[(opcode & 0xF00) >> 8] &= V[(opcode & 0xF0) >> 4];
                    pc += 2;
                    break;
                // 8XY3 sets VX to VX xor VY
                case 0x3:
                    V[(opcode & 0xF00) >> 8] ^= V[(opcode & 0xF0) >> 4];
                    pc += 2;
                    break;
                // 8XY4 Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
                case 0x4:
                    V[(opcode & 0xF00) >> 8] += V[(opcode & 0xF0) >> 4];
                    if (0xFF < V[(opcode & 0xF00) >> 8] + V[(opcode & 0xF0) >> 4]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    pc += 2;
                    break;
            }
        case 0x9000:
        case 0xA000:
        case 0xB000:
        case 0xC000:
        case 0xD000:
        case 0xE000:
        case 0xF000:
    }
    // execute

}

bool load(const char* filename) {



    FILE* file = fopen(filename, "rb");

    for (int i = 0; i < bufferSize; ++i)
    {
        memory[i+512] = buffer[i]
    }

    return true;
}