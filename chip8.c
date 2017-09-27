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
                    if (0xFF < V[(opcode & 0xF00) >> 8] + V[(opcode & 0xF0) >> 4]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0xF00) >> 8] += V[(opcode & 0xF0) >> 4];

                    pc += 2;
                    break;
                // 8XY5 VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
                case 0x5:
                    if (V[(opcode & 0xF00) >> 8] < V[(opcode & 0xF0) >> 8]) {
                        V[0xF] = 0;
                    } else {
                        V[0xF] = 1;
                    }

                    V[(opcode & 0xF00) >> 8] -= V[(opcode & 0xF0) >> 4];
                    pc += 2;
                    break;
                // 8XY6 Shifts VY right by one and copies the result to VX. VF is set to the value of the least significant bit of VY before the shift.
                case 0x6:
                    V[(0xF)] = V[(opcode & 0xF00) >> 8] & 0x1;
                    V[(opcode & 0xF0) >> 4] >>= 1;
                    V[(opcode & 0xF00) >> 8] = V[(opcode & 0xF0) >> 4];
                    pc += 2;
                    break;
                // 8XY7 Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
                case 0x7:
                    if (V[(opcode & 0xF00) >> 8] > V[(opcode & 0xF0) >> 8]) {
                        V[0xF] = 0;
                    } else {
                        V[0xF] = 1;
                    }
                    V[(opcode & 0xF00) >> 8] = V[(opcode & 0xF0) >> 4] - V[(opcode & 0xF00) >> 8];
                    pc += 2;
                    break;
                // 8XYE Shifts VY left by one and copies the result to VX. VF is set to the value of the most significant bit of VY before the shift
                case 0xE:
                    V[(0xF)] = V[(opcode & 0xF00) >> 8] & 0x1;
                    V[(opcode & 0xF0) >> 4] <<= 1;
                    V[(opcode & 0xF00) >> 8] = V[(opcode & 0xF0) >> 4];
                    pc += 2;
                    break;
                default:
                    printf("Not opcode %X\n", opcode);
            }
            break;
        // 9XY0 Skips the next instruction if VX doesn't equal VY
        case 0x9000:
            if (V[(opcode & 0xF00) >> 8] != V[(opcode & 0xF0) >> 4]) {
                pc += 4;
            }
            else {
                pc += 2;
            }
            break;
        // ANNN Sets I to the address NNN
        case 0xA000:
            I = opcode & 0xFFF;
            pc += 2;
            break;
        // BNNN jumps to the address NNN plus V0
        case 0xB000:
            pc = (opcode & 0xFFF) + V[0];
            break;
        // CXNN Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
        case 0xC000:
            V[(opcode & 0xF00) >> 8] = (opcode & 0xFF) & (rand() % 0xFF);
            pc += 2;
            break;
        // DXYN Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
        // Each row of 8 pixels is read as bit-coded starting from memory location I; I value doesn’t change after the execution of this instruction. 
        // As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen
        case 0xD000:
        {
            unsigned short x = V[(opcode & 0xF00) >> 8];
            unsigned short y = V[(opcode & 0xF0) >> 4];
            unsigned width = 8;
            unsigned height = opcode & 0xF;

            for (int i = 0; i < height; ++i)
            {
                for (int j = 0; j < width; ++j)
                {
                    if (graphic[x + j + ((y + i) * 64)] == 1) {
                        V[0xF] = 1;
                    }
                    graphic[x + j + ((y + i) * 64)] ^= 1;
                }
            }

            pc += 2;
        }
            break;
        case 0xE000:
            switch(opcode & 0xFF) {
                // EX9E Skips the next instruction if the key stored in VX is pressed
                case 0x9E:
                    if (key[(opcode & 0xF00) >> 8] != 0) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                // EXA1 Skips the next instruction if the key stored in VX isn't pressed.
                case 0XA1:
                    if (key[(opcode & 0xF00) >> 8] == 0) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                default:
                    printf("Not opcode %X\n", opcode);
            }
            break;
        case 0xF000:
            switch(opcode & 0xFF) {
                // FX07 Sets VX to the value of the delay timer.
                case 0x07:
                    V[(opcode & 0xF00) >> 8] = delay_timer;
                    pc += 2;
                    break;
                // FX0A A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
                case 0x0A:
                {
                    bool press = false;
                    for (int i = 0; i < 16; ++i)
                    {
                        if(key[i] != 0) {
                            V[(opcode & 0xF00) >> 8] = i;
                            press = true;
                        }
                    }
                    if (!press) {
                        return;
                    }
                    pc += 2;
                }
                break;
                // FX15 Sets the delay timer to VX.
                case 0x15:
                    delay_timer = V[(opcode & 0xF00) >> 8];
                    pc += 2;
                    break;
                // FX18 Sets the sound timer to VX.
                case 0x18:
                    sound_timer = V[(opcode & 0xF00) >> 8];
                    pc += 2;
                    break;
                // FX1E Adds VX to I
                case 0x1E:
                    if ((I + V[(opcode & 0xF00) >> 8]) > 0xFFF) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    I += V[(opcode & 0xF00) >> 8];
                    pc += 2;
                    break;
                // FX29 Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
                case 0x29:
                    I = V[(opcode & 0xF00) >> 8] * 0x5;
                    pc += 2;
                    break;
                // FX33 Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, 
                // the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, 
                // place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
                case 0x33:
                    memory[I] = V[(opcode & 0xF00) >> 8] / 100;
                    memory[I + 1] = (V[(opcode & 0xF00) >> 8] / 10) % 10;
                    memory[I + 2] = (V[(opcode & 0xF00) >> 8] % 100) % 10;
                    pc += 2;
                    break;
                // FX55 Stores V0 to VX (including VX) in memory starting at address I. I is increased by 1 for each value written.
                case 0x55:
                    
            }
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