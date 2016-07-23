#include <assert.h>

#include "buffer.h"
#include "memory.h"
#include "termio.h"
#include "textbuf.h"

struct Buffer{
	Textbuf *tb;
	int curx,cury;
};

Buffer* b_make(void){
	Buffer *b=malloc(1,Buffer);
	assert(b);
	b->tb=textbuf_make();
	b->curx=b->cury=0;
	return b;
}

void b_destroy(Buffer *b){
	assert(b);
	textbuf_destroy(b->tb);
	free(b);
}

void b_draw(Buffer *b,int x,int y,int w,int h){
	(void)b;
	setbg(1);
	setfg(7);
	fillrect(x,y,w,h,' ');
}


bool b_cursor_move(Buffer *b,Direction dir);

void b_insert(Buffer *b,const String *text);
