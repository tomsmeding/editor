#include <iostream>
#include <cmath>
#include "interface.h"
#include "disk.h"

using namespace std;

const IO::Colour screenbg=IO::Colour(0,0,0),
                 screenfg=IO::Colour(198,198,198),
                 tabbg=IO::Colour(38,38,38),
                 editbg=IO::Colour(38,38,38),
                 textfg=IO::Colour(255,255,255),
                 numberfg=IO::Colour(255,255,0);

namespace Inter {

vector<Filebuffer> buffers;

int frontBuffer=-1;


bool Filebuffer::cansave(void){
	return openpath.size()!=0;
}

bool Filebuffer::save(void){
	return Disk::writeToFile(openpath,to_string(contents));
}

bool Filebuffer::saveas(string fname){
	openpath=fname;
	return save();
}

bool Filebuffer::canopen(void){
	return true;
}

bool Filebuffer::open(string fname){
	openpath=fname;
	Maybe<string> mcont=Disk::readFromFile(fname);
	if(mcont.isNothing())return false;
	contents=Textblob(mcont.fromJust());
	Screen::redraw();
	return true;
}

bool Filebuffer::keypress(Key){
	Screen::redraw();
	return false; //STUB
}


Filebuffer& addfilebuffer(void){
	buffers.emplace_back();
	Screen::redraw();
	return buffers.back();
}

Filebuffer& addfilebuffer(const string &fname){
	frontBuffer=buffers.size();
	buffers.emplace_back();
	Filebuffer &buf=buffers.back();
	buf.open(fname);
	return buf;
}

void drawScreen(Screen::Screencell *screen,unsigned int width,unsigned int height){
	unsigned int tabwidth;
	if(buffers.size()*3-1>width)tabwidth=2;
	else tabwidth=UINT_MAX;
	unsigned int acclen=0;
	double factor=1;
	vector<string> basenames;
	basenames.reserve(buffers.size());
	for(const Filebuffer &buffer : buffers){
		basenames.push_back(basename(buffer.openpath));
		const string &fname=basenames.back();
		if(acclen)acclen++; //space between tabs
		acclen+=fname.size()?fname.size():1;
	}
	if(acclen>width)factor=(width-(buffers.size()-1))/acclen;
	//cerr<<"factor="<<factor<<endl;
	unsigned int x=0,y=0,linenum;
	unsigned int i,j;
	unsigned int nbuf=buffers.size();
	//for(i=0;i<nbuf;i++)cerr<<"basenames[i]="<<basenames[i]<<"  "; cerr<<endl;
	//cerr<<"textfg = "<<(int)textfg.r<<' '<<(int)textfg.g<<' '<<(int)textfg.b<<endl;
	for(i=0;i<nbuf;i++){
		if(i!=0){
			Screen::Screencell &cell=screen[width*y+x];
			cell.ch=' ';
			cell.clr.bg=screenbg;
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
				cell.clr.fg=screenfg;
				cell.clr.bg=screenbg;
			}
			cell.clr.ul=false;
		}
	}
	for(;x<width;x++){
		Screen::Screencell &cell=screen[width*y+x];
		cell.ch=' ';
		cell.clr.bg=screenbg;
	}

	y++; x=0;
	if(frontBuffer==-1){
		Screen::fillRect(screen,width,0,y,width,height-y,{textfg,screenbg});
		return;
	}
	Filebuffer &fbuf=buffers[frontBuffer];
	const unsigned int numberlen=log10(max(fbuf.contents.numlines(),(size_t)1))+1;
	const unsigned int editx=numberlen+2;
	linenum=fbuf.scrolly;
	Screen::fillRect(screen,width,0,y,editx,height-y,{numberfg,editbg});
	Screen::fillRect(screen,width,editx,y,width-editx,height-y,{textfg,editbg});
	for(;y<height&&linenum<fbuf.contents.numlines();y++,linenum++){
		int n=linenum+1;
		screen[width*y].ch=' ';
		for(x=editx-2;n;x--,n/=10)screen[width*y+x].ch='0'+n%10;
		x=editx;
		const string line=fbuf.contents.line(linenum);
		const size_t linelen=line.size();
		for(i=0;i<linelen;i++){
			const string pretty=line[i]=='\t'?string((x-editx+4)/4*4-(x-editx),' '):Screen::prettychar(line[i]);
			size_t plen=pretty.size();
			if(x+pretty.size()>=width){
				y++;
				n=linenum+1;
				screen[width*y].ch=' ';
				for(x=editx-2;n;x--,n/=10)screen[width*y+x].ch='0'+n%10;
				x=editx;
			}
			for(j=0;j<plen;j++,x++)
				screen[width*y+x].ch=pretty[j];
		}
	}
}

}
