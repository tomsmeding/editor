#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <signal.h>
#include "io.h"
#include "screen.h"
#include "interface.h"

using namespace std;

bool screeninited=false;
string closemessage;

void siginthandler(int){
	closemessage="Killed by ^C";
	exit(130);
}

void atexitfunc(void){
	if(screeninited)IO::endscreen();
	if(closemessage.size())cout<<closemessage<<endl;
}

int main(int argc,char **argv){
	signal(SIGINT,siginthandler);
	atexit(atexitfunc);
	ofstream logfile("editor.log");
	cerr.rdbuf(logfile.rdbuf());
	cerr<<string(5,'\n');
	IO::initscreen();
	screeninited=true;
	int i;
	try {
		for(i=1;i<argc;i++)Inter::addfilebufferfile(argv[i]);
		return IO::runloop();
	} catch(logic_error e){
		IO::endscreen();
		cerr.flush();
		throw e;
	}
}
