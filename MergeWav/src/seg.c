/******************************************************************************/
/*                                                                            */
/*                                  seg.c                                     */
/*                                                                            */
/*                        Audio Segmentation Library                          */
/*                                                                            */
/* Guig                                                       Mon Aug  5 2002 */
/* -------------------------------------------------------------------------- */
/*  Copyright (C) 2002 IRISA                                                  */
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
 *   $Author: guig $
 *   $Date: 2010-01-14 10:39:52 +0100 (Thu, 14 Jan 2010) $
 *   $Revision: 102 $
 *
 */

/*
 * Segment and segmentation related stuff. 
 *
 * A segment (asseg_t) is a chunk of signal and is defined by its
 * name, start and end times and its score. The name is actually a
 * list of names since several events can be present on a single
 * chunks of signal. A segmentation is a list of segments, organized
 * here as a d-list.
 *
 * Externally, segmentations are stored in text files with one line per
 * segment according to the following syntax:
 *
 *   label [st [et [score]]] [# comment]
 *
 * where label is a string possibly containing several names separated by
 * a '+' sign. The different fields are separated by blanks (space or tabs
 * as defined by isspace()). Empty lines and comment lines are authorized.
 */

#define _seg_c_

#include "audioseg.h"
#include <STDLIB.H>
#include <STRING.H>
# define MAX_LINE_LEN     1024    /* maximum line length in segmentation file */
# define COMMENT_CHAR '#'         /* comment character in segmentation file   */

/* ------------------------------------ */
/* ----- asseg_t *seg_alloc(void) ----- */
/* ------------------------------------ */
/*
 * Allocate memory for a segment.
 */
asseg_t *seg_alloc(void)
{
  asseg_t *p;

  if ((p = (asseg_t *)malloc(sizeof(asseg_t))) == NULL) {
    fprintf(stderr, "seg_alloc() -- unable to allocate %d bytes\n", sizeof(asseg_t));
    return(NULL);
  }

  p->label = (seglab_t *)NULL;
  p->st = p->et = ASEG_NULL_TIME;
  p->score = 0.0;
  p->next = p->prev = NULL;

  return(p);
}

/* ------------------------------------------------------------ */
/* ----- asseg_t *seg_create(char *, float, float, float) ----- */
/* ------------------------------------------------------------ */
/*
 * Create a segment with the specified attributes.
 */
asseg_t *seg_create(char *name, float st, float et, float score)
{
  asseg_t *p;

  if ((p = (asseg_t *)malloc(sizeof(asseg_t))) == NULL) {
    fprintf(stderr, "seg_create() -- unable to allocate %d bytes\n", sizeof(asseg_t));
    return(NULL);
  }

  p->label = (seglab_t *)NULL;
  p->st = st;
  p->et = et;
  p->score = score;
  p->next = p->prev = NULL;

  if (name) {
    if ((p->label = str_to_label(name)) == NULL) {
      fprintf(stderr, "seg_create() -- cannot set segment label\n");
      seg_free(p);
    }
  }

  return(p);
}

/* ------------------------------------ */
/* ----- void seg_free(asseg_t *) ----- */
/* ------------------------------------ */
/*
 * free memory for a segment.
 */
void seg_free(asseg_t *p) {

  if (p) {
    seg_label_free(p->label);
    free(p);
  }
}

/* ----------------------------------------- */
/* ----- void seg_list_free(asseg_t *) ----- */
/* ----------------------------------------- */
/*
 * Free segment list.
 */
void seg_list_free(asseg_t *p)
{
  asseg_t *next = NULL, *curr = p;

  while (curr) {
    next = curr->next;
    seg_free(curr);
    curr = next;
  }
}

/* ----------------------------------------------------- */
/* ----- seglab_t *seg_label_alloc(unsigned short) ----- */
/* ----------------------------------------------------- */
/* 
 * Allocate memory for a segment label.
 */
seglab_t *seg_label_alloc(unsigned short nlabs)
{
  unsigned short i;
  seglab_t *p;

  if ((p = (seglab_t *)malloc(sizeof(seglab_t))) == NULL) {
    fprintf(stderr, "seg_label_alloc() -- cannot allocate %d bytes\n", sizeof(seglab_t));
    return(NULL);
  }
  p->nlabels = 0;
  p->name = (char **)NULL;

  if (nlabs) {
    if ((p->name = (char **)malloc(nlabs * sizeof(char *))) == NULL) {
      fprintf(stderr, "seg_label_alloc() -- cannot allocate %d bytes\n", nlabs * sizeof(char *));
      free(p);
      return(NULL);
    }
    for (i = 0; i < nlabs; i++)
      p->name[i] = NULL;
    p->nlabels = nlabs;
  }

  return(p);
}

/* ------------------------------------------- */
/* ----- void seg_label_free(seglab_t *) ----- */
/* ------------------------------------------- */
/*
 * Free memory allocated for a segment label.
 */
void seg_label_free(seglab_t *p)
{
  unsigned short i;

  if (p) {
    for (i = 0; i < p->nlabels; i++)
      if (p->name[i])
	free(p->name[i]);
    free(p->name);
    free(p);
  }
}

/* ------------------------------------------------ */
/* ----- int set_seg_label(asseg_t *, char *) ----- */
/* ------------------------------------------------ */
/*
 * Set segment labels where the labels are given as a string of names
 * separated by a + sign. Return number of labels created or -1 in
 * case of error.
 */
int set_seg_label(asseg_t *p, char *name)
{
  if (name) {
    if (p->label)
      seg_label_free(p->label);
    
    if ((p->label = str_to_label(name)) == NULL) {
      fprintf(stderr, "set_seg_label() -- cannot convert string '%s' to label\n", name);
      return(-1);
    }
  }
  else {
    p->label = NULL;
    return(0);
  }

  return(p->label->nlabels);
}

/* ------------------------------------------------ */
/* ----- int add_seg_label(asseg_t *, char *) ----- */
/* ------------------------------------------------ */
/*
 * Add segment labels where the labels are given as a string of names
 * separated by a + sign. Return number of labels added or -1 in
 * case of error.
 */
int add_seg_label(asseg_t *p, char *name)
{
  unsigned short i, j, nadded = 0;
  seglab_t *lab;
  
  if (name) {
    
    if (p->label == NULL)
      return(set_seg_label(p, name));
    
    if ((lab = str_to_label(name)) == NULL) {
      fprintf(stderr, "add_seg_label() -- cannot convert string '%s' to label\n", name);
      return(-1);
    }
    
    nadded = lab->nlabels;
    
    if ((p->label->name = (char **)realloc(p->label->name, (p->label->nlabels + nadded) * sizeof(char *))) == NULL) {
      fprintf(stderr, "set_seg_label() -- cannot allocate %d bytes\n", (p->label->nlabels + nadded) * sizeof(char *));
      p->label->nlabels = 0;
      return(-1);
    }
    
    for (i = 0, j = p->label->nlabels; i < nadded; i++, j++) {
      if ((p->label->name[j] = strdup(lab->name[i])) == NULL) {
	fprintf(stderr, "set_seg_label() -- cannot allocate %d bytes\n", strlen(lab->name[i]));
	return(-1);
      }
      p->label->nlabels++;
    }

    seg_label_free(lab);
  }

  return(nadded);
}

/* --------------------------------------------------------------------- */
/* ----- int seg_label_compare(const seglab_t *, const seglab_t *) ----- */
/* --------------------------------------------------------------------- */
/*
 * Compare two segment labels. Return 0 if they match or 1
 * otherwise. 
 *
 * Note that a NULL label stands for *any* label so that if one of the
 * label is null, the match is considered correct.
 */
int seg_label_compare(const seglab_t *lab1, const seglab_t *lab2)
{
  unsigned short i;

  if (lab1 == NULL || lab2 == NULL)
    return(0);

  if (lab1->nlabels != lab2->nlabels)
    return(1);

  for (i = 0; i < lab1->nlabels; i++)
    if (seg_label_name_index(lab2, lab1->name[i]) < 0)
      return(1);

  return(0);
}

/* -------------------------------------------------------------------- */
/* ----- int seg_label_name_index(const seglab_t *, const char *) ----- */
/* -------------------------------------------------------------------- */
/*
 * Return index of name in the label name array, or -1 if not found.
 */
int seg_label_name_index(const seglab_t *lab, const char *name)
{
  unsigned short i;

  for (i = 0; i < lab->nlabels; i++)
    if (strcmp(lab->name[i], name) == 0)
      return(i);

  return(-1);
}

/* ------------------------------------------------ */
/* ----- seglab_t *str_to_label(const char *) ----- */
/* ------------------------------------------------ */
/*
 * Convert a string of names separated by + signs to a label.
 */
seglab_t *str_to_label(const char *str)
{
  int i;
  unsigned short n = 1;
  char *c, *dup;
  seglab_t *p;

  if (! str)
    return(NULL);

  if ((dup = strdup(str)) == NULL)
    return(NULL);

  /* count the number of labels */
  c = dup;
  while (*c) {
    if (*c == '+')
      n++;
    c++;
  }

  if ((p = seg_label_alloc(n)) == NULL) {
    free(dup);
    return(NULL);
  }

  c = strtok(dup, "+");
  for (i = 0; i < n; i++) {
    if ((p->name[i] = strdup(c)) == NULL) {
      free(dup); seg_label_free(p);
      return(NULL);
    }
    c  = strtok(NULL, "+");
  }

  free(dup);

  return(p);
}

/* ------------------------------------------- */
/* ----- asseg_t *seg_read(const char *) ----- */
/* ------------------------------------------- */
/*
 * Read a segmentation from file.
 *
 * NOTE: the current function is *very* permissive wrt the syntax.
 */
asseg_t *seg_read(const char *fn)
{
  char line[MAX_LINE_LEN];
  FILE *f;
  char *p;
  char *name;
  asseg_t *seg = NULL, *curr, *prev = NULL;
  float st, et, score;
  int lino = 0;

  /* ----- open file ----- */
  if (fn) {
    if (strcmp(fn, "-") == 0)
      f = stdin;
    else if ((f = fopen(fn, "r")) == NULL) {
      fprintf(stderr, "seg_read() -- cannot open input file %s\n", fn);
      return(NULL);
    }
  }
  else
    f = stdin;

  /* ----- process every line ----- */
  while (fgets(line, MAX_LINE_LEN, f) != NULL) {

    lino++;

    /* remove comments - i.e. search the # in the line */
    if ((p = (char *)strchr(line, COMMENT_CHAR)) != NULL)
      *p = 0x00; /* set end of string there */

    /* skip leading blanks */
    p = line;
    while (*p && ISSPACE(*p))
      p++;

    /* skip empty lines */
    if (! (*p)) {
      lino++;
      continue;
    }

    /* get the name, start and end times, score */
    st = et = ASEG_NULL_TIME;
    score = ASEG_NULL_SCORE;
    name = p;
    while (*p && ! ISSPACE(*p))
      p++;
    if (*p) {
      *p = 0x00;
      sscanf(p+1, "%f %f %f", &st, &et, &score);
    }
    
    /* create segment and append it to the list */
    if ((curr = seg_create(name, st, et, score)) == NULL) {
      fprintf(stderr, "seg_read() -- cannot create segment\n");
      if (f != stdin) fclose(f);
      seg_list_free(seg);
      return(NULL);
    }

    if (prev) {
      curr->prev = prev;
      prev->next = curr;
    }
    else
      seg = curr;

    prev = curr;
  }

  if (f != stdin)
    fclose(f);

  return(seg);
}

/* ---------------------------------------------------------------------- */
/* ----- int seg_write(const asseg_t *, const char *, const char *) ----- */
/* ---------------------------------------------------------------------- */
/*
 * Write a segmentation to file. Return the number of segment written
 * to file.
 */
int seg_write(const asseg_t *seg, const char *fn, const char *format)
{
  FILE *f;
  const asseg_t *p = seg;
  int i, nwritten = 0;

  if (fn) {
    if (strcmp(fn, "-") == 0)
      f = stdout;
    else if ((f = fopen(fn, "w")) == NULL) {
      fprintf(stderr, "seg_write() -- cannot open output file %s\n", fn);
      return(0);
    }
  }
  else
    f = stdout;

  while (p) {
    if (format == NULL || strchr(format, 'l') || strchr(format, 'L'))
      for (i = 0; i < get_seg_num_labels(p); i++)
	fprintf(f, (i) ? "-%s" : "%s", get_seg_label_name(p, i));
    if ( (format == NULL || strchr(format, 's') || strchr(format, 'S')) && get_seg_start_time(p) != ASEG_NULL_TIME )
      fprintf(f, " %-.5f", get_seg_start_time(p));
    if ( (format == NULL || strchr(format, 'e') || strchr(format, 'E')) && get_seg_end_time(p) != ASEG_NULL_TIME )
      fprintf(f, " %-.5f", get_seg_end_time(p));
    if ( (format == NULL || strchr(format, 'p') || strchr(format, 'P')) && get_seg_score(p) != ASEG_NULL_SCORE )
      fprintf(f, " %e", get_seg_score(p));
    fprintf(f, "\n");

    nwritten++;
    p = p->next;
  }

  if (f != stdout)
    fclose(f);

  return(nwritten);
}

#undef _seg_c_

