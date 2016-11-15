#pragma once

#include <iostream>
#include <stdexcept>
#include <cstdlib>

using namespace std;

class TestException : public logic_error{
public:
	TestException(const string &what_arg);
	TestException(const char *what_arg);
};


#define _TESTING_TO_STR_(x) #x
#define _TESTING_TO_STR(x) _TESTING_TO_STR_(x)

#define _TESTING_PREFIX "(" __FILE__ ":" _TESTING_TO_STR(__LINE__) ") "


#define ASSERT(cond) \
	do { \
		if(!(cond)){ \
			throw TestException(_TESTING_PREFIX "ASSERT failed: " _TESTING_TO_STR(cond)); \
		} \
	} while(0)

#define ASSERTEQ(a,b) \
	do { \
		const auto &av=(a); \
		const auto &bv=(b); \
		if(av!=bv){ \
			ostringstream osa,osb; \
			osa<<av; \
			osb<<bv; \
			throw TestException(_TESTING_PREFIX "ASSERTEQ failed: <" + osa.str() + "> != <" + osb.str() + ">"); \
		} \
	} while(0)

#define ASSERTTHROW(expr,type) \
	do { \
		try { \
			expr; \
		} catch(type){ \
			break; \
		} catch(...){ \
			try { \
				rethrow_exception(current_exception()); \
			} catch(const exception &e){ \
				cerr<<e.what()<<endl; \
			} catch(...){ \
				cerr<<"Unknown exception caught"<<endl; \
			} \
			throw TestException(_TESTING_PREFIX "ASSERTTHROW failed: wrong exception type thrown"); \
		} \
		throw TestException(_TESTING_PREFIX "ASSERTTHROW failed: no exception thrown"); \
	} while(0)


#define RUNTESTS(classname) \
	do { \
		try { \
			classname::_unitTest(); \
		} catch(const TestException &e) { \
			cerr<<"\x1B[0;1;31mTEST ERROR: \x1B[0;1m"<<e.what()<<"\x1B[0m"<<endl; \
			exit(1); \
		} \
	} while(0)
