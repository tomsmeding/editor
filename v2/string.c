#include <string.h>
#include <assert.h>

#include "memory.h"
#include "string.h"
#include "util.h"

#define FIX_RANGE(s,start,len,emptycode) \
	assert(start>=0&&start<s->len); \
	if(len<=0){emptycode} \
	if(len==-1||start+len>s->len)len=s->len-start;

struct String{
	char *buf;
	int sz,len;
};

String* string_empty(int cap){
	if(cap<=0)cap=32;
	String *s=malloc(1,String);
	if(!s)outofmem();
	s->buf=malloc(cap,char);
	if(!s->buf)outofmem();
	s->sz=cap;
	s->len=0;
	return s;
}

String* string_fromcstr(const char *cs){
	assert(cs);
	return string_frombuf(cs,strlen(cs));
}

String* string_frombuf(const char *buf,int len){
	assert(buf);
	String *s=malloc(1,String);
	if(!s)outofmem();
	s->buf=malloc(len,char);
	if(!s->buf)outofmem();
	memcpy(s->buf,buf,len);
	s->sz=s->len=len;
	return s;
}

void string_destroy(String *s){
	assert(s);
	free(s->buf);
	free(s);
}

void string_append_c(String *s,char c){
	assert(s);
	if(s->len==s->sz){
		s->sz+=16;
		s->buf=realloc(s->buf,s->sz);
		if(!s->buf)outofmem();
	}
	s->buf[s->len++]=c;
}

void string_append_cs(String *s,const char *cs){
	assert(s);
	assert(cs);
	string_append_b(s,cs,strlen(cs));
}

void string_append_b(String *s,const char *buf,int len){
	assert(s);
	assert(buf);
	if(len<=0)return;
	if(s->len+len>s->sz){
		s->sz=s->len+len+16; //if you're appending, you probably want some extra space
		s->buf=realloc(s->buf,s->sz);
		if(!s->buf)outofmem();
	}
	while(len-->0)s->buf[s->len++]=*buf++;
}

void string_append_s(String *s,const String *s2){
	assert(s);
	assert(s2);
	string_append_b(s,s2->buf,s2->len);
}

void string_insert_b(String *s,int pos,const char *buf,int len){
	assert(s);
	assert(buf);
	assert(pos>=0&&pos<=s->len);
	if(pos==s->len){
		string_append_b(s,buf,len);
		return;
	}
	if(s->len+len>s->sz){
		s->sz=s->len+len+16;
		s->buf=realloc(s->buf,s->sz);
		if(!s->buf)outofmem();
	}
	memmove(s->buf+pos+len,s->buf+pos,len);
	memcpy(s->buf+pos,buf,len);
	s->len+=len;
}

void string_insert_s(String *s,int pos,const String *s2){
	assert(s2);
	string_insert_b(s,pos,s2->buf,s2->len);
}

void string_erase(String *s,int start,int len){
	assert(s);
	FIX_RANGE(s,start,len,return;)
	memmove(s->buf+start,s->buf+start+len,s->len-(start+len));
	s->len-=len;
}

String* string_substr(const String *s,int start,int len){
	assert(s);
	FIX_RANGE(s,start,len,return string_empty(32);)
	String *s2=string_empty(len);
	memcpy(s2->buf,s->buf+start,len);
	s2->len=len;
	return s2;
}

char string_at(const String *s,int idx){
	assert(idx>=0&&idx<s->len);
	return s->buf[idx];
}

int string_length(const String *s){
	assert(s);
	return s->len;
}

const char* string_data(const String *s){
	assert(s);
	return s->buf;
}
