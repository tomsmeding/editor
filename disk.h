#pragma once

#include <string>
#include "maybe.h"

using namespace std;

namespace Disk {

bool writeToFile(string fname,string s);
Maybe<string> readFromFile(string fname);

} //namespace Disk
