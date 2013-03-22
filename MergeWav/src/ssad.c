/******************************************************************************/
/*                                                                            */
/*                                  ssad.c                                    */
/*                                                                            */
/*****************************************************************************
 * Signal activity detector.
 *
 * Signal activity detector based on a bi-gaussian model of the frame
 * energy distribution and a maximum likelihood criterion for
 * classification. The program outputs a segmentation wich identifies
 * either the silence segments, or the audio signal segments or even
 * both. A minimum silence segment length can be specified to avoid
 * over-segmentation problems due to the processing at the frame
 * level.
 */

#define _ssad_c_

#include "ssad.h"

/* ----------------------------------------------- */
/* ----- global variables set by read_args() ----- */
/* ----------------------------------------------- */
int channel = 1;
float st = 0.0;                   /* start and end times                      */
float et = ASEG_NULL_TIME;
float fm_l = 20.0;                /* frame length in ms                       */
float fm_d = 10.0;                /* frame shift in ms                        */
int win = 0;                      /* weighting window                         */
float threshold = 0.0;            /* deviation w.r.t. standard deviation      */
float minlen = 0.5;               /* minimum silence segment length           */
int ofmt = SPEECH;               /* output format                            */
int uselog = 1;                   /* use log-energy rather than energy        */

/* ----------------------------------------------------- */
/* ----- asseg_t *silence_detection(sigstream_t *) ----- */
/* ----------------------------------------------------- */
/*
 * process input file.
 */
asseg_t *silence_detection(sigstream_t *s)
{
  bigauss_t bg;
  spfbuf_t *e;
  asseg_t *seg;
  unsigned short nl, nd;
  double emin, emax;

  spfbuf_t *get_energy_profile(sigstream_t *, unsigned short, unsigned short, double *, double *);
  void init_bigauss(bigauss_t *, double, double);
  void buf_to_bigauss(spfbuf_t *, bigauss_t *, int, double);
  asseg_t *profile_to_seg(spfbuf_t *, bigauss_t *, float);

  nl = (unsigned short)(fm_l * s->Fs / 1000.0);
  nd = (unsigned short)(fm_d * s->Fs / 1000.0);

  if ((e = get_energy_profile(s, nl, nd, &emin, &emax)) == NULL)
    return(NULL);

  init_bigauss(&bg, emin, emax);
  buf_to_bigauss(e, &bg, 20, 0.0001);

  /* ----- convert profile to segmentation ----- */
  if ((seg = profile_to_seg(e, &bg, nd / s->Fs)) == NULL) {
    fprintf(stderr, "ssad error -- cannot create output segmentation\n");
    spf_buf_free(e); seg_list_free(seg);
    return(NULL);
  }

  spf_buf_free(e);

  return(seg);
}

/* ---------------------------------------------------------------------------- */
/* ----- spfbuf_t *get_energy_profile(sigstream_t *, unsigned short,      ----- */
/* -----                              unsigned short, double *, double *) ----- */
/* ---------------------------------------------------------------------------- */
/*
 * Compute signal stream energy profile.
 */
spfbuf_t *get_energy_profile(sigstream_t *s, unsigned short l, unsigned short d, double *emin, double *emax)
{
  spfbuf_t *buf;
  float *w = NULL;
  //int i;
  sample_t *sbuf;
  spsig_t *frame;
  spf_t e;
  unsigned long n, nact, sn, en, nframes = 0;

  /* ----- initialize some more stuff ----- */
  *emax = FLT_MIN;
  *emin = FLT_MAX;

  sn = (unsigned long)(st * s->Fs / (float)d); /* which frame to start with? */  
  if (et != ASEG_NULL_TIME) {
    en = (unsigned long)(et * s->Fs / (float)d); /* which one's last? */
    nframes = en - sn;
    if (! nframes)
      nframes = 1;
  }
  else 
    en = 0;

  if ((frame = sig_alloc(l)) == NULL) {
    fprintf(stderr, "ssad error -- cannot allocate frame signal buffer\n");
    return(NULL);
  }

  if ((buf = spf_buf_alloc(1, 40000)) == NULL) {
    fprintf(stderr, "ssad error -- cannot allocate output feature buffer\n");
    sig_free(frame);
    return(NULL);
  }

  if (win) {
    if ((sbuf = (sample_t *)malloc(l * sizeof(sample_t))) == NULL) {
      fprintf(stderr, "ssad error -- cannot allocate memory\n");
      sig_free(frame); spf_buf_free(buf);
      return(NULL);
    }
    if ((w = set_sig_win(l, win)) == NULL) {
      fprintf(stderr, "ssad error -- cannot allocate weighting window\n");
      free(sbuf); sig_free(frame); spf_buf_free(buf);
      return(NULL);    
    }
  }
  else
    sbuf = frame->s;

  /* ----- compute profile ----- */
  n = 0; nact = 0;
  
  while (get_next_sig_frame(s, channel, l, d, 0.0, sbuf)) {

    if (n < sn) {
      n += 1;
      continue;
    }

    /* weight signal */
    if (w)
      sig_weight(frame, sbuf, w);

    /* compute frame energy */
    e = (spf_t)sig_normalize(frame, 0);
    if (uselog) 
      e = (e < SPRO_ENERGY_FLOOR) ? (spf_t)log(SPRO_ENERGY_FLOOR) : (spf_t)log(e);

    if (spf_buf_append(buf, &e, 1, 10000) == NULL) {
      fprintf(stderr, "ssad error -- cannot append energy value to output feature buffer\n");
      free(sbuf); sig_free(frame); if (win) {spf_buf_free(buf); free(w);}
      return(NULL);
    }
    
    if (e > *emax)
      *emax = e;
    if (e < *emin)
      *emin = e;

    n += 1;
    nact += 1;
    if (nact == nframes)
      break;
  }

  /* ---- clean and get out of here! ----- */
  sig_free(frame); 
  if (win) {
    free(sbuf); 
    free(w);
  }

  return(buf);
}

/* ---------------------------------------------------------- */
/* ----- void init_bigauss(bigauss_t *, double, double) ----- */
/* ---------------------------------------------------------- */
/*
 * Initialize bi Gaussian model.
 */
void init_bigauss(bigauss_t *bg, double emin, double emax)
{
  double d = (emax - emin) / 8.0; 

  bg->m[0] = emin + d;
  bg->v[0] = 1.0; 
  
  bg->m[1] = emax - d;
  bg->v[1] = 1.0; /* v = 1/v */

  bg->c[0] = -0.5 * bg->m[0] * bg->m[0]; /* c = 0.5 * ( ln(1/v) - 1/v * m * m ) */ 
  bg->c[1] = -0.5 * bg->m[1] * bg->m[1];
}

/* ---------------------------------------------------------------------- */
/* -----  void buf_to_bigauss(spfbuf_t *, bigauss_t *, int, double) ----- */
/* ---------------------------------------------------------------------- */
/*
 * Map buffer features to bi-gaussian.
 */
void buf_to_bigauss(spfbuf_t *e, bigauss_t *bg, int maxiter, double epsilon)
{
  unsigned long i, t, n1, n2;
  double m1, v1, m2, v2; /* accumulators */
  double m;
  double llk1, llk2; /* log-likelihoods */
  double v, vv; /* sample value */
  double llk, llkmem = 0.0; /* total data log-probability */

  i = 0;

  while (i < maxiter) {
    /* set accumulators to zero */
    n1 = n2 = 0;
    m1 = m2 = v1 = v2 = 0.0;
    llk = 0;

    /* loop on energy profile samples */
    for (t = 0; t < e->n; t++) {
      v = *((e->s)+t);
      vv = v * v;

      llk1 = bg->v[0] * bg->m[0] * v - 0.5 * bg->v[0] * vv + bg->c[0];
      llk2 = bg->v[1] * bg->m[1] * v - 0.5 * bg->v[1] * vv + bg->c[1];

      if (llk1 > llk2) { /* mapped to gaussian #1 */
	m1 += v;
	v1 += vv;
	n1++;
	llk += llk1;
      }
      else { /* mapped to gaussian #2 */
	m2 += v;
	v2 += vv;
	n2++;
	llk += llk2;
      }
    }
    
   if (i&&llkmem)
	   if ((llkmem - llk) / llkmem < epsilon)
	break;

    /* update models */
    m = m1 / (double)n1;
    v = (double)n1 / (v1 - m * m1); /* this is the inverse of the variance estimator */
    bg->m[0] = m;
    bg->v[0] = v;
    bg->c[0] = 0.5 * (log(v) - m * m * v) ;

    m = m2 / (double)n2;
    v = (double)n2 / (v2 - m * m2); /* this is the inverse of the variance estimator */
    bg->m[1] = m;
    bg->v[1] = v;
    bg->c[1] = 0.5 * (log(v) - m * m * v) ;

    i++;
  }
}

/* ---------------------------------------------------------------------------- */
/* ------ asseg_t *profile_to_seg(spfbuf_t *, bigauss_t *, char *, float) ----- */
/* ---------------------------------------------------------------------------- */
/*
 * Create segmentation from features and the two gaussians.
 */
asseg_t *profile_to_seg(spfbuf_t *e, bigauss_t *bg, float frate)
{
  unsigned long i;
  float st1, et1; /* silence segment start and end times */
  float st2, et2; /* speech segment start and end times */
  unsigned long ns1, ns2;
  double d1, d2;
  asseg_t *seg = NULL;
  int state, label;
  double v, vv, logp1, logp2;
  
  asseg_t *add_seg(asseg_t **, float, float, int);
  
  state = UNKNOWN;
  st1 = st2 = 0.0;
  et1 = et2 = ASEG_NULL_TIME;
  ns1 = ns2 = 0;
  d1 = d2 = 0.0;

  for (i = 0; i < e->n; i++) {

    v = *(e->s+i);

    if (threshold != 0.0)
      label = (v < bg->m[1] - threshold *  sqrt(1.0 / bg->v[1])) ? (SILENCE) : (SPEECH);
    else {
      vv = v * v;    
      logp1 = bg->v[0] * bg->m[0] * v - 0.5 * bg->v[0] * vv + bg->c[0];
      logp2 = bg->v[1] * bg->m[1] * v - 0.5 * bg->v[1] * vv + bg->c[1];

      label = (logp1 > logp2) ? (SILENCE) : (SPEECH);
    }

    if (state == SPEECH && label == SILENCE) { /* potential end of a signal segment */
      et2 = (float)i * frate; /* detected a [st2,et2] speech segment */
      /* fprintf(stderr, " detected speech st=%.2f  et=%.2f\n", st2, et2); */
      st1 = et2; /* start new silence segment */
    }
    else if (state == SILENCE && label == SPEECH) { /* potential end of a silence segment */
      et1 = (float)i * frate; /* detected a [st1,et1] silence segment */
      /* fprintf(stderr, " detected silence st=%.2f  et=%.2f\n", st1, et1); */

      /* if segment is long enough, add the previous speech segment
	 and the current silence segment. Else, simply ignore the
	 silence segment and proceed... */

      if (et1 - st1 > minlen) {
	
	/* add previous speech segment if there was one */
	if (ofmt & SPEECH && et2 != ASEG_NULL_TIME) {
	  /* fprintf(stderr, " adding speech st=%.2f  et=%.2f\n", st2, et2); */
	  ns2++;
	  d2 += (et2 - st2);
	  if (add_seg(&seg, st + st2, st + et2, SPEECH) == NULL) {
	    seg_list_free(seg);
	    return(NULL);
	  }
	}
	
	/* add current silence segment */
	if (ofmt & SILENCE) {
	  ns1++;
	  d1 += (et1 - st1);
	  if (add_seg(&seg, st + st1, st + et1, SILENCE) == NULL) {
	    seg_list_free(seg);
	    return(NULL);
	  }
	}
	st2 = et1; /* set start of new speech segment */
      }
    
    }
    state = label;
  }

  /* check out if there's a last segment to output */
  if (state == SILENCE) {
      et1 = (float)i * frate; /* detected a [st1,et1] silence segment */
      
      /* if segment is long enough, add the previous speech segment
	 and the current silence segment. Else, add the last speech
	 segment. */
      
      if (et1 - st1 > minlen) {
	
	/* add previous speech segment */
	if (ofmt & SPEECH && et2 != ASEG_NULL_TIME) {

	  if (add_seg(&seg, st + st2, st + et2, SPEECH) == NULL) {
	    seg_list_free(seg);
	    return(NULL);
	  }
	  
	  ns2++;
	  d2 += (et2 - st2);
	}
	
	/* add current silence segment */
	if (ofmt & SILENCE) {

	  if (add_seg(&seg, st + st1, st + et1, SILENCE) == NULL) {
	    seg_list_free(seg);
	    return(NULL);
	  }

	  ns1++;
	  d1 += (et1 - st1);
	}
      }
      else if (ofmt & SPEECH) {
	et2 = et1;

	if (add_seg(&seg, st + st2, st + et2, SPEECH) == NULL) {
	  seg_list_free(seg);
	  return(NULL);
	}
	
	ns2++;
	d2 += (et2 - st2);
      }
  }
  else if (ofmt & SPEECH) {
    et2 = (float)i * frate; /* detected a [st2,et2] speech segment */

    if (add_seg(&seg, st + st2, st + et2, SPEECH) == NULL) {
      seg_list_free(seg);
      return(NULL);
    }    

    ns2++;
    d2 += (et2 - st2);
  }
  
  /* adjust end time to exact specified time if et is given */
  if (et != ASEG_NULL_TIME) {
    asseg_t *p = seg;
    while (p->next)
      p = p->next;
    if (fabs(et - get_seg_end_time(p)) < frate)
      set_seg_end_time(p, et);
  }
  return(seg);
}

/* ----------------------------------------------------------- */
/* ----- asseg_t *add_seg(asseg_t **, float, float, int) ----- */
/* ----------------------------------------------------------- */
/*
 * Add a segment after the current segment, returning the adress of
 * the new segment.
 */
asseg_t *add_seg(asseg_t **seg, float st, float et, int label)
{
  static asseg_t *last = NULL;
  asseg_t *p;
  char silstr[] = SILENCE_STRING;
  char sigstr[] = SPEECH_STRING;

  if (label == SILENCE)
    p = seg_create(silstr, st, et, 0.0);
  else
    p = seg_create(sigstr, st, et, 0.0);

  if (*seg == NULL)
    *seg = p;
  
  if (last)
    last->next = p;

  p->prev = last;
  last = p;
  
  return(p);
}

#undef _ssad_c_