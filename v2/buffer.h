#pragma once

#include <stdbool.h>

#include "string.h"

struct Buffer;
typedef struct Buffer Buffer;

typedef enum Direction{UP,RIGHT,DOWN,LEFT} Direction;

Buffer* b_make(void);
void b_destroy(Buffer *b);
int b_nlines(const Buffer *b);
String* b_linerepr(const Buffer *b,int line);
void b_draw(const Buffer *b,int atx,int aty,int w,int h);

bool b_cursor_move(Buffer *b,Direction dir);
void b_insert(Buffer *b,const String *text);
