#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include "disk.h"

using namespace std;

namespace Disk {

Maybe<string> writeToFile(string fname,string s){
	char templ[32];
	memcpy(templ,"/tmp/temp.XXXXXXXX",19);
	int fd=mkstemp(templ);
	if(fd<0)
		return Maybe<string>(string("mkstemp: ")+strerror(errno));
	size_t cursor=0,written;
	do {
		written=write(fd,s.data()+cursor,s.size()-cursor);
		if(written==0){
			const Maybe<string> mval=Maybe<string>(string("write: ")+strerror(errno));
			close(fd);
			return mval;
		}
		cursor+=written;
	} while(cursor<s.size());
	if(close(fd))
		return Maybe<string>(string("close: ")+strerror(errno));
	if(rename(templ,fname.data()))
		return Maybe<string>(string("rename: ")+strerror(errno));
	return Maybe<string>::Nothing();
}

Either<string,string> readFromFile(string fname){
	ifstream f(fname,ios_base::in|ios_base::binary|ios_base::ate);
	if(!f)return Either<string,string>::Left(strerror(errno));
	size_t sz=f.tellg();
	f.seekg(0,ios_base::beg);
	string res;
	res.reserve(sz+1); //space for possible newline
	res.resize(sz);
	f.read(&*res.begin(),sz);
	if(res[sz-1]!='\n')res+='\n';
	return Either<string,string>::Right(res);
}

} //namespace Disk
