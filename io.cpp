#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <tuple>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cctype>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <math.h>
#include "io.h"
#include "screen.h"
#include "interface.h"
#include "script.h"
#include "either.h"

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
enum KEY_ACTION{
			KEY_NULL = 0,       /* NULL */
			CTRL_C = 3,         /* Ctrl-c */
			CTRL_D = 4,         /* Ctrl-d */
			CTRL_F = 6,         /* Ctrl-f */
			CTRL_H = 8,         /* Ctrl-h */
			TAB = 9,            /* Tab */
			CTRL_L = 12,        /* Ctrl+l */
			ENTER = 13,         /* Enter */
			CTRL_Q = 17,        /* Ctrl-q */
			CTRL_S = 19,        /* Ctrl-s */
			CTRL_U = 21,        /* Ctrl-u */
			ESC = 27,           /* Escape */
			BACKSPACE =  127,   /* Backspace */
			/* The following are just soft codes, not really reported by the
			 * terminal directly. */
			ARROW_LEFT = 1000,
			ARROW_RIGHT,
			ARROW_UP,
			ARROW_DOWN,
			DEL_KEY,
			HOME_KEY,
			END_KEY,
			PAGE_UP,
			PAGE_DOWN
};

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
	cout.flush(); //flush for popen
	FILE *proc=popen(cmd,"r");
	free(cmd);
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
	unsigned int w,h;
	try {
		w=stoi(gettput("cols"));
		h=stoi(gettput("lines"));
	} catch(...){
		w=h=0;
	}
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
#ifdef NOKERNINFO
		|NOKERNINFO //don't print a status line on ^T
#endif
		|IEXTEN //don't handle things like ^V specially
		//|ISIG //disable ^C ^\ and ^Z
		);
	tios.c_cc[VMIN]=0; //return each byte, or zero for timeout.
	tios.c_cc[VTIME]=1; //100 ms timeout( unit is tens of second)
						// This prevents pressing ESC in normal mode from
						// locking up.
	tcsetattr(0,TCSAFLUSH,&tios);

	try {
		screencolours=stoi(gettput("colors"));
	} catch(...){
		screencolours=16;
	}
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
	cout<<gettput("rmcup")<<flush;
}

unsigned int sqColourDiff(const Colour &a,const Colour &b){
	return (a.r-b.r)*(a.r-b.r)+(a.g-b.g)*(a.g-b.g)+(a.b-b.b)*(a.b-b.b);
}

unsigned int nearestAnsi(const Colour &clr){
	unsigned int mind=sqColourDiff(clr,ansiTable[0]),d;
	int minat=0;
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



/* Read a key from the terminal put in raw mode, trying to handle
 * escape sequences. */
int editorReadKey() {
	int fd = STDIN_FILENO;
    int nread;
    char c, seq[3];
    while ((nread = read(fd,&c,1)) == 0);
    if (nread == -1) exit(1);

    while(1) {
        switch(c) {
        case ESC:    /* escape sequence */
            /* If this is just an ESC, we'll timeout here. */
            if (read(fd,seq,1) == 0) return ESC;
            if (read(fd,seq+1,1) == 0) return ESC;

            /* ESC [ sequences. */
            if (seq[0] == '[') {
                if (seq[1] >= '0' && seq[1] <= '9') {
                    /* Extended escape, read additional byte. */
                    if (read(fd,seq+2,1) == 0) return ESC;
                    if (seq[2] == '~') {
                        switch(seq[1]) {
                        case '3': return DEL_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        }
                    }
                } else {
                    switch(seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                    }
                }
            }

            /* ESC O sequences. */
            else if (seq[0] == 'O') {
                switch(seq[1]) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
                }
            }
            break;
        default:
            return c;
        }
    }
}
// These change the way input works so that getLineStdin works properly
void disableTimeout() {
	struct termios tios;
	tcgetattr(0,&tios);

	tios.c_cc[VMIN] = 1;
	tios.c_cc[VTIME] = 0;
	tcsetattr(0, TCSAFLUSH, &tios);
}
void enableTimeout() { 
	struct termios tios;
	tcgetattr(0,&tios);
	
	tios.c_cc[VMIN] = 0;
	tios.c_cc[VTIME] = 1;
	tcsetattr(0, TCSAFLUSH, &tios);
}

string getLineStdin(unsigned int startx){
	//TODO more editing keys
	unsigned int scrwidth,scrheight;
	tie(scrwidth,scrheight)=screensize();
	unsigned int maxlen=startx>=scrwidth?-1:scrwidth-startx;
	string line,prettyline;
	unsigned int scrollx=0;
	// Kind of a hack
	disableTimeout();
	while(true){
		char c=cin.get();
		if(c=='\n'||c=='\r')break;
		else if(c==BACKSPACE){ //backspace
			if(line.size()){
				char back=line.back();
				line.pop_back();
				const string pret=Screen::prettychar(back);
				for(unsigned int i=pret.size();i>0;i--)prettyline.pop_back();
				if(scrollx>0&&prettyline.size()-scrollx<=0){
					scrollx=scrollx<maxlen*2/3?0:scrollx-maxlen*2/3;
					cout<<gettput("hpa "+to_string(startx))
					    <<(scrollx>0?"$":"")
					    <<prettyline.substr(scrollx,maxlen-(scrollx>0))
					    <<gettput("el")
					    <<flush;
				} else {
					string cubstr=gettput("cub "+to_string(pret.size()));
					cout<<cubstr<<string(pret.size(),' ')<<cubstr;
				}
			} else cout<<gettput("bel");
		} else if(c==ESC){ //escape
			line="";
			break;
		} else {
			string pret=Screen::prettychar(c);
			line.push_back(c);
			prettyline+=pret;
			if(prettyline.size()+pret.size()-scrollx<=maxlen-(scrollx>0)){
				cout<<pret;
			} else {
				scrollx+=maxlen*2/3;
				cout<<gettput("hpa "+to_string(startx))
				    <<'$'
				    <<prettyline.substr(scrollx,maxlen-(scrollx>0))
				    <<gettput("el")
				    <<flush;
			}
		}
	}
	enableTimeout();
	return line;
}


void fillRestOfLine(unsigned int fromx,char c=' '){
	unsigned int scrwidth=screensize().first;
	scrwidth-=fromx;
	while(scrwidth-->0)cout<<c;
}


string getEditorCommand(void){
	unsigned int scrwidth,scrheight;
	tie(scrwidth,scrheight)=screensize();
	gotoxy(0,scrheight-1);
	switchColourFg(Inter::textfg);
	switchColourBg(Inter::screenbg);
	cout<<':'<<string(scrwidth-1,' ')<<flush;
	gotoxy(1,scrheight-1);
	return getLineStdin(1);
}

enum CommandRet{
	CR_OK=0, //succesful execution of command (or reported failure)
	CR_FAIL, //fatal failure!
	CR_QUIT, //should quit editor

	CR_NEXT //try next command, this one is not applicable...
};

#define CALL_EDITOR_COMMAND_RETURN_NOTNEXT(cmd,cmd0,bang,nm) \
	{ \
		CommandRet ret=editorCommand##nm(cmd,cmd0,bang); \
		if(ret!=CR_NEXT)return ret; \
	}
#define CALL_EDITOR_COMMAND_RETURN_FAILQUIT(cmd,cmd0,bang,nm) \
	{ \
		CommandRet ret=editorCommand##nm(cmd,cmd0,bang); \
		if(ret==CR_FAIL||ret==CR_QUIT)return ret; \
	}

CommandRet editorCommandColonNum(vector<string> cmd,string cmd0,bool bang){
	if(cmd0.size()==2&&!(cmd0[1]-50)&&cmd0[bang]+1+!!cmd0.size()==cmd0[0]){
		Inter::printStatus("🐊");
		return CR_OK;
	}
	if(bang||cmd0.size()==0)return CR_NEXT;
	unsigned int target;
	unsigned int i=1;
	Inter::Filebuffer &fbuf=Inter::buffers[Inter::frontBuffer];
	unsigned int nln=fbuf.contents.numlines();
	if(cmd0[0]=='.')target=fbuf.cury+1;
	else if(cmd0[0]=='$')target=nln;
	else if(cmd0[0]>='0'&&cmd0[0]<='9'){
		target=cmd0[0]-'0';
		while(i<cmd0.size()&&cmd0[i]>='0'&&cmd0[i]<='9'){
			target=target*10+cmd0[i]-'0';
			i++;
		}
	} else return CR_NEXT;
	int sign,inc;
	bool hasnumber;
	while(i<cmd0.size()){
		if(cmd0[i]=='+')sign=1;
		else if(cmd0[i]=='-')sign=-1;
		else return CR_NEXT;
		i++;
		inc=0;
		hasnumber=false;
		while(i<cmd0.size()&&cmd0[i]>='0'&&cmd0[i]<='9'){
			inc=inc*10+cmd0[i]-'0';
			i++;
			hasnumber=true;
		}
		if(!hasnumber)inc=1;
		target+=inc*sign;
		if(target<=0||target>nln){
			cout<<gettput("bel")<<flush;
			Inter::printStatus("Line number "+to_string(target)+" out of range",red);
			return CR_OK;
		}
	}
	if(cmd.size()>1){
		Inter::printStatus(":number doesn't take arguments yet",red);
		return CR_OK;
	}
	fbuf.cury=target-1;
	Screen::redraw();
	return CR_OK;
}

CommandRet editorCommandQuit(vector<string> cmd,string cmd0,bool bang){
	if(!startswith("quit",cmd0))return CR_NEXT;
	if(cmd.size()!=1){
		Inter::printStatus(":q[uit] takes no arguments",red);
		return CR_OK;
	}
	if(Inter::frontBuffer==-1)return CR_QUIT;
	if(!bang&&Inter::buffers[Inter::frontBuffer].dirty){
		Inter::printStatus("Unsaved changes in buffer, force quit with :q[uit]!",red);
		return CR_OK;
	}
	Inter::buffers.erase(Inter::buffers.begin()+Inter::frontBuffer);
	if(Inter::buffers.size()==0)return CR_QUIT;
	if(Inter::frontBuffer>0&&Inter::buffers.size())
		Inter::frontBuffer--;
	Screen::redraw();
	return CR_OK;
}

CommandRet editorCommandQall(vector<string> cmd,string cmd0,bool bang){
	if(!startswith("qall",cmd0,2))return CR_NEXT;
	if(cmd.size()!=1){
		Inter::printStatus(":qa[ll] takes no arguments",red);
		return CR_OK;
	}
	if(!bang){
		for(int i=Inter::buffers.size()-1;i>=0;i--){
			if(!Inter::buffers[i].dirty)continue;
			Inter::printStatus("Unsaved changes in this buffer, force quit all with :qa[ll]!",red);
			Inter::frontBuffer=i;
			Screen::redraw();
			return CR_OK;
		}
	}
	Inter::buffers.clear();
	return CR_QUIT;
}

CommandRet editorCommandWrite(vector<string> cmd,string cmd0,bool bang){
	if(!startswith("write",cmd0)||bang)return CR_NEXT;
	if(cmd.size()>2){
		Inter::printStatus(":w[rite] takes 0 or 1 argument",red);
		return CR_OK;
	}
	if(Inter::frontBuffer==-1){
		Inter::printStatus("Can't write no buffer",red);
		return CR_OK;
	}
	Inter::Filebuffer &fbuf=Inter::buffers[Inter::frontBuffer];
	if(fbuf.openpath.size()==0&&cmd.size()==1){
		Inter::printStatus("Buffer not linked to a file, and no name given",red);
		return CR_OK;
	}
	const Maybe<string> err=cmd.size()==2?fbuf.saveas(cmd[1]):fbuf.save();
	if(err.isJust()){
		Inter::printStatus("Couldn't write: "+err.fromJust(),red);
		return CR_OK;
	}
	fbuf.dirty=false;
	Inter::printStatus("written");
	Screen::redraw();
	return CR_OK;
}

CommandRet editorCommandWq(vector<string> cmd,string cmd0,bool bang){
	if(cmd0!="wq")return CR_NEXT;
	CALL_EDITOR_COMMAND_RETURN_FAILQUIT(cmd,"write",false,Write)
	CALL_EDITOR_COMMAND_RETURN_FAILQUIT(cmd,"quit",bang,Quit)
	return CR_OK; //apparently Quit didn't quit
}

CommandRet editorCommandTabops(vector<string> cmd,string cmd0,bool bang){
	if(startswith("tabnext",cmd0,4)){
		if(bang)return CR_NEXT;
		if(Inter::buffers.size()<2)Inter::printStatus("No tab to switch to",red);
		else {
			Inter::frontBuffer=(Inter::frontBuffer+1)%Inter::buffers.size();
			Screen::redraw();
		}
		return CR_OK;
	} else if(startswith("tabprev",cmd0,4)||startswith("tabNext",cmd0,4)){
		if(bang)return CR_NEXT;
		if(Inter::buffers.size()<2)Inter::printStatus("No tab to switch to",red);
		else {
			Inter::frontBuffer=(Inter::frontBuffer+Inter::buffers.size()-1)
		                       %Inter::buffers.size();
			Screen::redraw();
		}
		return CR_OK;
	} else if(startswith("tabclose",cmd0,4)){
		if(Inter::frontBuffer==-1)Inter::printStatus("No tab to close",red);
		else if(!bang&&Inter::buffers[Inter::frontBuffer].dirty)
			Inter::printStatus("Unsaved changes in buffer, force close with :tabc!",red);
		else {
			Inter::buffers.erase(Inter::buffers.begin()+Inter::frontBuffer);
			if(Inter::buffers.size()==0)Inter::frontBuffer=-1;
			else if(Inter::frontBuffer>0)Inter::frontBuffer--;
			Screen::redraw();
		}
		return CR_OK;
	} else if(cmd0=="tabnew"||startswith("tabedit",cmd0,4)){
		if(bang)return CR_NEXT;
		if(cmd.size()==2)Inter::addFilebufferFile(cmd[1]);
		else if(cmd.size()==1)Inter::addFilebuffer();
		else Inter::printStatus(":tabnew takes 0 or 1 argument",red);
		Screen::redraw();
		return CR_OK;
	} else return CR_NEXT;
}

CommandRet editorCommandEdit(vector<string> cmd,string cmd0,bool bang){
	if(!startswith("edit",cmd0))return CR_NEXT;
	if(cmd.size()>2){
		Inter::printStatus(":e[dit] 0 or 1 parameter",red);
		return CR_OK;
	}
	if(Inter::buffers.size()==0||Inter::frontBuffer==-1){
		if(cmd.size()==1)Inter::addFilebuffer();
		else Inter::addFilebufferFile(cmd[1]);
	} else {
		Inter::Filebuffer &fbuf=Inter::buffers[Inter::frontBuffer];
		if(!bang&&fbuf.dirty){
			Inter::printStatus("Unsaved changes in buffer, force re-edit with :e[dit]!",red);
			return CR_OK;
		}
		if(cmd.size()==1)fbuf.open(fbuf.openpath,false);
		else fbuf.open(cmd[1],false);
		fbuf.curx=fbuf.cury=0;
		Screen::redraw();
	}
	return CR_OK;
}

CommandRet editorCommandLuafile(vector<string> cmd,string cmd0,bool bang){
	if(!startswith("luafile",cmd0,4)||bang)return CR_NEXT;
	if(cmd.size()!=2){
		Inter::printStatus(":luaf[ile] expects a filename parameter",red);
		return CR_OK;
	}
	const Maybe<string> mval=Script::runfile(cmd[1]);
	if(mval.isJust())Inter::printStatus(mval.fromJust(),red);
	return CR_OK;
}

CommandRet editorCommandLua(vector<string> cmd,string cmd0,bool bang){
	if(cmd0!="lua"||bang)return CR_NEXT;
	if(cmd.size()<2){
		Inter::printStatus(":lua expects code to be run",red);
		return CR_OK;
	}
	const Maybe<string> mval=Script::runcode(join(vector<string>(cmd.begin()+1,cmd.end()),' '));
	if(mval.isJust())Inter::printStatus(mval.fromJust(),red);
	return CR_OK;
}

Either<char,CommandRet> waitForKeyOrCommand(void);

static const char *kaas="36 .6,\n28 .17,\n20 .26,.\n12 .31,~~,  .\n10 .41,\n .,6 .44,\n20,:~=+5?+==~:11,~4?=5,.\n,:~~6:10,:+11?+=24,\n.:12~::7,:++6?=:12,5:10,.\n :17~3:23,7~==3~::5,\n :~~=~~3=~==13~::4,::13,10~3:5,.\n :6~=~==~4=22~3:11,::13,.\n :3~=~11=7~==~~=~=6+4=3~:25,\n :7~9=8~==3~=8+5=~27,.\n :6~==~~7=5~==3~=~~==3+?3+5=~4:25,\n :6~3=~==~4=~=3~3=~==3~=++3?3+5=~=~=3~::3,:8,:3,4:\n ,7~3=~~7=~==7+~~=++4?3+4=~9=3~::3,5:,4:,,.\n ,6~==4~4=~5=4?II+==8+5=~~14=3~16:,.\n  :5~16=+II4L~~4=4+4=~~26=~~8:,,\n  :5~=~~13=~+?4I3~:~~::8~10=+3=++4=+4?++==~~:,6:,\n  :5~17=~=3+~~==~~4:6~10=3+=+=7+3?++3=4~8:.\n  ,3~6=3+11=5~==~3=~~5:~12=10+=6+11=~~3:.\n  .~~6=??I??17=6+~~14=+=11+9=~==4+4=~~.\n3 ~6=+I5L+14=++?5+~10=+5=13+==~=~~==8+==~~\n3 .:4+=+L5$LLI11=+6?3+16=29+==~~\n4 ..,,:~??8I11=+?5I??++12=+==4+=24+==~:\n6 4.,,3:~~=++11=+?ILL3I??+11=+==30+==~:\n10 5.3,::=8+3=+IILLII3?7=+=++=+=31+=~:\n16 5.3,~??5+==+6?+4=+=++==34+==~:\n22 4.,,:~3?+3=3+4=6+=10+4=4+8=9+=~~:\n27 3.3,::??22+=5+==3+9=7+=~~,\n31 4.3,::=5?13+5?3+=++==II?++=~==5+=~~.\n36 4.3,::~I6?6+6?3+=+==3I??++3=3+=3~\n41 4.3,::~3I6?4I??++=+==ILLI??++==3+=3~\n46 4.3,::~5ILLII??5+=ILLII??+~=++=~~:\n50 5.,,4:~+II??6+=4LII?==++=~~:\n";
CommandRet editorCommandKaas(vector<string>,string cmd0,bool){
	if(cmd0!="kaas")return CR_NEXT;
	Inter::clearStatus();
	gotoxy(0,screensize().second-1);
	switchColourFg(yellow);
	switchColourBg(Inter::screenbg);
	size_t i,n,x=0;
	for(i=0;kaas[i];i++){
		if(kaas[i]>='0'&&kaas[i]<='9'){
			n=kaas[i]-'0';
			i++;
			while(kaas[i]>='0'&&kaas[i]<='9')n=10*n+kaas[i++]-'0';
			if(kaas[i]=='\n'){
				while(n-->0){
					fillRestOfLine(x);
					cout<<'\n';
					x=0;
				}
			} else {
				cout<<string(n,kaas[i]);
				x+=n;
			}
		} else if(kaas[i]=='\n'){
			fillRestOfLine(x);
			cout<<'\n';
			x=0;
		} else {
			cout<<kaas[i];
			x++;
		}
	}
	cout.flush();
	const Either<char,CommandRet> ei=waitForKeyOrCommand();
	if(ei.isRight()){
		CommandRet cr=ei.fromRight();
		if(cr==CR_NEXT)cr=CR_OK;
		return cr;
	} else return CR_OK;
}

CommandRet editorCommandVerboseChar(vector<string>,string cmd0,bool){
	if(cmd0!="verbosechar")return CR_NEXT;

	Inter::printStatus("Entering verbose character mode! ^V to quit.",red);
	usleep(1000000);
	switchColourFg(Inter::textfg);
	switchColourBg(Inter::screenbg);
	cout<<gettput("clear")<<flush;
	while(true){
		int c=editorReadKey();
		string s=to_string((int)c)+" ("+Screen::prettychar(c)+')';
		if(c=='\x16'){
			Screen::redraw(true);
			Inter::printStatus(s);
			break;
		} else {
			cout<<s<<endl;
		}
	}

	return CR_OK;
}

CommandRet evalEditorCommand(string scmd){
	bool bang=false;
	vector<string> cmd=splitSmart(scmd,' ');
	if(cmd.size()==0)return CR_OK;
	string cmd0=cmd[0];

	//remove leading `:`s
	size_t endpos=cmd0.find_first_not_of(":");
	if(endpos!=string::npos)cmd0=cmd0.substr(endpos,string::npos);

	if(cmd0.back()=='!'){
		bang=true;
		cmd0.pop_back();
	}

	CALL_EDITOR_COMMAND_RETURN_NOTNEXT(cmd,cmd0,bang,ColonNum)
	CALL_EDITOR_COMMAND_RETURN_NOTNEXT(cmd,cmd0,bang,Quit)
	CALL_EDITOR_COMMAND_RETURN_NOTNEXT(cmd,cmd0,bang,Write)
	CALL_EDITOR_COMMAND_RETURN_NOTNEXT(cmd,cmd0,bang,Wq)
	CALL_EDITOR_COMMAND_RETURN_NOTNEXT(cmd,cmd0,bang,Qall)
	CALL_EDITOR_COMMAND_RETURN_NOTNEXT(cmd,cmd0,bang,Tabops)
	CALL_EDITOR_COMMAND_RETURN_NOTNEXT(cmd,cmd0,bang,Edit)
	CALL_EDITOR_COMMAND_RETURN_NOTNEXT(cmd,cmd0,bang,Luafile)
	CALL_EDITOR_COMMAND_RETURN_NOTNEXT(cmd,cmd0,bang,Lua)
	CALL_EDITOR_COMMAND_RETURN_NOTNEXT(cmd,cmd0,bang,Kaas)
	CALL_EDITOR_COMMAND_RETURN_NOTNEXT(cmd,cmd0,bang,VerboseChar)

	Inter::printStatus("Unrecognised command :"+cmd0,red);
	return CR_NEXT;
}

#undef CALL_EDITOR_COMMAND_RETURN_NOTNEXT
#undef CALL_EDITOR_COMMAND_RETURN_FAILQUIT




//returns given char unless it was ':'
Either<char,CommandRet> waitForKeyOrCommand(void){
	Inter::printStatus("Enter key or type command to continue");
	int c=editorReadKey();
	if(c==':'){
		Screen::redraw(true);
		string cmd=getEditorCommand();
		if(cmd.size()==0){
			return Either<char,CommandRet>::Right(CR_OK);
		}
		return Either<char,CommandRet>::Right(evalEditorCommand(cmd));
	}
	Screen::redraw(true);
	return Either<char,CommandRet>::Left(c);
}

void moveLeft(unsigned int repcount, Inter::Filebuffer &fbuf){
	if(fbuf.curx>=repcount)fbuf.curx-=repcount;
	else {
		fbuf.curx=0;
		cout<<gettput("bel")<<flush;
	}
}

void moveRight(unsigned int repcount, Inter::Filebuffer &fbuf){
	const unsigned int llen=fbuf.contents.linelen(fbuf.cury);
	if(fbuf.curx+repcount<llen)fbuf.curx+=repcount;
	else {
		fbuf.curx=llen==0?0:llen-1;
		cout<<gettput("bel")<<flush;
	}
}


void moveUp(unsigned int repcount, Inter::Filebuffer &fbuf){
	if(fbuf.cury>=repcount)fbuf.cury-=repcount;
	else {
		fbuf.cury=0;
		cout<<gettput("bel")<<flush;
	}
	const unsigned int llen=fbuf.contents.linelen(fbuf.cury);
	if(llen==0)fbuf.curx=0;
	else if(fbuf.curx>0&&fbuf.curx>=llen)fbuf.curx=llen-1;
}


void moveDown(unsigned int repcount, Inter::Filebuffer &fbuf){
	const unsigned int nln=fbuf.contents.numlines();
	if(fbuf.cury+repcount<nln)fbuf.cury+=repcount;
	else {
		fbuf.cury=nln==0?0:nln-1;
		cout<<gettput("bel")<<flush;
	}
	const unsigned int llen=fbuf.contents.linelen(fbuf.cury);
	if(llen==0)fbuf.curx=0;
	else if(fbuf.curx>0&&fbuf.curx>=llen)fbuf.curx=llen-1;
}



// Insert mode for inserting input.
void insertModeRunLoop(void){
	if(Inter::frontBuffer==-1)
		throw logic_error("Cannot enter insert mode in no buffer");
	Screen::redraw();
	Inter::printStatus("[INSERT]",Inter::textfg,true);
	Inter::Filebuffer &fbuf=Inter::buffers[Inter::frontBuffer];
	while(true){
		int c=editorReadKey();
		if(c==ESC)break; //escape
		if(c==BACKSPACE){ //backspace
			if(fbuf.curx==0){
				if(fbuf.cury==0){
					cout<<gettput("bel")<<flush;
					continue;
				}
				fbuf.cury--;
				const unsigned int pllen=fbuf.contents.linelen(fbuf.cury);
				fbuf.contents.erase(pllen,fbuf.cury,1);
				fbuf.curx=pllen;
			} else {
				fbuf.contents.erase(fbuf.curx-1,fbuf.cury,1);
				fbuf.curx--;
			}
			fbuf.dirty=true;
			Screen::redraw();
			continue;
		}
		if(c==ARROW_RIGHT) {
			moveRight(1, fbuf);
			Screen::redraw();
			continue;
		}
		if(c==ARROW_LEFT) {
			moveLeft(1, fbuf);
			Screen::redraw();
			continue;
		}
		if(c==ARROW_UP) {
			moveUp(1, fbuf);
			Screen::redraw();
			continue;
		}
		if(c==ARROW_DOWN) {
			moveDown(1, fbuf);
			Screen::redraw();
			continue;
		}
		if(c==HOME_KEY) {
			moveLeft(INT_MAX, fbuf);
			Screen::redraw();
			continue;
		}
		if(c==END_KEY) {
			moveRight(INT_MAX, fbuf);
			Screen::redraw();
			continue;
		}
		if(c=='\r')c='\n'; //enter
		if(c=='\t'||c=='\n'||(c>=' '&&c<=(unsigned char)'\x80')){
			fbuf.contents.insert(fbuf.curx,fbuf.cury,c);
			if(c=='\n'){
				fbuf.curx=0;
				fbuf.cury++;
			} else fbuf.curx++;
			fbuf.dirty=true;
			Screen::redraw();
		} else {
			cout<<gettput("bel")<<flush;
			continue;
		}
	}
	fbuf.curx=fbuf.curx==0?0:fbuf.curx-1;
	Screen::redraw();
	Inter::clearStatus();
}

void moveToBeginAfterIndent(Inter::Filebuffer &fbuf){
	const unsigned int llen=fbuf.contents.linelen(fbuf.cury);
	unsigned int i;
	for(i=0;i<llen;i++){
		const char c=fbuf.contents.at(i,fbuf.cury);
		if(c!=' '&&c!='\t')break;
	}
	if(i<llen)fbuf.curx=i;
	else i=llen==0?0:llen-1;
}

bool jumpToNextOccurrenceOfChar(Inter::Filebuffer &fbuf,char c,unsigned int skip=0){
	const unsigned int llen=fbuf.contents.linelen(fbuf.cury);
	unsigned int i;
	for(i=fbuf.curx+1+skip;i<llen;i++){
		const char curchar=fbuf.contents.at(i,fbuf.cury);
		if(curchar==c){
			fbuf.curx=i;
			return true;
		}
	}
	return false;
}

bool jumpToPreviousOccurrenceOfChar(Inter::Filebuffer &fbuf,char c,int skip=0){
	int i;
	for(i=fbuf.curx-1-skip;i>=0;i--){
		const char curchar=fbuf.contents.at(i,fbuf.cury);
		if(curchar==c){
			fbuf.curx=i;
			return true;
		}
	}
	return false;
}

enum CharCat{
	CC_SPACE=0,
	CC_WORD,
	CC_PUNCT
};

CharCat charCategory(char c){
	if(isspace(c))return CC_SPACE;
	else if(isalnum(c)||c=='_')return CC_WORD;
	else return CC_PUNCT;
}

int runloop(void){
	unsigned int repcount;
	bool repcountset;
	while(true){
		if(Inter::frontBuffer==-1){
			if(Inter::buffers.size()==0)Inter::addFilebuffer();
			else Inter::frontBuffer=Inter::buffers.size()-1;
		}
		Inter::Filebuffer &fbuf=Inter::buffers[Inter::frontBuffer];
		int c=editorReadKey();
		repcount=1;
		repcountset=false;
		if(c>='1'&&c<='9'){
			repcountset=true;
			repcount=c-'0';
			while(true){
				c=editorReadKey();
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
					if(ret==CR_QUIT){
						//This resets the terminal to how it should be
						endscreen();
						return 0;
					}
					if(ret==CR_NEXT)break;
				}
			}
			Screen::gotoFrontBufferCursor();
			break;
		}
		case ESC:{
			if (repcount > 1)Screen::redraw(true);
			repcount=1;
			Inter::clearStatus();
			Screen::redraw();
			break;
		}
		case ARROW_UP:
		case 'k':{
			moveUp(repcount, fbuf);
			Screen::redraw();
			break;
		}
		case ARROW_DOWN:
		case 'j':{
			moveDown(repcount, fbuf);
			Screen::redraw();
			break;
		}
		case ARROW_RIGHT:
		case 'l':{
			moveRight(repcount, fbuf);
			Screen::redraw();
			break;
		}
		case ARROW_LEFT:
		case 'h':{
			moveLeft(repcount, fbuf);
			Screen::redraw();
			break;
		}
		case HOME_KEY:
			moveLeft(INT_MAX, fbuf);
			Screen::redraw();
			break;
		case END_KEY: {
			moveRight(INT_MAX, fbuf);
			Screen::redraw();
			break;
		}
		case BACKSPACE:{// BACKSPACE
			if(fbuf.curx>=repcount)fbuf.curx-=repcount;
			else fbuf.curx = 0;
			Screen::redraw();
			break;
		}
		case '{': {
			int y;
			bool sawfilledline=false;
			bool jumped=false;
			for(y=fbuf.cury;y>0;y--){
				size_t llen=fbuf.contents.linelen(y);
				if(llen>0)sawfilledline=true;
				else if(sawfilledline){
					fbuf.cury=y;
					fbuf.curx=0;
					jumped=true;
					break;
				}
			}
			if(!jumped){
				fbuf.cury=0;
				fbuf.curx=0;
			}
			Screen::redraw();
			break;
		}
		case '}': {
			unsigned int y;
			size_t numlines=fbuf.contents.numlines();
			bool sawfilledline=false;
			bool jumped=false;
			for(y=fbuf.cury;y<numlines;y++){
				size_t llen=fbuf.contents.linelen(y);
				if(llen>0)sawfilledline=true;
				else if(sawfilledline){
					fbuf.cury=y;
					fbuf.curx=0;
					jumped=true;
					break;
				}
			}
			if(!jumped){
				fbuf.cury=numlines-1;
				fbuf.curx=fbuf.contents.linelen(fbuf.cury)-1;
			}
			Screen::redraw();
			break;
		}
		case 'w':{
			unsigned int x=fbuf.curx,y=fbuf.cury;
			if(x>=fbuf.contents.linelen(y))break;
			const unsigned int nln=fbuf.contents.numlines();
			for(unsigned int i=0;i<repcount;i++){
				CharCat startcat=charCategory(fbuf.contents.at(x,y));
				while(y<nln){
					const unsigned int llen=fbuf.contents.linelen(y);
					while(x<llen){
						const char c=fbuf.contents.at(x,y);
						const CharCat cat=charCategory(c);
						if(cat==CC_SPACE)startcat=CC_SPACE;
						else if(cat!=startcat)break;
						x++;
					}
					if(llen&&x<llen)break;
					startcat=CC_SPACE; //for the newline
					x=0;
					y++;
				}
				if(nln&&y==nln)break;
			}
			if(nln&&y==nln){
				y=nln-1;
				const unsigned int llen=fbuf.contents.linelen(y);
				x=llen==0?0:llen-1;
			}
			fbuf.curx=x; fbuf.cury=y;
			Screen::redraw();
			break;
		}
		case 'b':{
			int x=fbuf.curx,y=fbuf.cury;
			if(x>=(int)fbuf.contents.linelen(y))break;
			for(unsigned int i=0;i<repcount;i++){
				CharCat startcat=charCategory(fbuf.contents.at(x,y));
				if(x==0){
					if(y==0)break;
					y--;
					const unsigned int llen=fbuf.contents.linelen(y);
					x=llen==0?0:llen-1;
				} else {
					x--;
				}
				while(x>=0){
					const char c=fbuf.contents.at(x,y);
					const CharCat cat=charCategory(c);
					if(cat!=CC_SPACE)break;
					x--;
				}
				while(x>=0){
					const char c=fbuf.contents.at(x,y);
					const CharCat cat=charCategory(c);
					if(cat!=startcat){
						x++;
						break;
					}
					x--;
				}
			}
			fbuf.curx=x; fbuf.cury=y;
			Screen::redraw();
			break;
		}
		case 'g':{
			int c2=editorReadKey();
			if(c2=='g'){
				fbuf.cury=0;
				fbuf.curx=0;
				Screen::redraw();
			} else {
				Inter::printStatus("Unrecognised command '"+Screen::prettychar(c)+Screen::prettychar(c2)+'\'',red);
				cout<<gettput("bel")<<flush;
			}
			break;
		}
		case 'G':{
			const unsigned int nln=fbuf.contents.numlines();
			unsigned int newy=nln-1;
			if(repcountset&&repcount>0)newy=repcount-1;
			fbuf.cury=min(newy,nln-1); // don't move past end of buffer
			moveToBeginAfterIndent(fbuf);
			Screen::redraw();
			break;
		}
		case '0':
			fbuf.curx=0;
			Screen::redraw();
			break;
		case '^':
			moveToBeginAfterIndent(fbuf);
			Screen::redraw();
			break;
		case '_':
		case '+': {
			if(repcountset||c=='+'){
				const unsigned int nln=fbuf.contents.numlines();
				if(c=='_')repcount--;
				unsigned int newy=min(fbuf.cury+repcount,nln-1); // don't move past end of buffer
				fbuf.cury=newy;
			}
			moveToBeginAfterIndent(fbuf);
			Screen::redraw();
			break;
		}
		case '-': {
			unsigned int newy=max((int)(fbuf.cury-repcount),0); // don't move past begin of buffer
			fbuf.cury=newy;

			moveToBeginAfterIndent(fbuf);
			Screen::redraw();
			break;
		}
		case '%':
			if(repcountset&&repcount<=100){
				const unsigned int nln=fbuf.contents.numlines();
				fbuf.cury=((repcount*nln+99)/100)-1;
				moveToBeginAfterIndent(fbuf);
			} else if(repcountset){
				cout<<gettput("bel")<<flush;
			} else {
				// TODO: go to matching paren and stuff.
			}
			Screen::redraw();
			break;
		case '$':{
			const unsigned int llen=fbuf.contents.linelen(fbuf.cury);
			fbuf.curx=llen==0?0:llen-1;
			Screen::redraw();
			break;
		}
		case 'i':
			insertModeRunLoop();
			break;
		case 'a':
			fbuf.curx++;
			insertModeRunLoop();
			break;
		case 'I':
			moveToBeginAfterIndent(fbuf);
			insertModeRunLoop();
			break;
		case 'A':{
			const unsigned int llen=fbuf.contents.linelen(fbuf.cury);
			fbuf.curx=llen;
			insertModeRunLoop();
			break;
		}
		case 'o':
			fbuf.contents.insertLineBefore(fbuf.cury+1,"");
			fbuf.cury++;
			fbuf.curx=0;
			fbuf.dirty=true;
			insertModeRunLoop();
			break;
		case 'O':
			fbuf.contents.insertLineBefore(fbuf.cury,"");
			fbuf.curx=0;
			fbuf.dirty=true;
			insertModeRunLoop();
			break;
		case 'f':{
			bool jumped=jumpToNextOccurrenceOfChar(fbuf,editorReadKey());
			if(jumped)Screen::redraw();
			else cout<<gettput("bel")<<flush;
			break;
		}
		case 'F':{
			bool jumped=jumpToPreviousOccurrenceOfChar(fbuf,editorReadKey());
			if(jumped)Screen::redraw();
			else cout<<gettput("bel")<<flush;
			break;
		}
		case 't':{
			bool jumped=jumpToNextOccurrenceOfChar(fbuf,editorReadKey(),1);
			if(jumped){
				fbuf.curx=fbuf.curx==0?fbuf.curx:fbuf.curx-1;
				Screen::redraw();
			}
			else cout<<gettput("bel")<<flush;
			break;
		}
		case 'T':{
			bool jumped=jumpToPreviousOccurrenceOfChar(fbuf,editorReadKey(),1);
			if(jumped){
				const unsigned int llen=fbuf.contents.linelen(fbuf.cury);
				const unsigned int loc=fbuf.curx+1;
				fbuf.curx=loc>=llen?(llen==0?0:llen-1):loc;
				Screen::redraw();
			}
			else cout<<gettput("bel")<<flush;
			break;
		}
		case 'C':{
			const unsigned int llen=fbuf.contents.linelen(fbuf.cury);
			if(llen!=0)fbuf.dirty=true;
			fbuf.contents.erase(fbuf.curx,fbuf.cury,llen-fbuf.curx);
			insertModeRunLoop();
			break;
		}
		case 's':{
			const unsigned int llen=fbuf.contents.linelen(fbuf.cury);
			if(llen!=0){
				fbuf.dirty=true;
				fbuf.contents.erase(fbuf.curx,fbuf.cury,1);
			}
			insertModeRunLoop();
			break;
		}
		case 'S':{
			const unsigned int llen=fbuf.contents.linelen(fbuf.cury);
			if(llen!=0){
				fbuf.dirty=true;
				fbuf.contents.erase(0,fbuf.cury,llen);
			}
			insertModeRunLoop();
			break;
		}
		case 'r':{
			int replaceChar=editorReadKey();
			if(replaceChar==ESC)break; // escape
			if(!fbuf.contents.exists(fbuf.curx,fbuf.cury)){
				cout<<gettput("bel")<<flush;
				break;
			}

			fbuf.dirty=true;
			fbuf.contents.overwrite(fbuf.curx,fbuf.cury,replaceChar);
			Screen::redraw();

			break;
		}
		case 'x':{
			const unsigned int llen=fbuf.contents.linelen(fbuf.cury);
			if(llen==0)break;
			fbuf.dirty=true;
			fbuf.contents.erase(fbuf.curx,fbuf.cury,1);
			if(fbuf.curx==llen-1)fbuf.curx--;
			Screen::redraw();
			break;
		}
		case 'd':{
			int c2=editorReadKey();
			if(c2=='d'){
				fbuf.contents.removeLine(fbuf.cury);
				fbuf.dirty=true;// mark as file edited. TODO: check to see if file is empty first

				const unsigned int nln=fbuf.contents.numlines();
				unsigned int newy=min(fbuf.cury,nln-1); // don't move past end of buffer
				fbuf.cury=newy;
				moveToBeginAfterIndent(fbuf);

				Screen::redraw();
			} else {
				// TODO: support actual motions
				cout<<gettput("bel")<<flush;
			}
			break;
		}
		case 'D':{
			const unsigned int llen=fbuf.contents.linelen(fbuf.cury);
			if(llen!=0)fbuf.dirty=true;// mark as file edited
			fbuf.contents.erase(fbuf.curx,fbuf.cury,llen-fbuf.curx);
			if(fbuf.curx>0)fbuf.curx--;
			Screen::redraw();
			break;
		}
		case '\x0C':{ //^L
			screensizestore=queryscreensize();
			Inter::clearStatus();
			Screen::redraw(true);
			break;
		}
		default:
			Inter::printStatus("Unrecognised command '"+Screen::prettychar(c)+'\'',red);
		}
	}
	return 0;
}

} //namespace IO
