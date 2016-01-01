#pragma once

#include <vector>
#include <string>
#include "textblob.h"
#include "screen.h"
#include "util.h"

using namespace std;


namespace Inter {

struct Key{
	int ch;
	bool ctrl,shift,meta;
};

class Filebuffer{
public:
	unsigned int id=uniqid();
	int scrolly=0;
	string openpath;
	Textblob contents;

	Filebuffer(void) = default;

	bool cansave(void);
	bool save(void);
	bool saveas(string fname);

	bool canopen(void);
	bool open(string fname);

	bool keypress(Key key);
};


extern vector<Filebuffer> buffers;

extern int frontBuffer;


Filebuffer& addfilebuffer(void);
Filebuffer& addfilebuffer(const string &fname);

void drawScreen(Screen::Screencell *screen,unsigned int width,unsigned int height);

} //namespace Inter
