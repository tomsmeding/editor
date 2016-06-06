#include <iostream>
#include <sstream>
#include <stdexcept>
#include "textblob.h"

using namespace std;

Textblob::Textblob(void){
	data.emplace_back();
}

Textblob::Textblob(const string &from){
	size_t cursor=0,idx;
	string line;
	while(true){
		idx=from.find('\n',cursor);
		if(idx==string::npos){
			if(from.size()-cursor>0)data.emplace_back(from.begin()+cursor,from.end());
			break;
		}
		data.emplace_back(from.begin()+cursor,from.begin()+idx);
		cursor=idx+1;
	}
}

void Textblob::insert(unsigned int x,unsigned int y,string s){
	if(y>=data.size()||x>data[y].size())
		throw logic_error("Invalid x or y value in Textblob::insert");
	size_t cursor=0,idx=s.find('\n');
	if(idx==string::npos){
		data[y].insert(data[y].begin()+x,s.begin(),s.end());
		return;
	}
	data.emplace(data.begin()+(y+1),data[y].begin()+x,data[y].end());
	data[y].erase(data[y].begin()+x,data[y].end());
	data[y].insert(data[y].end(),s.begin(),s.begin()+idx);
	y++;
	cursor=idx+1;
	idx=s.find('\n',cursor);
	while(idx!=string::npos){
		data.emplace(data.begin()+y,s.begin()+cursor,s.begin()+idx);
		cursor=idx+1;
		y++;
	}
	if(s.size()-cursor){
		data[y].insert(data[y].begin(),s.begin()+cursor,s.end());
	}
}
void Textblob::insert(unsigned int x,unsigned int y,char c){
	insert(x,y,string(1,c));
}
void Textblob::insertLineBefore(unsigned int y,string s){
	if(y>data.size())
		throw logic_error("Invalid y value in Textblob::insertLineAfter");
	data.emplace(data.begin()+y,s.begin(),s.end());
}
void Textblob::overwrite(unsigned int x,unsigned int y,string s){
	erase(x,y,s.size());
	insert(x,y,s);
}
void Textblob::overwrite(unsigned int x,unsigned int y,char c){
	overwrite(x,y,string(1,c));
}
void Textblob::erase(unsigned int x,unsigned int y,unsigned int n){
	if(y>=data.size()||x>data[y].size())
		throw logic_error("Invalid x or y value in Textblob::erase");
	//cerr<<"Textblob::erase("<<x<<','<<y<<','<<n<<')'<<endl;
	if(x+n<data[y].size()){
		//cerr<<"  erasing in line"<<endl;
		data[y].erase(data[y].begin()+x,data[y].begin()+(x+n));
		return;
	}
	n-=data[y].size()-x;
	//cerr<<"  n="<<n<<endl;
	data[y].erase(data[y].begin()+x,data[y].end());
	if(n==0)return;

	n--; //newline after current line will be removed later
	y++; x=0;
	//cerr<<"  n="<<n<<" y="<<y<<endl;
	while(y<data.size()&&n>data[y].size()){
		//cerr<<"  while, n="<<n<<" y="<<y<<endl;
		n-=data[y].size()+1;
		data.erase(data.begin()+y);
	}
	//cerr<<"  after, n="<<n<<" y="<<y<<endl;
	data[y-1].insert(data[y-1].end(),data[y].begin()+n,data[y].end());
	data.erase(data.begin()+y);
}
void Textblob::removeLine(unsigned int y){
	if(y>=data.size())
		throw logic_error("Invalid y value in Textblob::removeLine");
	data.erase(data.begin()+y);
	if(data.size()==0)data.emplace_back();
}

void Textblob::clear(void){
	data.clear();
	data.emplace_back();
}

size_t Textblob::linelen(unsigned int y){
	if(y>=data.size())
		throw logic_error("Invalid y value in Textblob::linelen");
	return data[y].size();
}

size_t Textblob::numlines(void){
	return data.size();
}

string Textblob::line(unsigned int y){
	if(y>=data.size())
		throw logic_error("Invalid y value in Textblob::line");
	return string(data[y].begin(),data[y].end());
}

bool Textblob::exists(unsigned int x,unsigned int y){
	return y<data.size()&&x<data[y].size();
}

char Textblob::at(unsigned int x,unsigned int y){
	if(y>=data.size()||x>=data[y].size())
		throw logic_error("Invalid x or y value in Textblob::at");
	return data[y][x];
}

string to_string(Textblob &tb){
	string res;
	size_t len=0;
	for(const deque<char> &d : tb.data)len+=d.size()+1;
	res.reserve(len);
	for(deque<char> &d : tb.data){
		d.push_back('\n');
		res.insert(res.end(),d.begin(),d.end());
		d.pop_back();
	}
	return res;
}
