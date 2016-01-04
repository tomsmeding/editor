#include <iostream>
#include <string>
#include <unordered_map>
#include <tuple>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include "io.h"
#include "interface.h"

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

pair<unsigned int,unsigned int> screensizestore={80,25};

pair<unsigned int,unsigned int> queryscreensize(void){
	unsigned int w=stoi(gettput("cols")),h=stoi(gettput("lines"));
	if(w<1)w=80;
	if(h<1)h=25;
	return {w,h};
}

pair<unsigned int,unsigned int> screensize(void){
	return screensizestore;
}

void gotoxy(unsigned int x,unsigned int y){
	/*char *s;
	asprintf(&s,"cup %u %u",y,x);
	cout<<gettput(s);
	free(s);*/
	cout<<"\x1B["<<y+1<<';'<<x+1<<'H'; //yeah.
}

void winchhandler(int){
	screensizestore=queryscreensize();
}

struct termios tios_bak;

void initscreen(void){
	signal(SIGWINCH,winchhandler);
	screensizestore=queryscreensize();

	struct termios tios;
	tcgetattr(0,&tios_bak);
	tios=tios_bak;
	tios.c_lflag&=~(
		ECHO|ECHOE|ECHOKE //no echo of normal characters, erasing and killing
		|ECHOCTL //don't visibly echo control characters (^V etc.)
		|ECHONL //don't even echo a newline
		|ICANON //disable canonical mode
		|NOKERNINFO //don't print a status line on ^T
		|IEXTEN //don't handle things like ^V specially
		//|ISIG //disable ^C ^\ and ^Z
		);
	tios.c_cc[VMIN]=1; //read one char at a time
	tios.c_cc[VTIME]=0; //no timeout on reading, make it a blocking read
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
	cout<<tps;
	free(s);
}

void switchColourBg(const Colour &clr){
	char *s;
	if(screencolours==16)asprintf(&s,"setab %u",nearestAnsi(clr));
	else if(screencolours==256)asprintf(&s,"setab %u",nearest256(clr));
	else return;
	string tps=gettput(s);
	cout<<tps;
	free(s);
}

void switchColourUl(bool ul){
	if(ul)cout<<gettput("smul");
	else cout<<gettput("rmul");
}

void turnOnBold(void){
	cout<<gettput("bold");
}

void clearMarkup(void){
	cout<<gettput("sgr0");
}


void printStatus(string status,Colour clr=Inter::textfg,bool bold=false){
	unsigned int scrwidth,scrheight;
	tie(scrwidth,scrheight)=screensize();
	gotoxy(0,scrheight-1);
	switchColourFg(clr);
	switchColourBg(Inter::screenbg);
	if(bold)turnOnBold();
	if(status.size()>scrwidth){
		cout<<status.substr(0,scrwidth-3)<<"...";
	} else {
		cout<<status<<string(scrwidth-status.size(),' ');
	}
	if(bold)clearMarkup();
	Screen::gotoFrontBufferCursor();
	cout.flush();
}

void clearStatus(void){
	unsigned int scrwidth,scrheight;
	tie(scrwidth,scrheight)=screensize();
	gotoxy(0,scrheight-1);
	switchColourBg(Inter::screenbg);
	cout<<string(scrwidth,' ');
	Screen::gotoFrontBufferCursor();
	cout.flush();
}

string getEditorCommand(void){
	//TODO more editing keys
	unsigned int scrwidth,scrheight;
	tie(scrwidth,scrheight)=screensize();
	gotoxy(0,scrheight-1);
	switchColourFg(Inter::textfg);
	switchColourBg(Inter::screenbg);
	cout<<':'<<string(scrwidth-1,' ')<<flush;
	gotoxy(1,scrheight-1);
	string line;
	while(true){
		char c=cin.get();
		if(c=='\n'||c=='\r')break;
		else if(c=='\x7f'){ //backspace
			if(line.size()){
				char back=line.back();
				line.pop_back();
				string pret=Screen::prettychar(back);
				string cubstr=gettput("cub "+to_string(pret.size()));
				cout<<cubstr<<string(pret.size(),' ')<<cubstr;
			} else cout<<gettput("bel");
		} else if(c=='\x1B'){ //escape
			return "";
		} else {
			string pret=Screen::prettychar(c);
			line.push_back(c);
			cout<<pret;
		}
	}
	return line;
}

enum CommandRet{
	CR_SUCCESS=0,
	CR_FAIL=1,
	CR_QUIT=2
};

CommandRet evalEditorCommand(string cmd){
	bool bang=false;
	cmd=trim(cmd);
	if(cmd.back()=='!'){
		bang=true;
		cmd.pop_back();
	}

	if(string("quit").find(cmd)==0){
		bool unsavedChanges = false; // TODO: check if buffer content has changed.
		if(!bang&&unsavedChanges){
			printStatus("Unsaved changes in buffer, force quit with :q[uit]!",red);
			return CR_SUCCESS;
		}
		return CR_QUIT;
	}
	printStatus("Unrecognised command :"+cmd,red);
	return CR_SUCCESS;
}

void insertModeRunLoop(void){
	printStatus("<<INSERT>>",Inter::textfg,true);
}

int runloop(void){
	unsigned int repcount;
	Inter::Filebuffer *fbuf=Inter::frontBuffer!=-1?&Inter::buffers[Inter::frontBuffer]:NULL;
	while(true){
		char c=cin.get();
		repcount=1;
		if(c>'0'&&c<='9'){
			repcount=c-'0';
			while(true){
				c=cin.get();
				if(c<'0'||c>'9')break;
				repcount=10*repcount+c-'0';
			}
		}
		switch(c){
		case ':':{ //editor command
			string cmd=getEditorCommand();
			if(cmd.size()){
				while(repcount-->0){
					CommandRet ret=evalEditorCommand(cmd);
					if(ret==CR_FAIL)return 1;
					if(ret==CR_QUIT)return 0;
				}
			}
			Screen::gotoFrontBufferCursor();
			break;
		}
		case 'h': case 'j': case 'k': case 'l': case '0': case '$':
			if(!fbuf)printStatus("No buffer open!",red);
			else switch(c){
				case 'h':
					if(fbuf->curx>=repcount)fbuf->curx-=repcount;
					else {
						fbuf->curx=0;
						cout<<gettput("bel")<<flush;
					}
					break;
				case 'j':{
					const unsigned int nln=fbuf->contents.numlines();
					if(fbuf->cury+repcount<nln)fbuf->cury+=repcount;
					else {
						fbuf->cury=nln==0?0:nln-1;
						cout<<gettput("bel")<<flush;
					}
					const unsigned int llen=fbuf->contents.linelen(fbuf->cury);
					if(llen==0)fbuf->curx=0;
					else if(fbuf->curx>0&&fbuf->curx>=llen)fbuf->curx=llen-1;
					break;
				}
				case 'k':{
					if(fbuf->cury>=repcount)fbuf->cury-=repcount;
					else {
						fbuf->cury=0;
						cout<<gettput("bel")<<flush;
					}
					const unsigned int llen=fbuf->contents.linelen(fbuf->cury);
					if(llen==0)fbuf->curx=0;
					else if(fbuf->curx>0&&fbuf->curx>=llen)fbuf->curx=llen-1;
					break;
				}
				case 'l': {
					const unsigned int llen=fbuf->contents.linelen(fbuf->cury);
					if(fbuf->curx+repcount<llen)fbuf->curx+=repcount;
					else {
						fbuf->curx=llen==0?0:llen-1;
						cout<<gettput("bel")<<flush;
					}
					break;
				}
				case '0': {
					fbuf->curx=0;
					break;
				}
				case '$': {
					const unsigned int llen=fbuf->contents.linelen(fbuf->cury);
					fbuf->curx=llen==0?0:llen-1;
					break;
				}
			}
			Screen::redraw();
			break;
		case 'i':
			insertModeRunLoop();
			break;
		case '\x16': //^V
			printStatus("Entering verbose character mode!",red);
			usleep(1000000);
			switchColourFg(Inter::textfg);
			switchColourBg(Inter::screenbg);
			cout<<gettput("clear")<<flush;
			while(true){
				char c=cin.get();
				cout<<(int)c<<" ("<<Screen::prettychar(c)<<')'<<endl;
			}
			break;
		default:
			printStatus("Unrecognised command '"+string(1,c)+'\'',red);
		}
	}
	return 0;
}

} //namespace IO
