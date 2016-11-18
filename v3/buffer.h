#pragma once

#include <string>
#include <vector>
#include "global.h"
#include "textblob.h"

using namespace std;


class Buffer{
public:
	class Cursor{
	public:
		i64 x,y;

		Cursor(i64 x,i64 y);
	};

	enum class Dir{
		up, right, down, left
	};

private:
	TextBlob tb;
	vector<Cursor> cursors;

	void sortUniqCursors();

public:
	Buffer() = default;
	Buffer(const Buffer&) = default;
	Buffer(Buffer&&);

	Buffer& operator=(const Buffer &other) = default;
	Buffer& operator=(Buffer &&other);

	void read(istream &is);
	void write(ostream &os) const;

	void setText(const string &text);
	string fullText() const;

	i64 numLines() const;
	i64 lineLen(i64 y) const;
	vector<Cursor> getCursors() const; //returns sorted and uniq'ed in buffer order
	string getLine(i64 y) const;
	char getChar(i64 y,i64 x) const;

	void insertText(char c);
	void insertText(const string &text);
	bool backspace(int ntimes=1); //returns whether there was anything to delete
	bool forwardDelete(int ntimes=1); //returns whether there was anything to delete

	void addCursor(i64 y,i64 x);
	void setCursor(i64 y,i64 x);
	void singleCursor(); //discards all but the first cursor
	void moveCursors(Dir dir);
};
