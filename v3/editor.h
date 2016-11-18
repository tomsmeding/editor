#pragma once

#include <vector>
#include <functional>
#include "bufferview.h"
#include "wrappingbufferview.h"
#include "global.h"

using namespace std;


class Editor{
	vector<WrappingBufferView> views;
	i64 activeidx=0;
	const function<void(i64 &x,i64 &y,i64 &w,i64 &h)> &sizeProvider;

public:
	Editor(const function<void(i64 &x,i64 &y,i64 &w,i64 &h)> &sizeProvider);
	~Editor();
	
	bool handleKey(int key); //passes on to active view; returns whether handled

	i64 numViews() const;
	WrappingBufferView& view(i64 index);
	const WrappingBufferView& view(i64 index) const;
	i64 activeIndex() const;
	void setActiveIndex(i64 index);

	void drawActive();

	void newView();
	void closeView();
};
