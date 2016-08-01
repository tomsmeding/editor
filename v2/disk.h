#pragma once

#include <stdbool.h>

#include "string.h"

String* disk_read(const char *fname);
bool disk_write(const char *fname,const String *s);
