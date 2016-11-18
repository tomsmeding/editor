#pragma once

#include <string>
#include "buffer.h"
#include "bufferview.h"
#include "global.h"

using namespace std;


class WrappingBufferView : public BufferView{
	Buffer buffer;
	i64 drawx,drawy,draww,drawh;
	string name;
	i64 scrolly=0;
	bool justHandledKey=false;

public:
	WrappingBufferView(const string &name,i64 x,i64 y,i64 w,i64 h);
	WrappingBufferView(const WrappingBufferView &other) = default;
	WrappingBufferView(WrappingBufferView &&other) = default;
	~WrappingBufferView();

	WrappingBufferView& operator=(const WrappingBufferView &other) = default;
	WrappingBufferView& operator=(WrappingBufferView &&other) = default;

	void setText(const string &text);
	string fullText() const;

	void draw(i64 x,i64 y,i64 w,i64 h);
	void draw(); //uses last coordinates

	void setName(const string &newname);
	const string& getName() const;

	//returns whether handled
	bool handleKey(int key);
};
