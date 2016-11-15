#pragma once

#include <iostream>
#include <deque>
#include <string>

#include "global.h"

using namespace std;


class TextBlob{
	deque<string> lines;

	void checkInRange(i64 y,i64 x) const;
	void checkInRangeP1(i64 y,i64 x) const;
	void checkInRange(i64 y) const;
	void checkInRangeP1(i64 y) const;

public:
	TextBlob() = default;
	TextBlob(const TextBlob&) = default;
	TextBlob(TextBlob&&) = default;

	TextBlob& operator=(const TextBlob &other) = default;
	TextBlob& operator=(TextBlob &&other) = default;

	bool isInRange(i64 y,i64 x) const;
	bool isInRangeP1(i64 y,i64 x) const;
	bool isInRange(i64 y) const;
	bool isInRangeP1(i64 y) const;

	void clear();

	void read(istream &is);
	void write(ostream &os) const;

	//returns original
	char replace(i64 y,i64 x,char c);

	//All inserts are BEFORE. Pass one-past-end index to append.
	void insert(i64 y,i64 x,char c);
	void insertLine(i64 y,const string &line);
	void insertString(i64 y,i64 x,const string &str);

	//functions return original
	char erase(i64 y,i64 x);
	string erase(i64 y,i64 x,i64 nchars);

	string& operator[](i64 y);
	const string& operator[](i64 y) const;

	i64 numLines() const;
	i64 lineLen(i64 y) const;

	static void _unitTest();
};
