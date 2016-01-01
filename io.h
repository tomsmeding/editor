#pragma once

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
void gotoxy(unsigned int x,unsigned int y);
void initscreen(void);
void endscreen(void);

void switchColourFg(const Colour &clr);
void switchColourBg(const Colour &clr);
void switchColourUl(bool ul);

}

