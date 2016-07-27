#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <termio.h>

#include "buffer.h"
#include "memory.h"
#include "textbuf.h"

struct Buffer{
	Textbuf *tb;
	int curx,cury,topy;
};

Buffer* b_make(void){
	Buffer *b=malloc(1,Buffer);
	assert(b);
	b->tb=textbuf_make();
	b->curx=b->cury=0;
	b->topy=0;
	return b;
}

void b_destroy(Buffer *b){
	assert(b);
	textbuf_destroy(b->tb);
	free(b);
}

int b_nlines(const Buffer *b){
	return textbuf_nlines(b->tb);
}

//buf should have at least space for 5 characters
//returns string length
static int charrepr(char c,char *buf){
	switch(c){
		case '\b': memcpy(buf,"\\b",3); return 3;
		case '\r': memcpy(buf,"\\r",3); return 3;
		case '\n': memcpy(buf,"\\n",3); return 3;
		default:
			if(c<27){
				sprintf(buf,"^%c",c+'@');
				return 2;
			}
			if(c>=32&&c<127){
				buf[0]=c;
				buf[1]='\0';
				return 1;
			}
			sprintf(buf,"\\x%02X",(int)c);
			return 4;
	}
}

String* b_linerepr(const Buffer *b,int linenum){
	char buf[5];
	int len=textbuf_linelen(b->tb,linenum);
	const char *data=string_data(textbuf_line(b->tb,linenum));
	String *res=string_empty(len+16);
	for(int i=0;i<len;i++){
		int cwid=charrepr(data[i],buf);
		string_append_b(res,buf,cwid);
	}
	return res;
}

static int boxprintline(int atx,int aty,int w,int h,const String *line){
	char buf[5];
	int len=string_length(line);
	const char *data=string_data(line);
	int x=atx,y=aty;
	for(int i=0;i<len;i++){
		int cwid=charrepr(data[i],buf);
		if(x==atx+w||(cwid>=w&&x+cwid>atx+w)){
			x=atx;
			y++;
			if(y==aty+h)break;
		}
		if(y==aty+h-1&&x+cwid>atx+w)break;
		int j;
		for(j=0;j<cwid&&x<atx+w;j++,x++)tputc(buf[j]);
		if(j<cwid){
			x=atx;
			y++;
		}
		for(;j<cwid;j++,x++)tputc(buf[j]);
	}
	return y-aty+1;
}

void b_draw(const Buffer *b,int atx,int aty,int w,int h){
	int nl=b_nlines(b);
	int nlnumwid=nl==0?1:(int)log10(nl)+1;
	if(w<nlnumwid+2){
		setbg(1);
		fillrect(atx,aty,w,h,' ');
		setbg(9);
		return;
	}

	setbg(9);
	setfg(3);
	int linenum=b->topy,drawy=aty;
	int loopend=nl==0?1:nl;
	int flushx=atx+nlnumwid+1;
	while(linenum<loopend){
		int numwid=(int)log10(linenum+1)+1;
		moveto(atx+nlnumwid-numwid,drawy);
		tprintf("%d",linenum+1);
		if(linenum==nl)break;

		int lheight=boxprintline(
			flushx,drawy,
			w-(nlnumwid+1),h-(drawy-aty),
			textbuf_line(b->tb,linenum));
		drawy+=lheight;

		linenum++;
	}
}

bool b_cursor_move(Buffer *b,Direction dir);

void b_insert(Buffer *b,const String *text);
