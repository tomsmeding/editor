#pragma once

#include "maybe.h"

using namespace std;

namespace Script {

void init(void);
Maybe<string> runfile(const string &fname);
Maybe<string> runcode(const char *buf,const size_t bufsz,const string &tag="<runcode()>");
Maybe<string> runcode(const string &code);

}
