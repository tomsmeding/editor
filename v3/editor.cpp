#include <stdexcept>
#include "editor.h"

using namespace std;


Editor::Editor(const function<void(i64 &x,i64 &y,i64 &w,i64 &h)> &sizeProvider)
	:sizeProvider(sizeProvider){
	newView();
}

Editor::~Editor(){}

bool Editor::handleKey(int key){
	return views[activeidx].handleKey(key);
}

i64 Editor::numViews() const {
	return views.size();
}

BufferView& Editor::view(i64 index){
	return views[index];
}

const BufferView& Editor::view(i64 index) const {
	return views[index];
}

i64 Editor::activeIndex() const {
	return activeidx;
}

void Editor::setActiveIndex(i64 index){
	if(index<0||index>=(i64)views.size()){
		throw out_of_range("Index out of range in Editor::setActiveIndex");
	}
	activeidx=index;
}

void Editor::drawActive(){
	i64 x,y,w,h;
	sizeProvider(x,y,w,h);
	views[activeidx].draw(x,y,w,h);
}

void Editor::newView(){
	i64 x,y,w,h;
	sizeProvider(x,y,w,h);
	if(views.size()!=0)activeidx++;
	views.emplace(views.begin()+activeidx,"<New file>",x,y,w,h);
}

void Editor::closeView(){
	views.erase(views.begin()+activeidx);
	if(views.size()==0)newView();
	else if(activeidx==(i64)views.size())activeidx--;
}
