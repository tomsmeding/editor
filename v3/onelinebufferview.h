#pragma once

#include <string>
#include "bufferview.h"
#include "buffer.h"
#include "global.h"

using namespace std;


class OnelineBufferView : public BufferView{
	Buffer buffer;
	i64 drawx,drawy,draww;
	i64 scrollx=0;
	bool justHandledKey=false;
	bool dirty=false;

public:
	OnelineBufferView() = delete;
	OnelineBufferView(i64 x,i64 y,i64 w);
	OnelineBufferView(const OnelineBufferView &other) = default;
	OnelineBufferView(OnelineBufferView &&other) = default;
	~OnelineBufferView();

	OnelineBufferView& operator=(const OnelineBufferView &other) = default;
	OnelineBufferView& operator=(OnelineBufferView &&other) = default;

	void setText(const string &text);
	string fullText() const;

	void draw(i64 x,i64 y,i64 w,i64 h);
	void draw(); //uses last coordinates

	//returns whether handled
	bool handleKey(int key);

	bool isDirty() const;
	void setClean();
};
