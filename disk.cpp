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
#include "interface.h"

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

Maybe<string> writeToFile(string fname,const string &s){
	fname=resolveTilde(fname);
	const char *data=s.data();
	const size_t slen=s.size();
	size_t cursor;
	int written;
	int fd;

	size_t lastslash=fname.rfind('/');
	string tempfname=lastslash==string::npos?
	                 ".editor-"+fname+'~':
	                 fname.substr(0,lastslash+1)+".editor-"+fname.substr(lastslash+1)+'~';
	if(unlink(tempfname.data())==-1&&errno!=ENOENT)
		goto writeToFileSimple;
	fd=open(tempfname.data(),O_WRONLY|O_CREAT|O_EXCL,0666);
	if(fd==-1)
		goto writeToFileSimple;
	cursor=0;
	do {
		written=write(fd,data+cursor,slen-cursor);
		if(written<1){
			close(fd);
			unlink(tempfname.data());
			goto writeToFileSimple;
		}
		cursor+=written;
	} while(cursor<slen);
	if(close(fd)){
		unlink(tempfname.data());
		goto writeToFileSimple;
	}
	if(rename(tempfname.data(),fname.data())){
		unlink(tempfname.data());
		goto writeToFileSimple;
	}
	return Maybe<string>::Nothing();

writeToFileSimple:
	string response=Inter::askQuestion("Cannot write atomically; write directly? (unsafe!) [y/N] ",IO::red);
	if(response.size()==0||(response[0]!='y'&&response[0]!='Y'))
		return Maybe<string>("(canceled)");
	fd=open(fname.data(),O_WRONLY|O_CREAT,0666);
	if(fd==-1)
		return Maybe<string>("Cannot open file '"+fname+'\'');
	cursor=0;
	do {
		written=write(fd,data+cursor,slen-cursor);
		if(written==-1){
			const Maybe<string> mval(string("Error while writing file! ")+strerror(errno));
			close(fd);
			return mval;
		} else if(written==0){
			close(fd);
			return Maybe<string>("Error while writing file; disk full?");
		}
		cursor+=written;
	} while(cursor<slen);
	if(close(fd))
		return Maybe<string>(string("Could not close file: ")+strerror(errno));
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
