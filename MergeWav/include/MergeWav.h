#include "ssad.h"
#include "wavheader.h"

int seg_write_file(float start, float end, FILE* infp, FILE* outfp);
int MergeWav(const char* infilename, const char* outfilename);