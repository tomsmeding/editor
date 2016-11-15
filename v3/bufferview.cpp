#include <algorithm>
#include <utility>
#include <functional>
#include <cmath>
#include <termio.h>
#include "bufferview.h"
#include "textlayout.h"

using namespace std;


const i64 gutterBaseWidth=4;
const Style textStyle={.fg=9,.bg=9,.bold=false,.ul=false};
const Style gutterStyle={.fg=3,.bg=9,.bold=false,.ul=false};
const Style errorStyle={.fg=1,.bg=9,.bold=true,.ul=true};
const Style specialStyle={.fg=6,.bg=9,.bold=false,.ul=false};


BufferView::BufferView(const string &name,i64 x,i64 y,i64 w,i64 h)
		:drawx(x),drawy(y),draww(w),drawh(h),name(name){
	buffer.addCursor(0,0);
}

BufferView::~BufferView(){}

enum class FindDir{
	backward,
	found,
	forward
};

pair<int,TextLayout::Layout> findLayout(
		const Buffer &buffer,i64 width,i64 height,i64 scrolly,
		function<FindDir(const TextLayout::Layout&)> pred){
	while(true){
		TextLayout::Layout layout=TextLayout::wrap(buffer,width,height,scrolly);
		FindDir dir=pred(layout);
		switch(dir){
			case FindDir::forward:
				if(scrolly==buffer.numLines()-1){
					cerr<<"WARNING: FindDir::backward at scrolly=="<<scrolly
					    <<" (buffer.numLines()="<<buffer.numLines()<<")"<<endl;
					return {scrolly,layout};
				}
				scrolly++;
				break;

			case FindDir::backward:
				if(scrolly==0){
					cerr<<"WARNING: FindDir::backward at scrolly==0"<<endl;
					return {scrolly,layout};
				}
				scrolly--;
				break;

			case FindDir::found:
				return {scrolly,layout};
		}
	}
}

void BufferView::draw(){
	bool viewReposition=justHandledKey;
	justHandledKey=false;
	if(drawh<=0)return;

	i64 gutterWidth=(i64)floor(log10(buffer.numLines()+1))+3;
	if(gutterWidth<gutterBaseWidth)gutterWidth=gutterBaseWidth;

	if(draww<gutterWidth+1){
		setstyle(&textStyle);
		moveto(drawx,drawy);
		tputc('?');
		return;
	}

	setstyle(&gutterStyle);
	fillrect(drawx,drawy,gutterWidth,drawh,' ');
	setstyle(&textStyle);
	fillrect(drawx+gutterWidth,drawy,draww-gutterWidth,drawh,' ');


	TextLayout::Layout layout=TextLayout::wrap(buffer,draww-gutterWidth,drawh,scrolly);
	if(viewReposition){
		vector<Buffer::Cursor> cursors=buffer.getCursors();
		i64 targetx,targety;
		if(layout.lines.size()>0){
			i64 topLine=layout.lines[0].fromLineNum;
			i64 bottomLine=layout.lines.back().fromLineNum;
			i64 i;
			for(i=0;i<(i64)cursors.size()-1;i++){ //-1 because the last cursor always suffices
				if(cursors[i].y>=topLine)break;
			}
			if(i==0){
				targetx=cursors[0].x;
				targety=cursors[0].y+1;
			} else {
				if(i>=1&&topLine-cursors[i-1].y<=cursors[i].y-bottomLine){
					targetx=cursors[i-1].x;
					targety=cursors[i-1].y+1;
				} else {
					targetx=cursors[i].x;
					targety=cursors[i].y+1;
				}
			}
		} else {
			targetx=cursors.back().x;
			targety=cursors.back().y+1;
		}
		pair<int,TextLayout::Layout> found=findLayout(
			buffer,draww-gutterWidth,drawh,scrolly,[targety](const TextLayout::Layout &layout) -> FindDir {
				if(layout.lines.size()==0){
					cerr<<"Predicate: layout.lines.size() == 0"<<endl;
					return FindDir::backward;
				}
				cerr<<"Predicate: targety="<<targety<<" l.l[0].fLN = "<<layout.lines[0].fromLineNum<<endl;
				if(layout.lines[0].fromLineNum>targety)return FindDir::backward;
				if(layout.lines.back().fromLineNum<targety)return FindDir::forward;
				return FindDir::found;
			});
		scrolly=found.first;
		layout=move(found.second);
	}


	i64 curx=-1,cury=-1;
	for(i64 y=0;y<(i64)layout.lines.size();y++){
		const TextLayout::Line &line=layout.lines[y];
		if(line.lineNum!=-1){
			moveto(drawx+1,drawy+y);
			setstyle(&gutterStyle);
			tprintf("%lld",line.lineNum);
		}
		moveto(drawx+gutterWidth,drawy+y);
		for(i64 x=0;x<(i64)line.cells.size();x++){
			const TextLayout::Cell &cell=line.cells[x];
			Style thisStyle;
			if(cell.error)thisStyle=errorStyle;
			else if(cell.special)thisStyle=specialStyle;
			else thisStyle=textStyle;
			if(cell.cursor){
				if(curx==-1){
					curx=x;
					cury=y;
				} else {
					thisStyle.bg=7;
				}
			}
			setstyle(&thisStyle);
			tputc(cell.c);
		}
	}

	if(curx!=-1){
		moveto(drawx+gutterWidth+curx,drawy+cury);
	} else {
		//TODO: make this actually useful and unambiguous
		moveto(drawx+draww-1,drawy+drawh-1);
	}
}

void BufferView::draw(i64 x,i64 y,i64 w,i64 h){
	drawx=x;
	drawy=y;
	draww=w;
	drawh=h;
	draw();
}

void BufferView::setName(const string &newname){
	name=newname;
}

const string& BufferView::getName() const {
	return name;
}

bool BufferView::handleKey(int key){
	cerr<<"handleKey("<<key<<")"<<endl;
	switch(key){
		case KEY_BACKSPACE:
			if(!buffer.backspace())bel();
			break;

		case KEY_DELETE:
			if(!buffer.forwardDelete())bel();
			break;

		case KEY_LF:
		case KEY_CR:
			buffer.insertText('\n');
			break;

		case KEY_UP:    buffer.moveCursors(Buffer::Dir::up);    break;
		case KEY_RIGHT: buffer.moveCursors(Buffer::Dir::right); break;
		case KEY_DOWN:  buffer.moveCursors(Buffer::Dir::down);  break;
		case KEY_LEFT:  buffer.moveCursors(Buffer::Dir::left);  break;

		default:
			if(key>=32&&key<=126){
				buffer.insertText((char)key);
			} else {
				return false;
			}
	}
	justHandledKey=true;
	return true;
}
