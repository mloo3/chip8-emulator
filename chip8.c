#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void init() {
	opcode = 0;
	I = 0;
	// 0x200-0xFFF - Program ROM and work RAM
	pc = 0x200;
	sp = 0;
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
}

void emulateCycle() {
	opcode = (memory[pc] << 8) | memory[pc + 1];
}