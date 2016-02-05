#include <iostream>
#include <tuple>
#include <cmath>
#include <climits>
#include "interface.h"
#include "disk.h"
#include "either.h"

using namespace std;

namespace Inter {

const IO::Colour
	screenbg=IO::Colour(0,0,0),
	screenfg=IO::Colour(198,198,198),
	tabbg=IO::Colour(38,38,38),
	editbg=IO::Colour(38,38,38),
	textfg=IO::Colour(255,255,255),
	numberfg=IO::Colour(255,255,0);

vector<Filebuffer> buffers;

int frontBuffer=-1;


bool Filebuffer::cansave(void){
	return openpath.size()!=0;
}

Maybe<string> Filebuffer::save(void){
	return Disk::writeToFile(openpath,to_string(contents));
}

Maybe<string> Filebuffer::saveas(string fname){
	openpath=fname;
	return save();
}

bool Filebuffer::canopen(void){
	return true;
}

bool Filebuffer::open(string fname,bool doredraw){
	openpath=fname;
	Either<string,string> errcont=Disk::readFromFile(fname);
	if(errcont.isLeft()){
		printStatus(errcont.fromLeft(),IO::red);
		contents.clear();
		dirty=true;
	} else {
		contents=Textblob(errcont.fromRight());
		dirty=false;
	}
	if(doredraw)Screen::redraw();
	return true;
}


Filebuffer* filebufferById(unsigned int id){
	unsigned int i;
	unsigned int nb=buffers.size();
	for(i=0;i<nb;i++)if(buffers[i].id==id)return &buffers[i];
	return NULL;
}

Filebuffer& addFilebuffer(bool doredraw){
	frontBuffer++;
	buffers.emplace(buffers.begin()+frontBuffer);
	if(doredraw)Screen::redraw();
	return buffers[frontBuffer];
}

Filebuffer& addFilebufferFile(const string &fname,bool doredraw){
	frontBuffer++;
	buffers.emplace(buffers.begin()+frontBuffer);
	Filebuffer &buf=buffers[frontBuffer];
	buf.open(fname,false);
	if(doredraw)Screen::redraw();
	return buf;
}

void printStatus(string status,IO::Colour clr,bool bold){
	unsigned int scrwidth,scrheight;
	tie(scrwidth,scrheight)=IO::screensize();
	IO::gotoxy(0,scrheight-1);
	IO::switchColourFg(clr);
	IO::switchColourBg(Inter::screenbg);
	if(bold)IO::turnOnBold();
	if(status.size()>scrwidth){
		cout<<status.substr(0,scrwidth-3)<<"...";
	} else {
		cout<<status<<string(scrwidth-status.size(),' ');
	}
	if(bold)IO::clearMarkup();
	Screen::gotoFrontBufferCursor();
	cout.flush();
}

void clearStatus(void){
	unsigned int scrwidth,scrheight;
	tie(scrwidth,scrheight)=IO::screensize();
	IO::gotoxy(0,scrheight-1);
	IO::switchColourBg(Inter::screenbg);
	cout<<string(scrwidth,' ');
	Screen::gotoFrontBufferCursor();
	cout.flush();
}

string askQuestion(string question,const IO::Colour &clr){
	unsigned int scrwidth,scrheight;
	tie(scrwidth,scrheight)=IO::screensize();
	IO::gotoxy(0,scrheight-1);
	IO::switchColourFg(clr);
	IO::switchColourBg(Inter::screenbg);
	if(question.size()>scrwidth-10){
		cout<<question.substr(0,scrwidth-13)<<"...";
	} else {
		cout<<question<<string(scrwidth-question.size(),' ');
	}
	IO::gotoxy(question.size(),scrheight-1);
	cout.flush();
	IO::clearMarkup();
	string response=IO::getLineStdin(question.size());
	Screen::gotoFrontBufferCursor();
	cout.flush();
	return response;
}

void drawScreen(Screen::Screencell *screen,unsigned int width,unsigned int height){
	unsigned int acclen=0,acclensquashable=0;
	double factor=1;
	vector<string> basenames;
	const size_t nbuffers=buffers.size();
	basenames.reserve(nbuffers);
	int ndirty=0;
	for(const Filebuffer &buffer : buffers){
		basenames.push_back(basename(buffer.openpath));
		string &fname=basenames.back();
		if(fname.size()==0)basenames.back()="<>";
		acclensquashable+=fname.size();
		acclen+=(acclen>0)+fname.size()+buffer.dirty; //space, name, dirty-*
		ndirty+=buffer.dirty;
	}
	const unsigned int nnosquash=acclen-acclensquashable;
	if(acclen>width)factor=(double)(width-nnosquash)/acclensquashable;
	unsigned int x=0,y=0,linenum;
	unsigned int i,j;
	unsigned int nbuf=nbuffers;
	for(i=0;i<nbuf;i++){
		if(i!=0){
			Screen::Screencell &cell=screen[width*y+x];
			cell.ch=' ';
			cell.clr.bg=screenbg;
			cell.clr.ul=false;
			x++;
		}
		const unsigned int bnlen=(unsigned int)(basenames[i].size()*factor);
		for(j=0;j<bnlen;j++,x++){
			Screen::Screencell &cell=screen[width*y+x];
			cell.ch=basenames[i][j];
			cell.clr.fg=screenfg;
			if((int)i==frontBuffer){
				cell.clr.fg=textfg;
				cell.clr.bg=tabbg;
			} else {
				cell.clr.bg=screenbg;
			}
			cell.clr.ul=false;
		}
		if(buffers[i].dirty){
			Screen::Screencell &cell=screen[width*y+x];
			cell.ch='*';
			cell.clr.fg=textfg;
			cell.clr.bg=(int)i==frontBuffer?tabbg:screenbg;
			cell.clr.ul=false;
			x++;
		}
	}
	for(;x<width;x++){
		Screen::Screencell &cell=screen[width*y+x];
		cell.ch=' ';
		cell.clr.bg=screenbg;
	}

	y++; x=0;
	if(frontBuffer==-1){
		//addFilebuffer(false);
		Screen::fillRect(screen,width,0,y,width,height-y,{textfg,screenbg});
		return;
	}
	Filebuffer &fbuf=buffers[frontBuffer];
	const unsigned int numberlen=log10(max(fbuf.contents.numlines(),(size_t)1))+1;
	const unsigned int editx=numberlen+2;

	Screen::fillRect(screen,width,0,y,editx,height-y-1,{numberfg,editbg});
	Screen::fillRect(screen,width,editx,y,width-editx,height-y-1,{textfg,editbg});
	Screen::fillRect(screen,width,0,height-1,width,1,{textfg,screenbg});

	//preliminary adjustments to scrolly to make the process below take less time
	const unsigned int nln=fbuf.contents.numlines();
	if(fbuf.cury>=nln)fbuf.cury=nln==0?0:nln-1;
	const unsigned int llen=fbuf.contents.linelen(fbuf.cury);
	if(fbuf.curx>llen)fbuf.curx=llen;
	if(fbuf.scrolly>fbuf.cury)fbuf.scrolly=fbuf.cury;
	else if(fbuf.cury>=fbuf.scrolly+height-2)fbuf.scrolly=fbuf.cury-height+3;

	//preliminary, invalid values (file drawing won't place cursor on command bar)
	fbuf.screencurx=0;
	fbuf.screencury=height-1;

	//since we guaranteed above that cury>scrolly, we just need to progressively move down
	// the view until the cursor is visible
	unsigned int xbak=x,ybak=y;
	do {
		x=xbak; y=ybak;
		//cerr<<"curxy=("<<fbuf.curx<<','<<fbuf.cury<<") scrolly="<<fbuf.scrolly<<" do pass"<<endl;
		linenum=fbuf.scrolly;
		for(;y<height-1&&linenum<fbuf.contents.numlines();y++,linenum++){
			int n=linenum+1;
			screen[width*y].ch=' ';
			for(x=editx-2;n;x--,n/=10)screen[width*y+x].ch='0'+n%10;
			x=editx;
			const string line=fbuf.contents.line(linenum);
			const size_t linelen=line.size();
			/*if(linelen==0&&linenum==fbuf.cury){
				fbuf.screencurx=editx;
				fbuf.screencury=y;
			}*/
			i=0;
			for(i=0;i<linelen;i++){
				if(linenum==fbuf.cury&&i==fbuf.curx){
					fbuf.screencurx=x;
					fbuf.screencury=y;
				}
				const string pretty=line[i]=='\t'?string(4-(x-editx)%4,' '):Screen::prettychar(line[i]);
				size_t plen=pretty.size();
				if(x+pretty.size()>width){
					for(;x<width;x++)screen[width*y+x].ch=' ';
					y++;
					if(y>=height-1)break;
					n=linenum+1;
					for(x=0;x<editx;x++)screen[width*y+x].ch=' ';
				}
				for(j=0;j<plen;j++,x++){
					screen[width*y+x].ch=pretty[j];
					if(plen>1)screen[width*y+x].clr.fg=IO::yellow;
					else screen[width*y+x].clr.fg=textfg;
				}
			}
			if(i==linelen&&linenum==fbuf.cury&&fbuf.curx==linelen){
				if(x==width){
					x=editx;
					y++;
				}
				fbuf.screencurx=x;
				fbuf.screencury=y;
			}
			for(;x<width;x++)screen[width*y+x].ch=' ';
		}
		for(;y<height-1;y++){
			for(x=0;x<width;x++)screen[width*y+x].ch=' ';
		}
		if(fbuf.screencury==height-1){
			if(fbuf.scrolly==fbuf.cury){
				fbuf.curx=0;
				printStatus("Hm, sorry. I can't draw your cursor that far into the line...",IO::red);
			} else fbuf.scrolly++;
		}
	} while(fbuf.screencury==height-1);
	//cerr<<"stabilised at screencurxy=("<<fbuf.screencurx<<','<<fbuf.screencury<<") scrolly="<<fbuf.scrolly<<endl;
}

}
