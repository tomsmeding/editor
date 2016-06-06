#pragma once

#include <vector>
#include <deque>
#include <string>

using namespace std;

class Textblob{
	vector<deque<char>> data; //vector of lines

public:
	Textblob(void);
	explicit Textblob(const string &from);

	void insert(unsigned int x,unsigned int y,string s);
	void insert(unsigned int x,unsigned int y,char s);
	void insertLineBefore(unsigned int y,string s);
	void overwrite(unsigned int x,unsigned int y,string s);
	void overwrite(unsigned int x,unsigned int y,char c);
	void erase(unsigned int x,unsigned int y,unsigned int n);
	void removeLine(unsigned int y);

	void clear(void);

	size_t linelen(unsigned int y);
	size_t numlines(void);
	string line(unsigned int y);
	bool exists(unsigned int x,unsigned int y);
	char at(unsigned int x,unsigned int y);

	friend string to_string(Textblob &tb);
};
