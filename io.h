#pragma once

#include <string>
#include <utility>

using namespace std;

namespace IO {

struct Colour{
	unsigned char r,g,b;

	Colour(void);
	Colour(unsigned char _r,unsigned char _g,unsigned char _b);
};
inline bool operator==(const Colour &a,const Colour &b){
	return a.r==b.r&&a.g==b.g&&a.b==b.b;
}

inline bool operator!=(const Colour &a,const Colour &b){
	return !(a==b);
}

extern Colour black,red,green,yellow,blue,magenta,cyan,white,ansic[8];


pair<unsigned int,unsigned int> screensize(void);
string gettput(string);
void gotoxy(unsigned int x,unsigned int y);
void initscreen(void);
void endscreen(void);

void switchColourFg(const Colour &clr);
void switchColourBg(const Colour &clr);
void switchColourUl(bool ul);
void turnOnBold(void); //because freakin terminals don't support turning OFF bold
void clearMarkup(void);

int editorReadKey(int fd); // Read an input key

string getLineStdin(unsigned int startx=0);

int runloop(void);

}

