/* shio_printdline */

/* print a double-high double-wide line */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine prints a double-high double-wide line.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"shio.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	ESBUFLEN	20

#define	ISCONT(b,bl)	\
	(((bl) >= 2) && ((b)[(bl) - 1] == '\n') && ((b)[(bl) - 2] == '\\'))


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	msleep(int) ;
extern int	termescseq(char *,int,int,int,int,int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* forward references */

static int	shio_printdpart(void *,int,int,const char *,int) ;


/* local variables */


/* exported subroutines */


int shio_printdline(ofp,lbuf,llen)
void		*ofp ;
const char	lbuf[] ;
int		llen ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;


	if (llen < 0) llen = strlen(lbuf) ;

	while (llen && (lbuf[llen-1] == '\n')) llen -= 1 ;

	if (rs >= 0) {
	    rs = shio_printdpart(ofp,'6','3',lbuf,llen) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = shio_printdpart(ofp,'6','4',lbuf,llen) ;
	    wlen += rs ;
	}

#if	CF_DEBUGS
	    debugprintf("shio_printdline: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (shio_printdline) */


/* local subroutines */


static int shio_printdpart(ofp,f1,f2,lbuf,llen)
void		*ofp ;
int		f1 ;
int		f2 ;
const char	lbuf[] ;
int		llen ;
{
	int	rs = SR_OK ;
	int	eslen = 0 ;
	int	amount ;
	int	wlen = 0 ;

	char	esbuf[ESBUFLEN+1], *ebp = esbuf ;


#ifdef	COMMENT
	if (llen < 0) llen = strlen(lbuf) ;
#endif

	if (rs >= 0) {
	    rs = termescseq(ebp,8,f1,'#',-1,-1,-1) ;
	    ebp += rs ;
	}

	if (rs >= 0) {
	    rs = termescseq(ebp,8,f2,'#',-1,-1,-1) ;
	    ebp += rs ;
	}

	eslen = (ebp-esbuf) ;
	amount = (llen + eslen + 1) ;
	if (rs >= 0)
	    rs = shio_reserve(ofp,amount) ;

	if (rs >= 0) {
	    rs = shio_write(ofp,esbuf,eslen) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = shio_print(ofp,lbuf,llen) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (shio_printdpart) */



