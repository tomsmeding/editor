#include <stdio.h>
#include <stdlib.h>
#include <termio.h>

#include "util.h"

__attribute__((noreturn)) void outofmem(void){
	endkeyboard(); //clean that up while we're busy
	endscreen();
	fprintf(stderr,"OUT OF MEMORY!\n");
	exit(1);
}
