#ifndef _ssad_h_
#define _ssad_h_

#include "audioseg.h"
#include <STRING.H>
#include <STDLIB.H>
#include <MATH.H>
static char *cvsid = "$Id: ssad.c 94 2009-07-30 07:38:35Z guig $";

/* output strings */
#define SILENCE_STRING "sil"
#define SPEECH_STRING  "speech"

/* output modes */
#define UNKNOWN 0
#define SILENCE 1
#define SPEECH 2
#define BOTH 3

/* ---------------------------------- */
/* ----- local type definitions ----- */
/* ---------------------------------- */
typedef struct {
  float m[2];
  float v[2];
  double c[2];
} bigauss_t;

asseg_t *silence_detection(sigstream_t *s);

spfbuf_t *get_energy_profile(sigstream_t *s, unsigned short l, unsigned short d, double *emin, double *emax);

void init_bigauss(bigauss_t *bg, double emin, double emax);

void buf_to_bigauss(spfbuf_t *e, bigauss_t *bg, int maxiter, double epsilon);

asseg_t *profile_to_seg(spfbuf_t *e, bigauss_t *bg, float frate);

asseg_t *add_seg(asseg_t **seg, float st, float et, int label);

#endif /* _ssad_h_ */