#include <iostream>
#include <chrono>
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

string trim(string s){
	unsigned int start=0,end=s.size()-1;
	while(start<=end&&s[start]==' ')start++;
	while(end>=start&&s[end]==' ')end--;
	return s.substr(start,end-start+1);
}

bool startswith(string s,string prefix,size_t minlen){
	if(prefix.size()<minlen)return false;
	if(s.size()==prefix.size())return s==prefix;
	return s.substr(0,prefix.size())==prefix;
}

vector<string> split(string s,char sep){
	vector<string> res;
	size_t cursor=0,idx=s.find(sep);
	while(idx!=string::npos){
		res.push_back(s.substr(cursor,idx-cursor));
		cursor=idx+1;
		idx=s.find(sep,cursor);
	}
	res.push_back(s.substr(cursor));
	return res;
}

vector<string> splitSmart(string s,char sep){
	vector<string> res;
	size_t cursor=0,idx,len=s.size();
	while(cursor<len&&s[cursor]==sep)cursor++;
	if(cursor==len)return res;
	idx=s.find(sep,cursor);
	while(idx!=string::npos){
		res.push_back(s.substr(cursor,idx-cursor));
		cursor=idx+1;
		while(cursor<len&&s[cursor]==sep)cursor++;
		if(cursor==len)return res;
		idx=s.find(sep,cursor);
	}
	res.push_back(s.substr(cursor));
	return res;
}

string join(vector<string> v,char sep){
	if(!v.size())return "";
	string res;
	size_t reslen=0;
	for(const string &s : v)reslen+=s.size()+1;
	reslen--;
	res.reserve(reslen);
	size_t nelem=v.size();
	for(size_t i=0;i<nelem;i++){
		if(i>0)res+=sep;
		res+=v[i];
	}
	return res;
}

/*template <class InputIt>
string join(InputIt begin,InputIt end,char sep){
	string res;
	for(size_t i=0;begin!=end;begin++){
		if(i>0)res+=sep;
		res+=*begin;
	}
	return res;
}*/


chrono::time_point<chrono::steady_clock> programstart=chrono::steady_clock::now();

unsigned long long int msclock(void){
	return chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now()-programstart).count();
}
