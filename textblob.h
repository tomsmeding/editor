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
	void insertLineBefore(unsigned int y,string s);
	void overwrite(unsigned int x,unsigned int y,string s);
	void erase(unsigned int x,unsigned int y,unsigned int n);

	size_t linelen(unsigned int y);
	size_t numlines(void);
	string line(unsigned int y);
	char at(unsigned int x,unsigned int y);

	/*string pretty(unsigned int fromy=0);
	string prettyline(unsigned int y);
	string prettychar(unsigned int x,unsigned int y);

	static string prettychar(char c);*/

	friend string to_string(Textblob &tb);
};
