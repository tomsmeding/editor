#include <iostream>
#include "util.h"

using namespace std;

unsigned int uniqid(void){
	static int i=0;
	return ++i;
}

string basename(string s){
	//cerr<<"Taking basename of <"<<s<<'>'<<endl;
	size_t idx=s.rfind('/');
	if(idx==string::npos)return s;
	return s.substr(idx+1);
}

string basename(string s,string suf){
	size_t idx=s.rfind('/');
	size_t start;
	if(idx==string::npos)start=0;
	else start=idx+1;
	if(suf.size()>=s.size()||s.substr(s.size()-suf.size())!=suf)
		return start==0?s:s.substr(start);
	return s.substr(start,s.size()-suf.size()-start);
}

string dirname(string s){
	size_t end=s.size()-1;
	while(end>0&&s[end]=='/')end--;
	if(end==0)return "/";
	size_t idx=s.rfind('/',end);
	if(idx==string::npos)return ".";
	return s.substr(0,idx-1);
}
