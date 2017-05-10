/* hdrctype */

/* process the input messages and spool them up */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/****************************************************************************

        This object module parses a "content-type" header specification. The
        parsed results are broken into three types of items: the type, the
        sub-type, and parameters.


****************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"hdrctype.h"


/* local defines */

#define	HDRCTYPE_MAGIC		0x53232857

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	HDRNAMELEN
#define	HDRNAMELEN	80
#endif

#ifndef	MSGLINELEN
#define	MSGLINELEN	(2 * 1024)
#endif

#ifndef	MAXMSGLINELEN
#define	MAXMSGLINELEN	76
#endif

#undef	CHAR_TOKSEP
#define	CHAR_TOKSEP(c)	(CHAR_ISWHITE(c) || (! isprintlatin(c)))


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	hasuc(const char *,int) ;
extern int	isprintlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int hdrctype_decode(ctp,hp,hl)
HDRCTYPE	*ctp ;
const char	*hp ;
int		hl ;
{
	int	rs = SR_OK ;
	int	sl, cl ;

	const char	*tp ;
	const char	*sp ;
	const char	*cp ;


	memset(ctp,0,sizeof(HDRCTYPE)) ;

	if (hl < 0) hl = strlen(hp) ;

/* ignore any parameters */

	if ((tp = strnchr(hp,hl,';')) != NULL)
	    hl = ((hp+hl) - tp) ;

/* parse the type and subtype */

	if ((tp = strnchr(hp,hl,'/')) != NULL) {
	    sp = (tp+1) ;
	    sl = ((hp+hl) - (tp+1)) ;
	    if ((cl = sfshrink(sp,sl,&cp)) > 0) {
		ctp->sub.tp = cp ;
		ctp->sub.tl = cl ;
	    }
	    hl = (tp-hp) ;
	}

	if ((cl = sfshrink(hp,hl,&cp)) > 0) {
	    ctp->main.tp = cp ;
	    ctp->main.tl = cl ;
	}

	return rs ;
}
/* end subroutine (hdrctype_decode) */


/* private subroutines */



