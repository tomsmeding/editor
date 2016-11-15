#pragma once

#include <string>
#include "buffer.h"
#include "global.h"

using namespace std;


class BufferView{
	Buffer buffer;
	i64 drawx,drawy,draww,drawh;
	string name;
	i64 scrolly=0;
	bool justHandledKey=false;

public:
	BufferView(const string &name,i64 x,i64 y,i64 w,i64 h);
	BufferView(const BufferView &other) = default;
	BufferView(BufferView &&other) = default;
	~BufferView();

	BufferView& operator=(const BufferView &other) = default;
	BufferView& operator=(BufferView &&other) = default;

	void draw(i64 x,i64 y,i64 w,i64 h);
	void draw(); //uses last coordinates

	void setName(const string &newname);
	const string& getName() const;

	//returns whether handled
	bool handleKey(int key);
};
