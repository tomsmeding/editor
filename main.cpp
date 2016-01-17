#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <signal.h>
#include "io.h"
#include "screen.h"
#include "interface.h"
#include "script.h"

using namespace std;

bool screeninited=false;
string closemessage;
streambuf *cerrbuf;

void siginthandler(int){
	Inter::printStatus("Use :q[uit] to exit editor.");
	cout<<IO::gettput("bel")<<flush;
}

void atexitfunc(void){
	cerr.rdbuf(cerrbuf);
	if(screeninited)IO::endscreen();
	if(closemessage.size())cout<<closemessage<<endl;
}

int main(int argc,char **argv){
	signal(SIGINT,siginthandler);
	cerrbuf=cerr.rdbuf();
	ofstream logfile("editor.log");
	cerr.rdbuf(logfile.rdbuf());
	atexit(atexitfunc);
	cerr<<string(5,'\n');
	IO::initscreen();
	screeninited=true;

	Script::init();

	try {
		if(argc==1)Inter::addFilebuffer();
		else for(int i=1;i<argc;i++)Inter::addFilebufferFile(argv[i]);
		return IO::runloop();
	} catch(logic_error e){
		IO::endscreen();
		cerr.flush();
		throw e;
	}
}
