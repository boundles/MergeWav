#ifndef _wavheader_h
#define _wavheader_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct{
    short channels;
    short bits;
    int rate;
	int datasize;
}head_pama;

head_pama wav_header_read(const char* wavfile);
void wav_write_header(FILE* fp,head_pama pt);

#endif