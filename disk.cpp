#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include "disk.h"

#if defined(__APPLE__)||defined(__FreeBSD__)
#include <copyfile.h>
#else
#include <sys/stat.h>
#include <sys/sendfile.h>
#endif

using namespace std;

namespace Disk {

string resolveTilde(const string &path){
	if(path.size()==0||path[0]!='~')return path;
	if(path.size()==1)return getenv("HOME");
	if(path[1]=='/')return getenv("HOME")+path.substr(1);
	size_t namelen=path.find('/',1);
	if(namelen!=string::npos)namelen--;
	struct passwd *pwd=getpwnam(path.substr(1,namelen).data());
	return pwd->pw_dir+path.substr(namelen+1);
}

//http://stackoverflow.com/a/2180157
int osCopyFile(const char *from,const char *to){
	int in,out;
	if((in=open(from,O_RDONLY))==-1)
		return -1;
	if((out=open(to,O_RDWR|O_CREAT|O_TRUNC,0666))==-1){
		close(in);
		return -1;
	}
#if defined(__APPLE__)||defined(__FreeBSD__)
	int res=fcopyfile(in,out,0,COPYFILE_ALL);
#else
	off_t ncopied=0;
	struct stat st;
	fstat(in,&st);
	int res=sendfile(out,in,&ncopied,st.st_size);
#endif

	close(in);
	close(out);
	return res;
}

Maybe<string> writeToFile(string fname,string s){
	fname=resolveTilde(fname);
	char tempfname[32];
	memcpy(tempfname,"/tmp/temp.XXXXXXXX",19);
	int fd=mkstemp(tempfname);
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
	if(rename(tempfname,fname.data())){
		if(errno!=EXDEV)
			return Maybe<string>(string("rename: ")+strerror(errno));
		//cross-device rename didn't work, fallback to copying
		if(osCopyFile(tempfname,fname.data())!=-1)
			return Maybe<string>::Nothing();
		//copying didn't work, fall back to direct writing
		ofstream outf(fname);
		if(!outf)
			return Maybe<string>("Cannot open file '"+fname+'\'');
		outf<<s;
		outf.close();
		return Maybe<string>::Nothing();
	}
	return Maybe<string>::Nothing();
}

Either<string,string> readFromFile(string fname){
	fname=resolveTilde(fname);
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
