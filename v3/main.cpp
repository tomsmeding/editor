#include <iostream>
#include <fstream>
#include <stdexcept>
#include "interface.h"
#include "textblob.h"
#include "testing.h"

using namespace std;


class StreamRelocate{
	ostream &oldstream;
	streambuf *origbuf;
	ofstream newstream;

public:
	StreamRelocate(ostream &stream,const char *fname)
			:oldstream(stream),newstream(fname){
		if(!newstream){
			throw invalid_argument(string("Cannot open file '")+fname+"'");
		}
		origbuf=oldstream.rdbuf();
		oldstream.rdbuf(newstream.rdbuf());
	}

	~StreamRelocate(){
		oldstream.flush();
		oldstream.rdbuf(origbuf);
	}
};

int main(){
	RUNTESTS(TextBlob);

	StreamRelocate cerrRelocator(cerr,"editor.log");
	cerr<<"Starting editor v3"<<endl;

	Interface::show();
}
