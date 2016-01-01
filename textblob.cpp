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
	if(y>=data.size()||x>=data[y].size())throw logic_error("Invalid x or y value in Textblob::insert");
	size_t cursor=0,idx=s.find('\n');
	if(idx==string::npos){
		data[y].insert(data[y].begin()+x,s.begin(),s.end());
		return;
	}
	do {
		data.emplace_back(s.begin()+cursor,s.begin()+idx);
		cursor=idx+1;
		idx=s.find('\n',cursor);
	} while(idx!=string::npos);
	if(s.size()-cursor){
		data.emplace_back(s.begin()+cursor,s.end());
	}
}
void Textblob::overwrite(unsigned int x,unsigned int y,string s){
	erase(x,y,s.size());
	insert(x,y,s);
}
void Textblob::erase(unsigned int x,unsigned int y,unsigned int n){
	if(y>=data.size()||x>=data[y].size())throw logic_error("Invalid x or y value in Textblob::erase");
	if(x+n<data[y].size()){
		data[y].erase(data[y].begin()+x,data[y].begin()+(x+n));
		return;
	}
	n-=data.size()-x;
	data[y].erase(data[y].begin()+x,data[y].end());
	if(n==0)return;

	y++; x=0; n--;
	while(y<data.size()&&n>data[y].size()){
		n-=data[y].size()+1;
		data.erase(data.begin()+y);
	}
	data[y-1].insert(data[y-1].end(),data[y].begin()+n,data[y].end());
	data.erase(data.begin()+y);
}

size_t Textblob::linelen(unsigned int y){
	return data[y].size();
}

size_t Textblob::numlines(void){
	return data.size();
}

string Textblob::line(unsigned int y){
	return string(data[y].begin(),data[y].end());
}

char Textblob::at(unsigned int x,unsigned int y){
	return data[y][x];
}

/*string Textblob::pretty(unsigned int fromy){
	stringstream ss;
	size_t y,len=data.size();
	for(y=fromy;y<len;y++)ss<<prettyline(y);
	return ss.str();
}

string Textblob::prettyline(unsigned int y){
	stringstream ss;
	for(const char c : data[y])ss<<prettychar(c);
	return ss.str();
}

string Textblob::prettychar(unsigned int x,unsigned int y){
	if(y>=data.size()||x>=data[y].size())throw logic_error("Invalid x or y value in Textblob::prettychar");
	return prettychar(data[y][x]);
}

string Textblob::prettychar(char c){
	if(c==0)return "\\0";
	if(c==9)return "    ";
	if(c==10)return "\n";
	if(c<28)return string(1,'^')+(char)('A'+c-1);
	if(c<32)return string("\\x")+(char)dec2hexChar(c/16)+(char)dec2hexChar(c%16);
	return string(1,c);
}*/

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