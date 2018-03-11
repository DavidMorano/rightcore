/* linecenter */

/* text fill (for line-centering) */


/* revision history:

	= 2008-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	LINECENTER_INCLUDE
#define	LINECENTER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>		/* base types */

#include	<fifostr.h>
#include	<localmisc.h>		/* extra types */


#define	LINECENTER_MAGIC	0x88442239
#define	LINECENTER_DEFLINES	7
#define	LINECENTER_FNLEN	3
#define	LINECENTER_DEFPERCENT	0.60
#define	LINECENTER		struct linecenter_head
#define	LINECENTER_CENTER	struct linecenter_center


struct linecenter_center {
	uint		linelen ;
	uint		linebrk ;
} ;

struct linecenter_head {
	uint		magic ;
	const char	**lines ;
	fifostr		sq ;
	int		li ;		/* line-index */
	int		le ;		/* line-extent */
	int		wc ;		/* word-count */
	int		cc ;		/* character count (w/ blanks) */
	char		fn[LINECENTER_FNLEN + 1] ;
} ;


#if	(! defined(LINECENTER_MASTER)) || (LINECENTER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	linecenter_start(LINECENTER *,const char *) ;
extern int	linecenter_addword(LINECENTER *,const char *,int) ;
extern int	linecenter_addline(LINECENTER *,const char *,int) ;
extern int	linecenter_addlines(LINECENTER *,const char *,int) ;
extern int	linecenter_mklinefull(LINECENTER *,char *,int) ;
extern int	linecenter_mklinepart(LINECENTER *,char *,int) ;
extern int	linecenter_count(LINECENTER *) ;
extern int	linecenter_mklines(LINECENTER *,int,int) ;
extern int	linecenter_getline(LINECENTER *,int,const char **) ;
extern int	linecenter_finish(LINECENTER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* LINECENTER_MASTER */

#endif /* LINECENTER_INCLUDE */


