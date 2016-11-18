#pragma once

#include <vector>
#include <string>
#include "buffer.h"
#include "global.h"

using namespace std;


namespace TextLayout{

	struct Cell{
		char c;
		bool error,special,cursor;
		i64 fromX; //position in line the cell is a part of
	};

	struct Line{
		i64 lineNum; //-1 if no line number here
		i64 fromLineNum; //number of line this is a part of
		vector<Cell> cells;
	};

	struct Layout{
		vector<Line> lines;
	};


	Layout wrap(const Buffer &buffer,i64 width,i64 height,i64 scrolly);
	Layout oneline(const Buffer &buffer,i64 width,i64 scrollx);

}
