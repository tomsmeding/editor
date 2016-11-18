#include <algorithm>
#include <utility>
#include <functional>
#include <cmath>
#include <termio.h>
#include "onelinebufferview.h"
#include "textlayout.h"

using namespace std;


const Style textStyle={.fg=9,.bg=9,.bold=false,.ul=false};
const Style errorStyle={.fg=1,.bg=9,.bold=true,.ul=true};
const Style specialStyle={.fg=6,.bg=9,.bold=false,.ul=false};


OnelineBufferView::OnelineBufferView(i64 x,i64 y,i64 w)
		:drawx(x),drawy(y),draww(w){
	buffer.addCursor(0,0);
}

OnelineBufferView::~OnelineBufferView(){}

void OnelineBufferView::setText(const string &text){
	buffer.setText(text);
	buffer.setCursor(0,buffer.lineLen(0));
}

string OnelineBufferView::fullText() const {
	string text=buffer.fullText();
	text.erase(text.begin()+(text.size()-1)); //remove the newline
	return text;
}

enum class FindDir{
	backward,
	found,
	forward
};

static pair<int,TextLayout::Layout> findLayout(
		const Buffer &buffer,i64 width,i64 scrollx,
		function<FindDir(const TextLayout::Layout&)> pred){
	const string &line=buffer.getLine(0);
	while(true){
		TextLayout::Layout layout=TextLayout::oneline(buffer,width,scrollx);
		FindDir dir=pred(layout);
		switch(dir){
			case FindDir::forward:
				if(scrollx==(i64)line.size()-1){
					cerr<<"[OL] WARNING: FindDir::forward at scrollx=="<<scrollx
					    <<" (line.size()="<<line.size()<<")"<<endl;
					return {scrollx,layout};
				}
				scrollx++;
				break;

			case FindDir::backward:
				if(scrollx==0){
					cerr<<"[OL] WARNING: FindDir::backward at scrollx==0"<<endl;
					return {scrollx,layout};
				}
				scrollx--;
				break;

			case FindDir::found:
				return {scrollx,layout};
		}
	}
}

void OnelineBufferView::draw(){
	bool viewReposition=justHandledKey;
	justHandledKey=false;

	setstyle(&textStyle);
	fillrect(drawx,drawy,draww,1,' ');

	TextLayout::Layout layout=TextLayout::wrap(buffer,draww,1,0);
	if(viewReposition){
		vector<Buffer::Cursor> cursors=buffer.getCursors();
		i64 targetx;

		i64 i;
		for(i=0;i<(i64)cursors.size()-1;i++){ //-1 because the last cursor always suffices
			if(cursors[i].x>=scrollx)break;
		}
		if(i>=1&&scrollx-cursors[i-1].x<=cursors[i].x-layout.lines[0].cells.back().fromX){
			targetx=cursors[i-1].x;
		} else {
			targetx=cursors[i].x;
		}

		pair<int,TextLayout::Layout> found=findLayout(
			buffer,draww,scrollx,[targetx](const TextLayout::Layout &layout) -> FindDir {
				if(layout.lines[0].cells.size()==0){
					cerr<<"[OL] Predicate: layout.lines[0].cells.size() == 0"<<endl;
					return FindDir::backward;
				}
				cerr<<"[OL] Predicate: targetx="<<targetx
				    <<" l.l[0].c[0].fX = "<<layout.lines[0].cells[0].fromX
				    <<" l.l[0].c[$].fX = "<<layout.lines[0].cells.back().fromX<<endl;
				if(layout.lines[0].cells[0].fromX>targetx)return FindDir::backward;
				if(layout.lines[0].cells.back().fromX<targetx)return FindDir::forward;
				return FindDir::found;
			});
		scrollx=found.first;
		layout=move(found.second);
	}

	i64 curx=-1;
	const vector<TextLayout::Cell> &cells=layout.lines[0].cells;

	moveto(drawx,drawy);

	for(i64 x=0;x<(i64)cells.size();x++){
		const TextLayout::Cell &cell=cells[x];
		Style thisStyle;
		if(cell.error)thisStyle=errorStyle;
		else if(cell.special)thisStyle=specialStyle;
		else thisStyle=textStyle;
		if(cell.cursor){
			if(curx==-1){
				curx=x;
			} else {
				thisStyle.bg=7;
			}
		}
		setstyle(&thisStyle);
		tputc(cell.c);
	}

	if(curx!=-1){
		moveto(drawx+curx,drawy);
	} else {
		//TODO: make this actually useful and unambiguous
		moveto(drawx+draww-1,drawy);
	}
}

void OnelineBufferView::draw(i64 x,i64 y,i64 w,i64 h){
	if(h!=1)throw invalid_argument("OnelineBufferView should have height 1");
	drawx=x;
	drawy=y;
	draww=w;
	draw();
}

bool OnelineBufferView::handleKey(int key){
	switch(key){
		case KEY_BACKSPACE:
			if(!buffer.backspace())bel();
			break;

		case KEY_DELETE:
			if(!buffer.forwardDelete())bel();
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
