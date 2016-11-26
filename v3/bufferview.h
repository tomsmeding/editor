#pragma once

#include <string>
#include "global.h"

using namespace std;


class BufferView{
public:
	BufferView();
	BufferView(const BufferView&) = default;
	BufferView(BufferView&&) = default;
	virtual ~BufferView() = 0;

	BufferView& operator=(const BufferView &other) = default;
	BufferView& operator=(BufferView &&other) = default;

	void setText(const string &text);
	string fullText() const;

	void draw(i64 x,i64 y,i64 w,i64 h);
	void draw(); //uses last coordinates

	//returns whether handled
	bool handleKey(int key);

	bool isDirty() const;
	void setClean();
};
