#pragma once

struct Textbuf;
typedef struct Textbuf Textbuf;

Textbuf* textbuf_make(void);
void textbuf_destroy(Textbuf *tb);
void textbuf_insert(Textbuf *tb,int line,int col,const String *s);
void textbuf_delete(Textbuf *tb,int line1,int col1,int line2,int col2);
int textbuf_nlines(const Textbuf *tb);
int textbuf_linelen(const Textbuf *tb,int line);
char textbuf_at(const Textbuf *tb,int line,int col);
