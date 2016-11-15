#include <algorithm>
#include <cassert>
#include "buffer.h"

using namespace std;


Buffer::Cursor::Cursor(i64 x,i64 y)
	:x(x),y(y)
	{}


Buffer::Buffer(Buffer &&other)
		:tb(move(other.tb)),cursors(move(other.cursors)){
	other.cursors.emplace_back(0,0);
}

Buffer& Buffer::operator=(Buffer &&other){
	tb=move(other.tb);
	cursors=move(other.cursors);
	other.cursors.emplace_back(0,0);
	return *this;
}

void Buffer::sortUniqCursors(){
	sort(cursors.begin(),cursors.end(),[](const Cursor &a,const Cursor &b){
		return a.y<b.y||(a.y==b.y&&a.x<b.x);
	});
	i64 ncursors=cursors.size();
	for(i64 i=1;i<ncursors;i++){
		if(cursors[i].y==cursors[i-1].y&&cursors[i].x==cursors[i-1].x){
			cursors.erase(cursors.begin()+i);
			i--;
		}
	}
}

void Buffer::read(istream &is){
	tb.read(is);
}

void Buffer::write(ostream &os) const {
	tb.write(os);
}

i64 Buffer::numLines() const {
	return tb.numLines();
}

vector<Buffer::Cursor> Buffer::getCursors() const {
	return cursors;
}

string Buffer::getLine(i64 y) const {
	return tb[y];
}

char Buffer::getChar(i64 y,i64 x) const {
	return tb[y][x];
}

void Buffer::insertText(char c){
	if(tb.numLines()==0){
		sortUniqCursors();
		assert(cursors.size()<=1);
		if(cursors.size()==0)return;
		assert(cursors[0].x==0&&cursors[0].y==0);
		tb.insert(0,0,c);
		if(c!='\n')cursors[0].x++;
		else cursors[0].y++;
		return;
	}
	if(c!='\n'){
		for(Cursor &cur : cursors){
			tb.insert(cur.y,cur.x,c);
			cur.x++;
		}
	} else {
		const i64 ncursors=cursors.size();
		for(i64 i=0;i<ncursors;i++){
			for(i64 j=0;j<ncursors;j++){
				if(j==i)continue;
				if(cursors[j].y>cursors[i].y){
					cursors[j].y++;
				} else if(cursors[j].y==cursors[i].y){
					if(cursors[j].x>=cursors[i].x){
						cursors[j].y++;
						cursors[j].x-=cursors[i].x;
					}
				}
			}
			tb.insert(cursors[i].y,cursors[i].x,'\n');
			cursors[i].y++;
			cursors[i].x=0;
		}
	}
}

void Buffer::insertText(const string &text){
	//TODO: fix this implementation to be WAY faster
	for(char c : text){
		insertText(c);
	}
}

bool Buffer::backspace(int ntimes){
	if(tb.numLines()==0)return false;;
	const i64 ncursors=cursors.size();
	bool success=true;
	while(ntimes-->0){
		for(i64 i=0;i<ncursors;i++){
			if(cursors[i].x==0){
				if(cursors[i].y==0){
					success=false;
					continue;
				}
				const i64 prevlen=tb.lineLen(cursors[i].y-1);
				for(i64 j=0;j<ncursors;j++){
					if(j==i)continue;
					if(cursors[j].y>cursors[i].y){
						cursors[j].y--;
					} else if(cursors[j].y==cursors[i].y){
						cursors[j].y--;
						cursors[j].x+=prevlen;
					}
				}
				tb.erase(cursors[i].y-1,prevlen);
				cursors[i].y--;
				cursors[i].x=prevlen;
			} else {
				for(i64 j=0;j<ncursors;j++){
					if(j==i)continue;
					if(cursors[j].y==cursors[i].y&&cursors[j].x>=cursors[i].x){
						cursors[j].x--;
					}
				}
				tb.erase(cursors[i].y,cursors[i].x-1);
				cursors[i].x--;
			}
		}
	}
	sortUniqCursors();
	return success;
}

bool Buffer::forwardDelete(int ntimes){
	if(tb.numLines()==0)return false;
	const i64 ncursors=cursors.size();
	bool success=true;
	while(ntimes-->0){
		for(i64 i=0;i<ncursors;i++){
			const i64 llen=tb.lineLen(cursors[i].y);
			if(cursors[i].x==llen){
				if(cursors[i].y==tb.numLines()-1){
					success=false;
					continue;
				}
				for(i64 j=0;j<ncursors;j++){
					if(j==i)continue;
					if(cursors[j].y==cursors[i].y+1){
						cursors[j].y--;
						cursors[j].x+=llen;
					} else if(cursors[j].y>cursors[i].y+1){
						cursors[j].y--;
					}
				}
				tb.erase(cursors[i].y,cursors[i].x);
			} else {
				for(i64 j=0;j<ncursors;j++){
					if(j==i)continue;
					if(cursors[j].y==cursors[i].y&&cursors[j].x>=cursors[i].x){
						cursors[j].x--;
					}
				}
				tb.erase(cursors[i].y,cursors[i].x);
			}
		}
	}
	sortUniqCursors();
	return success;
}

void Buffer::addCursor(i64 y,i64 x){
	if(tb.numLines()==0){
		if(x!=0||y!=0){
			throw out_of_range("Position y="+to_string(y)+" x="+to_string(x)+" not in range of empty Buffer");
		}
	} else {
		if(!tb.isInRange(y)||!tb.isInRangeP1(y,x)){
			throw out_of_range("Position y="+to_string(y)+" x="+to_string(x)+" not in range of Buffer");
		}
	}
	cursors.emplace_back(x,y);
	sortUniqCursors();
}

void Buffer::singleCursor(){
	sortUniqCursors();
	cursors.erase(cursors.begin()+1,cursors.end());
}

void Buffer::moveCursors(Dir dir){
	if(tb.numLines()==0)return;
	i64 ncursors=cursors.size();
	switch(dir){
		case Dir::up:
			for(i64 i=0;i<ncursors;i++){
				if(cursors[i].y==0)cursors[i].x=0;
				else {
					cursors[i].y--;
					if(cursors[i].x>tb.lineLen(cursors[i].y)){
						cursors[i].x=tb.lineLen(cursors[i].y);
					}
				}
			}
			break;

		case Dir::right:
			for(i64 i=0;i<ncursors;i++){
				if(cursors[i].x!=tb.lineLen(cursors[i].y)){
					cursors[i].x++;
				} else if(cursors[i].y!=tb.numLines()){
					cursors[i].x=0;
					cursors[i].y++;
				}
			}
			break;

		case Dir::down:
			for(i64 i=0;i<ncursors;i++){
				if(cursors[i].y==tb.numLines()-1)cursors[i].x=tb.lineLen(tb.numLines()-1);
				else {
					cursors[i].y++;
					if(cursors[i].x>tb.lineLen(cursors[i].y)){
						cursors[i].x=tb.lineLen(cursors[i].y);
					}
				}
			}
			break;

		case Dir::left:
			for(i64 i=0;i<ncursors;i++){
				if(cursors[i].x!=0){
					cursors[i].x--;
				} else if(cursors[i].y!=0){
					cursors[i].y--;
					cursors[i].x=tb.lineLen(cursors[i].y);
				}
			}
			break;
	}
}
