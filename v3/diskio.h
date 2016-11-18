#pragma once

#include <string>
#include <stdexcept>

using namespace std;


namespace DiskIO {

	class DiskError : public runtime_error{
	public:
		DiskError(const string &what_arg);
		DiskError(const char *what_arg);
	};

	string resolveTilde(const string &path); //used by the other functions, not necessary to call

	void writeFile(string fname,const string &s);
	string readFile(string fname);

}
