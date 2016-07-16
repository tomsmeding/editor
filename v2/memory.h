#pragma once

#include <stdlib.h>

#define malloc(n,t) ((t*)malloc((n)*sizeof(t)))
