#include <fstream>
#include <cstdlib>
#include <sys/types.h>
#include <pwd.h>
#include "diskio.h"

using namespace std;


namespace DiskIO {

	DiskError::DiskError(const string &what_arg)
		:runtime_error(what_arg){}

	DiskError::DiskError(const char *what_arg)
		:runtime_error(what_arg){}


	string resolveTilde(const string &path){
		if(path.size()==0||path[0]!='~')return path;
		if(path.size()==1)return getenv("HOME");
		if(path[1]=='/')return getenv("HOME")+path.substr(1);
		size_t slashidx=path.find('/',1);
		string name=path.substr(1,slashidx==string::npos ? string::npos : slashidx-1);
		struct passwd *pwd=getpwnam(name.data());
		return pwd->pw_dir+path.substr(slashidx);
	}

	void writeFile(string fname,const string &s){
		ofstream os(fname);
		if(!os){
			throw DiskError("Could not open file '"+fname+"'");
		}
		os<<s<<flush;
		if(!os){
			throw DiskError("Could not write to file '"+fname+"'");
		}
		os.close();
		if(!os){
			throw DiskError("Error while closing file '"+fname+"'; potentially corrupted!");
		}
	}

	string readFile(string fname){
		ifstream is(fname);
		if(!is){
			throw DiskError("Could not open file '"+fname+"'");
		}
		string res;
		char buf[1024];
		while(true){
			is.read(buf,sizeof(buf));
			size_t count=is.gcount();
			if(count==0)break;
			res.append(buf,count);
			if(is.eof()){
				is.clear(); //main purpose: clear the fail bit
				break;
			} else if(!is){
				throw DiskError("Error while reading file '"+fname+"'");
			}
		}
		is.close(); //errors are not checked
		return res;
	}

}
