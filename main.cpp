#include <iostream>
#include <cstdlib>
#include "io.h"
#include "screen.h"
#include "interface.h"

using namespace std;

int main(int argc,char **argv){
	IO::initscreen();
	atexit(IO::endscreen);
	int i;
	for(i=1;i<argc;i++)Inter::addfilebuffer(argv[i]);
	cin.get();
}
