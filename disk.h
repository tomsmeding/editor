#pragma once

#include <string>
#include "maybe.h"

using namespace std;

namespace Disk {

Maybe<string> writeToFile(string fname,string s); //maybe returns an error string
Maybe<string> readFromFile(string fname);

} //namespace Disk
