#include "wavheader.h"

head_pama wav_header_read(const char* wavfile)
{
    char ChunkID[4],Format[4],Subchunk1ID[4],Subchunk2ID[4];
    int  ChunkSize,Subchunk1Size,SampleRate,Subchunk2Size;
    short AudioFormat,NumChannels,BitsPerSample;
    head_pama pt;

	FILE* fp;
	fp=fopen(wavfile,"rb+");
	if(fp == NULL)
	{
		printf("Can't open wave file!\n");
	}

    fseek(fp, 0, SEEK_SET);
    fread(ChunkID, 1, 4, fp);
    if(strncmp("RIFF", ChunkID, 4) != 0)
    {
        printf("The input file is not wav format!\n");
        exit(2);
    }

    fread(&ChunkSize, 4, 1, fp);

    fread(Format, 1, 4, fp);
    if(strncmp("WAVE", Format, 4) != 0)
    {
        printf("The input file is not wav format!\n");
        exit(2);
    }

    fread(Subchunk1ID, 1, 4, fp);
    if(strncmp("fmt ", Subchunk1ID, 4) != 0)
    {
        printf("The input file is not wav format!\n");
        exit(2);
    }
    
    fread(&Subchunk1Size, 4, 1, fp);
	
    fread(&AudioFormat, 2, 1, fp);

    fread(&NumChannels, 2, 1, fp);
 
    fread(&SampleRate , 4, 1, fp);

    fseek(fp,34,SEEK_SET);
    fread(&BitsPerSample, 2, 1, fp);

    fread(Subchunk2ID, 1, 4, fp);
    if(strncmp("data", Subchunk2ID, 4) != 0)
    {
        printf("This file is not wave!\n");
        exit(2);
    }

	fseek(fp,40,SEEK_SET);
	fread(&Subchunk2Size, 4, 1, fp);

    pt.bits=BitsPerSample;
    pt.channels=NumChannels;
    pt.rate=SampleRate;
	pt.datasize=Subchunk2Size;
    fclose(fp);
    return pt;
}

void wav_write_header(FILE* fp,head_pama pt)
{
    int long_temp;
    short short_temp;
    short BlockAlign;
    char *data = NULL;

    data = "RIFF";
    fwrite(data, sizeof(char), 4, fp);

    long_temp=pt.datasize*2+36;
	fwrite(&long_temp, sizeof(long_temp), 1, fp);

    data = "WAVE";
    fwrite(data, sizeof(char), 4, fp);

    data = "fmt ";
    fwrite(data, sizeof(char), 4, fp);

    long_temp = 16;
    fwrite(&long_temp, sizeof(long_temp), 1, fp);

    short_temp = 0x0001;
    fwrite(&short_temp, sizeof(short_temp), 1, fp);

    short_temp = pt.channels;
    fwrite(&short_temp, sizeof(short_temp), 1, fp);

    long_temp = pt.rate;
    fwrite(&long_temp, sizeof(long_temp), 1, fp);
    
    long_temp = ((pt.bits)/8) * (pt.channels) * (pt.rate);
    fwrite(&long_temp, sizeof(long_temp), 1, fp);

    BlockAlign = ((pt.bits)/8) * (pt.channels);
    fwrite(&BlockAlign, sizeof(BlockAlign), 1, fp);

    short_temp = pt.bits;
    fwrite(&short_temp, sizeof(short_temp), 1, fp);

    data = "data";
    fwrite(data, sizeof(char), 4, fp);

    long_temp = pt.datasize*(pt.channels)*(pt.bits/8);
    fwrite(&long_temp, sizeof(long_temp), 1, fp);
}