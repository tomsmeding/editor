#include <iostream>
#include <string>
#include <unordered_map>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <termios.h>
#include "io.h"

using namespace std;

namespace IO {

Colour::Colour(void)
	:r(0),g(0),b(0){}
Colour::Colour(unsigned char _r,unsigned char _g,unsigned char _b)
	:r(_r),g(_g),b(_b){}

Colour black  ={  0,  0,  0},
       red    ={255,  0,  0},
       green  ={  0,255,  0},
       yellow ={255,255,  0},
       blue   ={  0,  0,255},
       magenta={255,  0,255},
       cyan   ={  0,255,255},
       white  ={255,255,255};
Colour ansiTable[8]={black,red,green,yellow,blue,magenta,cyan,white};

unsigned int screencolours;

unordered_map<string,string> tputcache;

/*
alternate screen: smcup, rmcup
dimensions: cols, lines
#colours: colors
clear screen: clear
goto pos: cup %d %d
*/
string gettput(string cap){
	auto it=tputcache.find(cap);
	if(it!=tputcache.end())return it->second;
	char *cmd;
	asprintf(&cmd,"/usr/bin/tput %s",cap.data());
	free(cmd);
	cout.flush(); //flush for popen
	FILE *proc=popen(cmd,"r");
	if(proc==NULL)return string();
	char buf[64]; //should be enough
	size_t nread=fread(buf,1,64,proc);
	pclose(proc);
	string res(buf,nread);
	tputcache[cap]=res;
	return res;
}

pair<unsigned int,unsigned int> screensize(void){
	unsigned int w=stoi(gettput("cols")),h=stoi(gettput("lines"));
	if(w<1)w=80;
	if(h<1)h=25;
	return {w,h};
}

void gotoxy(unsigned int x,unsigned int y){
	/*char *s;
	asprintf(&s,"cup %u %u",y,x);
	cout<<gettput(s);
	free(s);*/
	cout<<"\x1B["<<y+1<<';'<<x+1<<'H'; //yeah.
}

struct termios tios_bak;

void initscreen(void){
	struct termios tios;
	tcgetattr(0,&tios_bak);
	tios=tios_bak;
	tios.c_lflag&=~(ECHO|ICANON);
	tios.c_cc[VMIN]=1;
	tios.c_cc[VTIME]=0;
	tcsetattr(0,TCSAFLUSH,&tios);

	screencolours=stoi(gettput("colors"));
	// if(screencolours>256*256*256)screencolours=256*256*256;
	if(screencolours>256)screencolours=256; //we can't handle 24-bit true colour yet
	else if(screencolours<16)screencolours=16;
	//cerr<<screencolours<<" colours"<<endl;
	//pair<unsigned int,unsigned int> p=screensize();
	//cerr<<"Screen size: "<<p.first<<'*'<<p.second<<endl;

	cout<<gettput("smcup")<<gettput("clear")<<flush;
}

void endscreen(void){
	tcsetattr(0,TCSAFLUSH,&tios_bak);
	cout<<gettput("rmcup");
}

unsigned int sqColourDiff(const Colour &a,const Colour &b){
	return (a.r-b.r)*(a.r-b.r)+(a.g-b.g)*(a.g-b.g)+(a.b-b.b)*(a.b-b.b);
}

unsigned int nearestAnsi(const Colour &clr){
	unsigned int mind=sqColourDiff(clr,ansiTable[0]),d;
	int minat;
	int i;
	for(i=1;i<16;i++){
		d=sqColourDiff(clr,ansiTable[i]);
		if(d<mind){
			mind=d;
			minat=i;
		}
	}
	return minat;
}

unsigned int nearest256(const Colour &clr){
	const unsigned int red=(unsigned int)clr.r*5/255,
	                   green=(unsigned int)clr.g*5/255,
	                   blue=(unsigned int)clr.b*5/255,
	                   avg=((unsigned int)clr.r+clr.g+clr.b)/3;
	int avg10=(avg+2)/10-1;
	if(avg10<0)avg10=0;
	else if(avg10>23)avg10=23;
	const Colour cubeClr(red*255/5,green*255/5,blue*255/5),
	             rampClr(avg10*10-2,avg10*10-2,avg10*10-2);
	unsigned int res;
	if(sqColourDiff(clr,rampClr)<=sqColourDiff(clr,cubeClr))res=232+avg10;
	else res=16+36*red+6*green+blue;
	//cerr<<"nearest256("<<(int)clr.r<<','<<(int)clr.g<<','<<(int)clr.b<<")="<<res<<endl;
	return res;
}

void switchColourFg(const Colour &clr){
	char *s;
	if(screencolours==16)asprintf(&s,"setaf %u",nearestAnsi(clr));
	else if(screencolours==256)asprintf(&s,"setaf %u",nearest256(clr));
	else return;
	string tps=gettput(s);
	//cerr<<tps;
	cout<<tps;
	free(s);
}

void switchColourBg(const Colour &clr){
	char *s;
	if(screencolours==16)asprintf(&s,"setab %u",nearestAnsi(clr));
	else if(screencolours==256)asprintf(&s,"setab %u",nearest256(clr));
	else return;
	string tps=gettput(s);
	cerr<<tps;
	cout<<tps;
	free(s);
}

void switchColourUl(bool ul){
	if(ul)cout<<gettput("smul");
	else cout<<gettput("rmul");
}

} //namespace IO
