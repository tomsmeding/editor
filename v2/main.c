#include <stdio.h>
#include <stdlib.h>
#include <termio.h>

int main(void){
	initscreen();
	atexit(endscreen);
	initkeyboard();
	atexit(endkeyboard);
	setfg(3);
	tprintf("jeej!");
	redraw();
	tgetkey();
}
