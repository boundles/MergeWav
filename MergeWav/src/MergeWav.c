#include "MergeWav.h"

int seg_write_file(float start, float end, FILE* infp, FILE* outfp)
{
	char buffer[800];
	char silSample[4800] = {0};
	int sampleCount, readNum, restCount, startByte, i;
	const float sampleRate = 16000.0;
	const unsigned int bytespersample = 2;
	startByte = (int)(start*sampleRate)*bytespersample+44;
	sampleCount = ((int)((end-start)*sampleRate))*2;
	fseek(infp, startByte, SEEK_SET);

	if(sampleCount<800)
	{
		fread(buffer, 1, sampleCount, infp);
		fwrite(buffer, 1, sampleCount, outfp);
		fwrite(silSample, 1, 4800, outfp);
	}
	else
	{
		readNum = floor((double)sampleCount/sizeof(buffer));
		restCount = sampleCount-readNum*sizeof(buffer);
		for(i = 0; i<readNum; i++)
		{
			fread(buffer, 1, sizeof(buffer), infp);
			fwrite(buffer, 1, sizeof(buffer), outfp);
		}
		fread(buffer, 1, restCount, infp);
		fwrite(buffer, 1, restCount, outfp);
		fwrite(silSample, 1, 4800, outfp);
	}

	return 0;
}

int MergeWav(const char* infilename, const char* outfilename)
{
	 FILE *infp, *outfp;

	 sigstream_t *s;			          /* input signal stream                   */
     asseg_t *seg;
	 int format = SPRO_SIG_PCM16_FORMAT; /* signal file format                     */
     float Fs = 16000.0;                /* input signal sample rate                */
     int swap = 0;                     /* change input sample byte order           */
     size_t ibs = 10000000;            /* input buffer size                        */

	 float start_time = 0.0, end_time = 0.0;
	 int datasize;
	 float temptime, datatime;
	 head_pama header, pt={0,0,0};
	 header=wav_header_read(infilename);
	 if(header.bits != 16)
	 {
		 printf("请输入精度为16bits的音频!\n");
		 return 1;
	 }

	 if(header.channels != 1)
	 {
		 printf("请输入单声道音频!\n");
		 return 1;
	 }

	 if(header.rate != 16000)
	 {
		printf("请输入采样率为16k的音频!\n");
		return 1;
	 }

	 if((s = sig_stream_open(infilename, format, Fs, ibs, swap)) == NULL)
	 {
		fprintf(stderr, "ssad error -- cannot open input signal stream %s\n", (infilename) ? (infilename) : "stdin");
		return(1);
	 }
 
     if((seg = silence_detection(s)) == NULL)
		 return(1);

	 datasize = header.datasize;
	 temptime= (float)datasize/header.rate;
     datatime=temptime/2;
	 pt.bits = header.bits;
	 pt.channels = header.channels;
	 pt.rate = header.rate;

	 infp = fopen(infilename,"rb+");
	 outfp = fopen(outfilename,"wb+");
	 fseek(outfp, 44, SEEK_SET);

	 while(seg)
	 {
		 start_time = get_seg_start_time(seg);
		 end_time = get_seg_end_time(seg);
		 seg_write_file(start_time,end_time,infp,outfp);
		 pt.datasize += ((int)((end_time-start_time)*16000.0));
		 pt.datasize += 4800;
		 seg = seg->next;
	 }
	 fseek(outfp, 0, SEEK_SET);
	 wav_write_header(outfp, pt);
	 fclose(infp);
	 fclose(outfp);
	 /* ----- clean memory ----- */
     sig_stream_close(s);
     seg_list_free(seg);

	 return 0;
}

void main()
{
	char* infile = "C:\\Users\\Administrator\\Desktop\\debug\\Result\\习近平\\全国政协举行新年茶话会.wav";
	char* outfile = "C:\\Users\\Administrator\\Desktop\\debug\\Result\\习近平\\全国政协举行新年茶话会_new.wav";

	MergeWav(infile, outfile);
}