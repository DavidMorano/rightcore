/* shio_print */

/* the SHell-IO hack (print) */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine is the simple 'print' that other I-O packages have 
	already.


******************************************************************************/


#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<dlfcn.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"shio.h"


/* local defines */

#undef	COMMENT

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;

extern int	format(char *,int,int,const char *,va_list) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* forward references */


/* local variables */


/* exported subroutines */


int shio_print(SHIO *fp,cchar *lbuf,int llen)
{
	int	rs = SR_OK ;
	int	wlen = 0 ;


	if (llen < 0)
	    llen = strlen(lbuf) ;

	if (llen > 0) {
	    rs = shio_write(fp,lbuf,llen) ;
	    wlen += rs ;
	}

	if ((rs >= 0) && ((llen == 0) || (lbuf[llen-1] != '\n'))) {
	    char	eol[2] ;
	    eol[0] = '\n' ;
	    eol[1] = '\0' ;
	    rs = shio_write(fp,eol,1) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (shio_print) */


