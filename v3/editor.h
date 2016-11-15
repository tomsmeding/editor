#pragma once

#include <vector>
#include <functional>
#include "bufferview.h"
#include "global.h"

using namespace std;


class Editor{
	vector<BufferView> views;
	i64 activeidx=0;
	const function<void(i64 &x,i64 &y,i64 &w,i64 &h)> &sizeProvider;

public:
	Editor(const function<void(i64 &x,i64 &y,i64 &w,i64 &h)> &sizeProvider);
	~Editor();
	
	bool handleKey(int key); //returns whether handled

	i64 numViews() const;
	BufferView& view(i64 index);
	const BufferView& view(i64 index) const;
	i64 activeIndex() const;

	void drawActive();

	void newView();
};
