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

void Editor::drawActive(){
	i64 x,y,w,h;
	sizeProvider(x,y,w,h);
	views[activeidx].draw(x,y,w,h);
}

void Editor::newView(){
	i64 x,y,w,h;
	sizeProvider(x,y,w,h);
	views.emplace(views.begin()+activeidx,"<New file>",x,y,w,h);
	if(activeidx!=(i64)views.size()-1)activeidx++;
}
