#pragma once

struct String;
typedef struct String String;

String* string_empty(int cap);
String* string_fromcstr(const char *cs);
String* string_frombuf(const char *buf,int len);
void string_destroy(String *s);
void string_append_c(String *s,char c);
void string_append_cs(String *s,const char *cs);
void string_append_b(String *s,const char *buf,int len);
void string_append_s(String *s,const String *s2);
void string_insert_b(String *s,int pos,const char *buf,int len);
void string_insert_s(String *s,int pos,const String *s2);
void string_erase(String *s,int start,int len); //len==-1 means till end of string
String* string_substr(const String *s,int start,int len); //len-1 means till end of string
char string_at(const String *s,int idx);
int string_length(const String *s);

const char* string_data(const String *s); //does not null-terminate
