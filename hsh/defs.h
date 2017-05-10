/* main */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>


/* define stuff */

#define		BUFSIZE		1024
#define		LINESIZE	160
#define		SWIDTH		79
#define		NLINES		18		/* number visible on screen  */
#define		NLINEBUF	30

#define		CPP		"> "
#define		CPL		(sizeof(CPP) - 1)

#define		CBL		80

#define		HSIZE		800

#define		SEM_CY		0
#define		SEM_INT		1

#define		PARAM_OK	0
#define		PARAM_BAD	-1
#define		PARAM_EMPTY	-2



/* define the global data structures */

struct history {
	int	n, ri, wi, c ;
	char	buf[HSIZE] ;
} ;


struct linebuf {
	struct linebuf	*np ;
	struct linebuf	*pp ;
	int	len ;
	int	flags ;
	char	buf[LINESIZE] ;
} ;


struct linehead {
	struct linebuf	*hp ;	/* head */
	struct linebuf	*tp ;	/* tail */
} ;







struct gdata {
	bfile		*ifp, *ofp, *efp ;
	bfile		*tfp, *bfp ;
	struct linehead	alq, flq ;
	struct linebuf	*clp ;	/* current */
	struct linebuf	*slp ;	/* top screen */
	struct linebuf	*blp ;	/* bottom screen */
	struct history	*hp ;
	long		top_off, bot_off ;
	long		term_mode ;
	int		ifd, ofd, efd ;
	int		cur_row, cur_col ;
	char		*progname ;
} ;


#endif /* DEFS_INCLUDE */


