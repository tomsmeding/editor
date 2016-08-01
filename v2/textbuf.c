#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "memory.h"
#include "string.h"
#include "textbuf.h"
#include "util.h"

struct Textbuf{
	String **lines;
	int sz,len;
};

Textbuf* textbuf_make(void){
	Textbuf *tb=malloc(1,Textbuf);
	if(!tb)outofmem();
	tb->sz=16;
	tb->len=0;
	tb->lines=malloc(tb->sz,String*);
	for(int i=0;i<tb->sz;i++)tb->lines[i]=NULL;
	return tb;
}

void textbuf_destroy(Textbuf *tb){
	assert(tb);
	fprintf(stderr,"tb %p: nlines = %d sz = %d lines = %p\n",tb,tb->len,tb->sz,tb->lines);
	for(int i=0;i<tb->len;i++){
		fprintf(stderr,"Destroying line %i of tb %p\n",i,tb);
		string_destroy(tb->lines[i]);
	}
	free(tb->lines);
	free(tb);
}

void textbuf_insert(Textbuf *tb,int line,int col,const String *s){
	assert(tb);
	if(tb->len==0){
		assert(line==0&&col==0);
	} else {
		assert(line>=0&&line<tb->len);
		assert(col>=0&&col<=string_length(tb->lines[line]));
	}
	int len=string_length(s);
	const char *sd=string_data(s);
	int i,nlf=0;
	for(i=0;i<len;i++)nlf+=sd[i]=='\n';
	fprintf(stderr,"tb->len = %d\n",tb->len);
	if(tb->len==0){
		if(nlf+1>tb->sz){
			tb->sz=nlf+1;
			tb->lines=realloc(tb->lines,tb->sz*sizeof(String*));
			if(!tb->lines)outofmem();
		}
		const char *start=sd;
		const char *end;
		i=0;
		for(end=start;end-start<len;end++){
			if(*end=='\n'){
				tb->lines[i++]=string_frombuf(start,end-start);
				start=end+1;
			}
		}
		if(end>start)tb->lines[i++]=string_frombuf(start,end-start);
		tb->len=nlf+1;
		return;
	}
	if(nlf==0){
		string_insert_s(tb->lines[line],col,s);
		return;
	}
	String *after=string_substr(tb->lines[line],col,-1);
	string_erase(tb->lines[line],col,-1);
	const char *sstarts[nlf];
	int slens[nlf];
	int j=0;
	for(i=0;i<len;i++){
		if(sd[i]!='\n')continue;
		sstarts[j]=sd+i+1;
		if(j>0)slens[j-1]=i-(sstarts[j-1]-sd);
		j++;
	}
	slens[nlf-1]=len-(sstarts[nlf-1]-sd);

	string_append_b(tb->lines[line],sd,sstarts[0]-sd); //add the part before the first newline

	if(tb->len+nlf>tb->sz){
		tb->sz=tb->len+nlf+16;
		tb->lines=realloc(tb->lines,tb->sz*sizeof(String*));
	}
	memmove(tb->lines+line+nlf,tb->lines+line,nlf*sizeof(String*));
	tb->len+=nlf;

	//first part of s has been appended to the given line, and the trailing part of the line stored in after.
	//room has been made for the other lines of s.

	for(j=0;j<nlf;j++){
		line++;
		tb->lines[line]=string_frombuf(sstarts[j],slens[j]);
	}
	string_append_s(tb->lines[line],after);
	string_destroy(after);
}

void textbuf_delete(Textbuf *tb,int line1,int col1,int line2,int col2){
	assert(line1>=0&&line1<tb->len);
	assert(line2>=0&&line2<tb->len);
	assert(line2>=line1);
	assert(col1>=0&&col1<=string_length(tb->lines[line1]));
	assert(col2>=0&&col2<=string_length(tb->lines[line2]));
	assert(line2>line1||col2>=col1);
	if(line1==line2&&col1==col2)return;
	if(line1==line2){
		string_erase(tb->lines[line1],col1,col2-col1);
		return;
	}
	string_erase(tb->lines[line1],col1,-1);
	string_erase(tb->lines[line2],0,col2);
	for(int i=line1+1;i<line2;i++)string_destroy(tb->lines[i]);
	memmove(tb->lines+(line1+1),tb->lines+line2,(tb->len-line2)*sizeof(String*));
	tb->len-=line2-line1-1;
}

void textbuf_clear(Textbuf *tb){
	for(int i=0;i<tb->len;i++)string_destroy(tb->lines[i]);
	tb->len=0;
}

int textbuf_nlines(const Textbuf *tb){
	assert(tb);
	return tb->len;
}

int textbuf_linelen(const Textbuf *tb,int line){
	assert(tb);
	assert(line>=0&&line<tb->len);
	return string_length(tb->lines[line]);
}

char textbuf_at(const Textbuf *tb,int line,int col){
	assert(tb);
	assert(line>=0&&line<tb->len);
	assert(col>=0&&col<string_length(tb->lines[line]));
	return string_at(tb->lines[line],col);
}

const String* textbuf_line(const Textbuf *tb,int line){
	assert(tb);
	assert(line>=0&&line<tb->len);
	return tb->lines[line];
}
