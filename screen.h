#pragma once

#include <string>
#include "io.h"

using namespace std;

namespace Screen {

struct Colourmode{
	IO::Colour fg,bg;
	bool ul; //underline

	Colourmode(void);
	Colourmode(IO::Colour _fg,IO::Colour _bg);
	Colourmode(IO::Colour _fg,IO::Colour _bg,bool _ul);
};

struct Screencell{
	int ch=' '; //character to be displayed
	Colourmode clr;
};
bool operator==(const Screencell &a,const Screencell &b);
bool operator!=(const Screencell &a,const Screencell &b);

void fillRect(Screencell *screen,
              unsigned int W,
              unsigned int x,unsigned int y,
              unsigned int width,unsigned int height,
              const Colourmode &clr);

void gotoFrontBufferCursor(void);

string prettychar(char c);

void redraw(void (*drawfunc)(Screencell*,unsigned int,unsigned int),bool copyover);
void redraw(bool forceredraw=false);

} //namespace Screen
