#include <iostream>
#include <tuple>
#include <cassert>
#include "interface.h"
#include "screen.h"

using namespace std;

namespace Screen {

Colourmode::Colourmode(void)
	:ul(false){}
Colourmode::Colourmode(IO::Colour _fg,IO::Colour _bg)
	:fg(_fg),bg(_bg),ul(false){}
Colourmode::Colourmode(IO::Colour _fg,IO::Colour _bg,bool _ul)
	:fg(_fg),bg(_bg),ul(_ul){}

unsigned int prevwidth=-1,prevheight=-1;
Screencell *prevscreen=NULL;

inline bool operator==(const Screencell &a,const Screencell &b){
	return a.ch==b.ch&&a.clr.ul==b.clr.ul&&a.clr.fg==b.clr.fg&&a.clr.bg==b.clr.bg;
}

inline bool operator!=(const Screencell &a,const Screencell &b){
	return !(a==b);
}

void fillRect(Screencell *screen,unsigned int W,unsigned int x,unsigned int y,unsigned int width,unsigned int height,const Colourmode &clr){
	/*cerr<<"fillRect called with clr: "
	    <<(int)clr.fg.r<<','<<(int)clr.fg.g<<','<<(int)clr.fg.b<<"  "
	    <<(int)clr.bg.r<<','<<(int)clr.bg.g<<','<<(int)clr.bg.b<<"  "
	    <<clr.ul<<endl;*/
	unsigned int i,j;
	for(i=y;i<y+height;i++){
		for(j=x;j<x+width;j++){
			Screencell &cell=screen[W*i+j];
			cell.clr=clr;
		}
	}
}

char dec2hexChar(int n){
	if(n<10)return '0'+n;
	else return 'a'+n;
}

string prettychar(char c){
	if(c==0)return "\\0";
	if(c==9)return "\t";
	if(c==10)return "\n";
	if(c<28)return string(1,'^')+(char)('A'+c-1);
	if(c<32)return string("\\x")+(char)dec2hexChar(c/16)+(char)dec2hexChar(c%16);
	return string(1,c);
}

void copylinetoscreen(const Screencell *screen,unsigned int W,unsigned int x,unsigned int y,unsigned int width){
	IO::gotoxy(x,y);
	Colourmode clrmode=screen[W*y+x].clr;
	IO::switchColourFg(clrmode.fg);
	IO::switchColourBg(clrmode.bg);
	for(unsigned int i=x;i<x+width;i++){
		if(screen[W*y+i].clr.fg!=clrmode.fg){
			clrmode.fg=screen[W*y+i].clr.fg;
			IO::switchColourFg(clrmode.fg);
		}
		if(screen[W*y+i].clr.bg!=clrmode.bg){
			clrmode.bg=screen[W*y+i].clr.bg;
			IO::switchColourBg(clrmode.bg);
		}
		if(screen[W*y+i].clr.ul!=clrmode.ul){
			clrmode.ul=screen[W*y+i].clr.ul;
			IO::switchColourUl(clrmode.ul);
		}
		cout<<(unsigned char)screen[W*y+i].ch;
	}
}

void copytoscreen(const Screencell *screen,unsigned int W,unsigned int x,unsigned int y,unsigned int width,unsigned char height){
	for(unsigned int i=y;i<y+height;i++)
		copylinetoscreen(screen,W,x,i,width);
}

void redraw(void){
	//cerr<<"redraw called"<<endl;
	unsigned int scrwidth,scrheight;
	tie(scrwidth,scrheight)=IO::screensize();
	Screencell *newscreen=new Screencell[scrwidth*scrheight];
	Inter::drawScreen(newscreen,scrwidth,scrheight);
	if(!prevscreen||scrwidth!=prevwidth||scrheight!=prevheight){
		copytoscreen(newscreen,scrwidth,0,0,scrwidth,scrheight);
		if(prevscreen)delete[] prevscreen;
		prevwidth=scrwidth;
		prevheight=scrheight;
		prevscreen=new Screencell[scrwidth*scrheight];
		assert(prevscreen);
		memcpy(prevscreen,newscreen,scrwidth*scrheight*sizeof(Screencell));
		return;
	}
	unsigned int x,y;
	for(y=0;y<scrheight;y++){
		for(x=0;x<scrwidth;x++){
			if(prevscreen[scrwidth*y+x]!=newscreen[scrwidth*y+x])break;
		}
		if(x<scrwidth)copylinetoscreen(newscreen,scrwidth,0,y,scrwidth);
	}
	IO::gotoxy(0,scrheight-1);
	cout.flush();
}

} //namespace Screen
