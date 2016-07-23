#include <stdio.h>
#include <stdlib.h>
#include <termio.h>

#include "buffer.h"

int main(void){
	initscreen();
	//atexit(endscreen);
	initkeyboard();
	//atexit(endkeyboard);
	installCLhandler(true);

	Buffer *b=b_make();
	b_draw(b,2,2,5,5);
	redraw();
	tgetkey();
	b_destroy(b);

	endkeyboard();
	endscreen();
}
