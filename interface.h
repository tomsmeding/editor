#pragma once

#include <vector>
#include <string>
#include "textblob.h"
#include "screen.h"
#include "maybe.h"
#include "util.h"

using namespace std;


namespace Inter {

extern const IO::Colour screenbg,screenfg,tabbg,editbg,textfg,numberfg;

class Filebuffer{
public:
	unsigned int id=uniqid();
	unsigned int curx=0,cury=0,screencurx=0,screencury=0,scrolly=0;
	bool dirty=false;
	string openpath;
	Textblob contents;

	Filebuffer(void) = default;

	bool cansave(void);
	Maybe<string> save(void); //maybe returns an error
	Maybe<string> saveas(string fname); //maybe returns an error

	bool canopen(void);
	bool open(string fname,bool doredraw=true);
};


extern vector<Filebuffer> buffers;

extern int frontBuffer;


Filebuffer& addfilebuffer(bool doredraw=true);
Filebuffer& addfilebufferfile(const string &fname,bool doredraw=true);

void printStatus(string status,IO::Colour clr=textfg,bool bold=false);
void clearStatus(void);

void drawScreen(Screen::Screencell *screen,unsigned int width,unsigned int height);

} //namespace Inter
