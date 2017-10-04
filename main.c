#include <stdio.h>
#include <stdlib.h>
#include "chip8.h"
#include <GL/glut.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define MODIFIER 32

struct Chip8 chip8;

// window size
int display_width = SCREEN_WIDTH * MODIFIER;
int display_height = SCREEN_HEIGHT * MODIFIER;

void display();
void reshape_window(GLsizei w, GLsizei h);
void keyboardUp(unsigned char key, int x, int y);
void keyboardDown(unsigned char key, int x, int y);

#define DRAWWITHTEXTURE
typedef uint8_t u8;
u8 screenData[SCREEN_HEIGHT][SCREEN_WIDTH][3];
void setupTexture();

int main(int argc, char const *argv[])
{
	if (argc < 2) {
		printf("need 2 arguments\n");
		return 1;
	}

	if (!chip8.loadApplication(argv[1])) {
		return 1;
	}



	return 0;
}