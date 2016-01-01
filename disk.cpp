#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include "disk.h"

using namespace std;

namespace Disk {

bool writeToFile(string fname,string s){
	char templ[32];
	memcpy(templ,"/tmp/temp.XXXXXXXX",19);
	int fd=mkstemp(templ);
	if(fd<0){
		perror("(writeToFile) mkstemp");
		return false;
	}
	size_t cursor=0,written;
	do {
		written=write(fd,s.data()+cursor,s.size()-cursor);
		if(written==0){
			perror("(writeToFile) write");
			close(fd);
			return false;
		}
		cursor+=written;
	} while(cursor<s.size());
	if(close(fd)){
		perror("(writeToFile) close");
		return false;
	}
	if(rename(templ,fname.data())){
		perror("(writeToFile) rename");
		return false;
	}
	return true;
}

Maybe<string> readFromFile(string fname){
	ifstream f(fname,ios_base::in|ios_base::binary|ios_base::ate);
	if(!f)return Maybe<string>::Nothing();
	size_t sz=f.tellg();
	f.seekg(0,ios_base::beg);
	string res;
	res.reserve(sz+1); //space for possible newline
	res.resize(sz);
	f.read(&*res.begin(),sz);
	if(res[sz-1]!='\n')res+='\0';
	return Maybe<string>(res);
}

} //namespace Disk
