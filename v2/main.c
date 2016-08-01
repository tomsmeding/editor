#include <stdio.h>
#include <stdlib.h>
#include <termio.h>

#include "buffer.h"
#include "disk.h"

static void drawbox(int x,int y,int w,int h){
	moveto(x,y);
	tputc('+');
	for(int i=0;i<w-2;i++)tputc('-');
	tputc('+');
	for(int i=0;i<h-2;i++){
		moveto(x,y+i+1);
		tputc('|');
		moveto(x+w-1,y+i+1);
		tputc('|');
	}
	moveto(x,y+h-1);
	tputc('+');
	for(int i=0;i<w-2;i++)tputc('-');
	tputc('+');
}

static void drawboxaround(int x,int y,int w,int h){
	drawbox(x-1,y-1,w+2,h+2);
}

static void closecatstderr(void){
	fclose(stderr);
	FILE *f=fopen("editor.log","r");
	char buf[1024];
	bool havedata=false;
	while(true){
		int nwr=fread(buf,1,sizeof(buf),f);
		if(nwr<=0)break;
		if(!havedata){
			havedata=true;
			printf("STDERR DATA:\n");
		}
		fwrite(buf,1,nwr,stdout);
	}
	fclose(f);
}

static void reroutestderr(void){
	freopen("editor.log","w",stderr);
	atexit(closecatstderr);
}

int main(int argc,char **argv){
	reroutestderr();

	initscreen();
	//atexit(endscreen);
	initkeyboard();
	//atexit(endkeyboard);
	installCLhandler(true);

	Buffer *b=b_make();
	if(argc>=2){
		String *s=disk_read(argv[1]);
		if(!s)s=string_fromcstr("Could not read this file");
		b_insert(b,s);
		string_destroy(s);
	}
	drawboxaround(2,2,9,7); b_draw(b,2,2,9,7);
	redraw();
	tgetkey();
	b_destroy(b);

	endkeyboard();
	endscreen();
}
