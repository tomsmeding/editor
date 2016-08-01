#include <stdio.h>

#include "disk.h"
#include "memory.h"
#include "string.h"
#include "util.h"

String* disk_read(const char *fname){
	FILE *f=fopen(fname,"rb");
	if(!f)return NULL;
	fseek(f,0,SEEK_END);
	int flen=ftell(f);
	rewind(f);
	char *buf=malloc(flen+1,char);
	if(!buf)outofmem();
	int nread=fread(buf,1,flen,f);
	fclose(f);
	if(nread<flen){
		free(buf);
		return false;
	}
	return string_frombufnocopy(buf,flen);
}

bool disk_write(const char *fname,const String *s);
