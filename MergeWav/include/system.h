/******************************************************************************/
/*                                                                            */
/*                                system.h                                    */
/*                                                                            */
/*                        Audio Segmentation Library                          */
/*                                                                            */
/* Guig                                                       Sun Aug  4 2002 */
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
 * $Author: ggravier $
 * $Date: 2003-11-20 16:50:58 +0100 (Thu, 20 Nov 2003) $
 * $Revision: 61 $
 *
 */

/*
 * System dependent includes and defines.
 */

#ifndef _system_h_
# define _system_h_

# ifdef HAVE_CONFIG_H
#  include <config.h>
# endif

/* GNU small library functions.  */
# ifndef PARAMS
#  if PROTOTYPES
#   define PARAMS(Args) Args
#  else
#   define PARAMS(Args) ()
#  endif
# endif

/* Dmalloc stuff */
# if WITH_DMALLOC
#  include <dmalloc.h>
# else
#  if defined STDC_HEADERS || defined _LIBC
#    include <stdlib.h>
#  else
#    ifdef HAVE_MALLOC_H
#      include <malloc.h>
#    endif
#  endif
# endif

# include <stdio.h>

# if HAVE_UNISTD_H
#  include <unistd.h>
# endif

/* string stuff */
# if HAVE_STRING_H
#  if !STDC_HEADERS && HAVE_MEMORY_H
#   include <memory.h>
#  endif
#  include <string.h>
# endif

/* mathematics */
# if HAVE_MATH_H
#  include <math.h>
# endif

/* ctype stuff */
#include <ctype.h>
/* Jim Meyering writes:

   "... Some ctype macros are valid only for character codes that
   isascii says are ASCII (SGI's IRIX-4.0.5 is one such system --when
   using /bin/cc or gcc but without giving an ansi option).  So, all
   ctype uses should be through macros like ISPRINT...  If
   STDC_HEADERS is defined, then autoconf has verified that the ctype
   macros don't need to be guarded with references to isascii. ...
   Defining isascii to 1 should let any compiler worth its salt
   eliminate the && through constant folding."  */

# if defined (STDC_HEADERS) || (!defined (isascii) && !defined (HAVE_ISASCII))
#  define ISASCII(c) 1
# else
#  define ISASCII(c) isascii((int) c)
# endif

# ifdef isblank
#  define ISBLANK(c) (ISASCII (c) && isblank ((int) c))
# else
#  define ISBLANK(c) ((c) == ' ' || (c) == '\t')
# endif

# ifdef isgraph
#  define ISGRAPH(c) (ISASCII (c) && isgraph ((int) c))
# else
#  define ISGRAPH(c) (ISASCII (c) && isprint ((int) c) && !isspace ((int) c))
# endif

# define ISPRINT(c) (ISASCII (c) && isprint   ((int) c))
# define ISDIGIT(c) (ISASCII (c) && isdigit   ((int) c))
# define ISALNUM(c) (ISASCII (c) && isalnum   ((int) c))
# define ISALPHA(c) (ISASCII (c) && isalpha   ((int) c))
# define ISCNTRL(c) (ISASCII (c) && iscntrl   ((int) c))
# define ISLOWER(c) (ISASCII (c) && islower   ((int) c))
# define ISPUNCT(c) (ISASCII (c) && ispunct   ((int) c))
# define ISSPACE(c) (ISASCII (c) && isspace   ((int) c))
# define ISUPPER(c) (ISASCII (c) && isupper   ((int) c))
# define ISXDIGIT(c) (ISASCII (c) && isxdigit ((int) c))

/* limits */
# if HAVE_LIMITS_H
#  include <limits.h>
# endif
/* some systems (at least mine) do not define properly the following limits.... */
# ifndef FLT_MIN
#  define FLT_MIN 1.17549435E-38F
# endif
# ifndef FLT_MAX
#  define FLT_MAX 3.40282347e+20F
# endif
# ifndef DBL_MIN
#  define DBL_MIN 2.2250738585072014E-308
# endif
# ifndef DBL_MAX
#  define DBL_MAX 1.7976931348623157E+308
# endif

# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif

/* time stuff */
# if TIME_WITH_SYS_TIME
#  include <time.h>
#  include <sys/time.h>
# else 
#  ifdef HAVE_SYS_TIME_H
#    include <sys/time.h>
#  else
#    include <time.h>
#  endif
# endif

#endif /* _system_h_ */
