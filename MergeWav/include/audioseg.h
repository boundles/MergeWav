/******************************************************************************/
/*                                                                            */
/*                               audioseg.h                                   */
/*                                                                            */
/*                        Audio Segmentation Library                          */
/*                                                                            */
/* Guig                                                       Sun Aug  4 2002 */
/* -------------------------------------------------------------------------- */
/*  Copyright (C) 2003 IRISA                                                  */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or             */
/*  modify it under the terms of the GNU General Public License               */
/*  as published by the Free Software Foundation; either version 2            */
/*  of the License, or (at your option) any later version.                    */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place - Suite 330,                            */
/*  Boston, MA  02111-1307, USA.                                              */
/*                                                                            */
/******************************************************************************/
/*
 * CVS log:
 *
 * $Author: guig $
 * $Date: 2010-01-14 10:39:52 +0100 (Thu, 14 Jan 2010) $
 * $Revision: 102 $
 *
 */

/*
 * Audio Segmentation Library Header.
 */

#ifndef _audioseg_h_
# define _audioseg_h_

#include "system.h"
#include "spro.h"
# if HAVE_GSL
#include <gsl/gsl_linalg.h>
#endif /* HAVE_GSL */

     /* ---------------------------- */
     /* ----- some hard limits ----- */
     /* ---------------------------- */

# define ASEG_NULL_TIME  -1.0          /* null time                            */
# define ASEG_NULL_SCORE (float)(-1e-20) /* null score                         */

# define GM_DIAG_COV 0                 /* Gaussian covariance type             */
# define GM_FULL_COV 1

# define GMM_NUM_COMP_MAX   4096       /* maximum number of components         */
# define GMM_MIN_WEIGHT     1e-5       /* minimum component weight             */

# define W_UP 1                        /* update weights                       */
# define M_UP 2                        /* update means                         */
# define V_UP 4                        /* update variances                     */

     /* ---------------------------- */
     /* ----- type definitions ----- */
     /* ---------------------------- */

/*
   segment definition
*/

typedef struct {
  unsigned short nlabels;       /* number of labels                           */
  char **name;                  /* segment labels                             */
} seglab_t;  

typedef struct asseg_s {
  seglab_t *label;              /* segment label                              */
  float st, et;                 /* segment start and end times                */
  float score;                  /* segment score (whatever this is)           */
  struct asseg_s *next;         /* next segment in a "transcription"          */
  struct asseg_s *prev;         /* next segment in a "transcription"          */
} asseg_t;

/*
   Gaussian model definition
*/

typedef struct {
  unsigned short dim;          /* model dimension                      */
  double * mean;               /* mean vector                          */
  char covtype;                /* one of GM_DIAG_COV or GM_FULL_COV    */
  double * var;                /* variance vector if GM_DIAG_COV       */
#if HAVE_GSL
  gsl_matrix * covmat;         /* covariance matrix if GM_FULL_COV     */
  gsl_matrix * icovmat;        /* inverse covariance matrix            */
  struct gsl_ws_s {            /* workspace for matrix operations      */
    char is_set;               /* whether decomposition done or not    */
    gsl_matrix * cholesky;     /* cholesky decompostion                */
  } * ws;
#endif /* HAVE_GSL */               
  double logdet;               /* log-determinant                      */
  double logz;                 /* log of normalization factor          */
} gm_t;

typedef struct {
  unsigned short dim;          /* model dimension                      */
  char covtype;                /* one of GM_DIAG_COV or GM_FULL_COV    */
  unsigned long nsamples;      /* number of samples accumulated        */
  double * mean;               /* mean accumulator                     */
  double **var;                /* variance accumulator                 */
} gmacc_t;

/*
   Gaussian mixture model definition
*/

typedef struct {
  unsigned short n;             /* number of Gaussian components              */
  unsigned short dim;           /* dimension                                  */
  float *w;                     /* component weights                          */
  float **m;                    /* mean vectors                               */
  float **v;                    /* (inverse) diagonal covariance vectors      */
  double *logc;                 /* some constants                             */
  double *expz;
} gmm_t;                        /* Gaussian mixture model                     */

typedef struct {
  unsigned short n;             /* number of Gaussian components              */
  unsigned short dim;           /* dimension                                  */
  double *w;                    /* component weight accumulator               */
  double **m;                   /* array of mean accumulators                 */
  double **v;                   /* array of covariance accumulators           */
} gmmacc_t;                     /* accumulators                               */

     /* ---------------------------------------- */
     /* ----- segment allocation functions ----- */
     /* ---------------------------------------- */

/* allocate a segment */
asseg_t *seg_alloc(void);

/* create a segment (with attributes) */
asseg_t *seg_create(
  char *,                       /* labels separated by '+'                    */
  float,                        /* start time                                 */
  float,                        /* end time                                   */
  float                         /* score                                      */
);

/* free a segment */
void seg_free(
  asseg_t *                     /* segment address                            */
);

/* free a segment list */
void seg_list_free(
  asseg_t *                     /* pointer to the first segment               */
);

/* allocate memory for a segment label */
seglab_t *seg_label_alloc(
  unsigned short                /* number of labels                           */
);

/* free a segment label */
void seg_label_free(
  seglab_t *                    /* label to free                              */
);

     /* ------------------------------------ */
     /* ----- label handling functions ----- */
     /* ------------------------------------ */

/* compare a two labels, comparison is always true if one of the
   label is NULL. */
int seg_label_compare(
  const seglab_t *,             /* label                                      */
  const seglab_t *              /* label                                      */
);

/* return index of name in the label name array, or -1 if not
   found. */
int seg_label_name_index(
  const seglab_t *,             /* label                                      */
  const char *                  /* name                                       */
);

/* label string to table */
seglab_t *str_to_label(
  const char *                  /* label bames separated by '+'               */
);

# define get_num_labels(p)            ((p)->nlabels)
# define get_label_name(p, i)         ((p)->name[i])

     /* --------------------------------- */
     /* ----- segment I/O functions ----- */
     /* --------------------------------- */

/* read a segmentation */
asseg_t *seg_read(
  const char *                  /* input stream name (or NULL for stdin)      */
);

/* write a segmentation */
int seg_write(
  const asseg_t *,              /* pointer to the first segment               */
  const char *,                 /* output stream name (or NULL for stdout)    */
  const char *                  /* format string (lsep)                       */
);

/* write a segmentation */
int seg_print(
  const asseg_t *             /* pointer to the first segment               */
);

     /* --------------------------------------- */
     /* ----- segment attribute accessors ----- */
     /* --------------------------------------- */

  
/* set segment labels, return the number of labels set or -1 in ase of
   error */
int set_seg_label(
  asseg_t *,                    /* pointer to the first segment               */
  char *                        /* labels separated by '+'                    */
);

/* add labels to the segment, return the number of labels set or -1 in
   case of error */
int add_seg_label(
  asseg_t *,                    /* pointer to the first segment               */
  char *                        /* labels separated by '+'                    */
);

# define get_seg_label(s)             ((s)->label)
# define get_seg_num_labels(s)        (get_num_labels(get_seg_label(s)))
# define get_seg_label_name(s, i)     (get_label_name(get_seg_label(s), i))
# define get_seg_start_time(s)        ((s)->st)
# define get_seg_end_time(s)          ((s)->et)
# define get_seg_score(s)             ((s)->score)
# define get_seg_next(s)              ((s)->next)
# define get_seg_prev(s)              ((s)->prev)

# define set_seg_start_time(s, a)     ((s)->st = (a))
# define set_seg_end_time(s, a)       ((s)->et = (a))
# define set_seg_score(s, a)          ((s)->score = (a))
# define set_seg_next(s, p)           ((s)->next = (p))
# define set_seg_prev(s, p)           ((s)->prev = (p))

# define seg_time_round(a)            (((a - floor(a)) < 0.5) ? (unsigned long)floor(a) : (unsigned long)ceil(a))

     /* -------------------------------------------- */
     /* ----- likelihood computation functions ----- */
     /* -------------------------------------------- */

# if HAVE_GSL
/* invert Gaussian full covariance matrix */
void gm_covmat_invert(
  gm_t *                        /* model                                      */
);

/* compute log-determinant of the covariance matrix */
double gm_covmat_logdet(
  gm_t *                        /* model                                      */
);
#endif

/* pre-compute constants for a Gaussian model */
void gm_const_set(
  gm_t *                        /* model                                      */
);

/* compute the log-likelihood of a frame for a Gaussian model */
double gm_frame_log_like(
  const spf_t *,                /* feature vector                             */
  const gm_t *                  /* model                                      */
);

/* pre-compute constants for a mixture Gaussian model */
void gmm_const_set(
  gmm_t *                       /* model                                      */
);

/* compute segment log-likelihood for a Gausian mixture model */
double gmm_segment_log_like(
  spfstream_t *,                /* input stream                               */	
  unsigned long *,              /* segment length (0 to end of stream)        */  
  const gmm_t *                 /* model                                      */
);

/* computes the log-likelihood of a frame for a Gaussian mixture model */
double gmm_frame_log_like(
  const spf_t *,                /* feature vector                             */
  const gmm_t *                 /* model                                      */
);

/* computes the log-likelihood of a frame for a Gaussian mixture model */
double gmm_nbest_frame_log_like(
  const spf_t *,                /* feature vector                             */
  const gmm_t *,                /* model                                      */
  int *,                        /* indexes of Gaussian's to score             */
  unsigned short,               /* number of Gaussians to score               */
  int                           /* find out N-best or use existing indexes    */
);

     /* ------------------------------------------------ */
     /* ----- model distance computation functions ----- */
     /* ------------------------------------------------ */

double gm_ahs_dist(const gm_t *, const gm_t *);
double gm_kl2_dist(const gm_t *, const gm_t *);


     /* ------------------------------- */
     /* ----- model I/O functions ----- */
     /* ------------------------------- */

gm_t * gm_alloc (unsigned short, char);
void gm_free (gm_t *);

void gm_print (FILE *, const gm_t *);
int gm_write (const char *, gm_t *);
gm_t * gm_read (const char *);

/* allocate memory for a model */
gmm_t *gmm_alloc(
  unsigned short,               /* number of Gaussian components             */
  unsigned short                /* dimension                                 */
);

/* free memory */
void gmm_free(
  gmm_t *                       /* model                                     */
);

/* read model from file */
gmm_t *gmm_read(
  const char *                  /* input filename                            */
);

/* write model to file */
int gmm_write(
  const char *,                 /* output filename                           */
  gmm_t *                       /* model                                     */
);

/* display in ASCII format -- should be in sgmcopy.c but I keep it
   here for debug purposes for the moment. */
void gmm_print(
  FILE *,                       /* output stream                             */
  gmm_t *                       /* model                                     */
);

     /* --------------------------------------------------- */
     /* ----- Gaussian parameter estimation functions ----- */
     /* --------------------------------------------------- */

/* allocate memory for accumulators */
gmacc_t * gm_acc_alloc(
  unsigned short,               /* dimension                                 */
  char                          /* covariance type                           */
);

/* free accumulators */
void gm_acc_free(
  gmacc_t *                     /* accumulators                              */
);

/* zero accumulators */
void gm_acc_reset(
  gmacc_t *                     /* accumulators                              */
);

/* accumulate stats for a segment */
void gm_accumulate(
  spfstream_t *,                /* input feature stream                      */
  unsigned long,                /* number of frames                          */
  gmacc_t *                     /* accumulators                              */
);

/* estimate a model from accumulator (maximum likelihood) */
void gm_estim(
  gm_t *,                       /* model                                     */ 
  gmacc_t *,                    /* accumulators                              */
  int                           /* compute inverse covmat and consts ?       */ 
);

     /* ---------------------------------------------- */
     /* ----- GMM parameter estimation functions ----- */
     /* ---------------------------------------------- */

/* allocate accumulators */
gmmacc_t *gmm_acc_alloc(
  unsigned short,               /* number of Gaussian components             */
  unsigned short                /* dimension                                 */
);

/* free accumulators */
void gmm_acc_free(
  gmmacc_t *                    /* accumulator                               */
);

/* Reset accumulator's values to zero */
void gmm_acc_reset(
  gmmacc_t *                    /* accumulator                               */
);

/* compute Gaussian occupation probabilities and return the frame log-prob. */
double gmm_occ_probs(
  const gmm_t *,                /* model                                     */
  const spf_t *,                /* feature vector                            */
  double *                      /* output occupation probabilities           */
);

/* sum up statistics in the accumulators for a segment */
double gmm_accumulate(
  spfstream_t *,                /* input feature stream                      */
  unsigned long *,              /* number of frames (0 to end of stream)     */
  const gmm_t *,                /* model                                     */
  gmmacc_t *                    /* accumulators                              */
);

/* ML re-estimation */
void gmm_ml_update(
  gmm_t *,                      /* updated model                             */
  gmmacc_t *,                   /* accumulators                              */
  unsigned long,                /* number of accumulated frames              */
  int,                          /* update flag                               */
  double,                       /* weights floor                             */
  double *                      /* variance floor                            */
);

/* MAP re-estimation */
void gmm_map_update(
  gmm_t *,                      /* updated model                             */
  gmmacc_t *,                   /* accumulators                              */
  unsigned long,                /* number of accumulated frames              */
  int,                          /* update flag                               */
  double,                       /* weights floor                             */
  double *,                     /* variance floor                            */
  double,                       /* prior weight                              */
  gmm_t *                       /* prior model                               */
);

/* set variance floor vector */
double *set_var_floor(
  float,                        /* asbolute floor or variance fraction       */ 
  const char *,                 /* global variance filename or NULL          */
  unsigned short                /* feature vector dimension                  */
);

     /* --------------------------------------- */
     /* ----- segment attribute accessors ----- */
     /* --------------------------------------- */

# define get_gmm_num_comp(g)          ((g)->n)
# define get_gmm_dim(g)               ((g)->dim)
# define get_gmm_weights(g)           ((g)->w)

# define get_gmm_comp_weight(g, i)    (*((g)->w+i))
# define get_gmm_comp_mean(g, i)      (*((g)->m+i))
# define get_gmm_comp_ivar(g, i)      (*((g)->v+i))
# define get_gmm_comp_logc(g, i)      (*((g)->logc+i))
# define get_gmm_comp_expz(g, i)      (*((g)->expz+i))

# define set_gmm_comp_weight(g, i, a) (*((g)->w+i) = (a))
# define set_gmm_comp_logc(g, i, a)   (*((g)->logc+i) = (a))
# define set_gmm_comp_expz(g, i, a)   (*((g)->expz+i) = (a))

# define set_gmm_num_comp(g, a)       ((g)->n = (a))
# define set_gmm_dim(g, a)            ((g)->dim = (a))

# define get_acc_num_comp(g)          ((g)->n)
# define get_acc_dim(g)               ((g)->dim)
# define get_acc_weights(g)           ((g)->w)
# define get_acc_comp_weight(g, i)    (*((g)->w+i))
# define get_acc_comp_mean(g, i)      (*((g)->m+i))
# define get_acc_comp_var(g, i)       (*((g)->v+i))

# if defined _gmm_c_ || defined _gauss_c_ || defined _scluster_c_
#  define LOG_2_PI ((double)1.8378770664093453390819377091247588396072) /* ln(2 * M_PI) */
void swap_bytes(void *, unsigned long, size_t);
# endif

#endif /* _audioseg_h_ */
