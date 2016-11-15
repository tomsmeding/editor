#include "testing.h"

using namespace std;

TestException::TestException(const string &what_arg)
	:logic_error(what_arg){}

TestException::TestException(const char *what_arg)
	:logic_error(what_arg){}
