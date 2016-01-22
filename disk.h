#pragma once

#include <string>
#include "maybe.h"
#include "either.h"

using namespace std;

namespace Disk {

string canonicalisePath(const string &path);
Maybe<string> writeToFile(string fname,const string &s); //maybe returns an error string
Either<string,string> readFromFile(string fname);

} //namespace Disk
