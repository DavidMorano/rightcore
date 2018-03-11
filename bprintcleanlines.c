/* bprintcleanlines */

/* print a clean (cleaned up) line of text */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_LINEFOLD	1		/* use 'linefold(3dam)' */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine prints out a cleaned up line of text.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"linefold.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	COLUMNS
#define	COLUMNS		80		/* output columns (should be 80) */
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	bprintcleanline(bfile *,const char *,int) ;
extern int	isprintlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strcpylc(char *,const char *) ;
extern char	*strcpyuc(char *,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	bprintcleanliner(bfile *,int,const char *,int) ;
static int	isend(int) ;


/* module global variables */


/* local variables */


/* exported subroutines */


int bprintcleanlines(ofp,linelen,lp,ll)
bfile	*ofp ;
int	linelen ;
char	lp[] ;
int	ll ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (ofp == NULL) return SR_FAULT ;
	if (lp == NULL) return SR_FAULT ;

	if (ofp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	if (ofp->f.nullfile) goto ret0 ;

/* continue */

	if (linelen <= 0)
	    linelen = COLUMNS ;

	if (ll < 0)
	    ll = strlen(lp) ;

#if	CF_DEBUGS
	debugprintf("bprintcleanlines: ent linelen=%u ll=%u\n",
	    linelen,ll) ;
#endif /* CF_DEBUGS */

#if	CF_LINEFOLD
	{
	    LINEFOLD	lf ;
	    int		sl ;
	    const char	*sp ;
	    if ((rs = linefold_start(&lf,linelen,0,lp,ll)) >= 0) {
		int	i ;
	        for (i = 0 ; (sl = linefold_get(&lf,i,&sp)) >= 0 ; i += 1) {
#if	CF_DEBUGS
	            debugprintf("bprintcleanlines: linefold_get() sl=%d\n",sl) ;
	            debugprintf("bprintcleanlines: line=>%t<\n",
	                sp,strlinelen(sp,sl,40)) ;
#endif
	            rs = bprintcleanliner(ofp,linelen,sp,sl) ;
	            wlen += rs ;
	            if (rs < 0) break ;
	        } /* end while */
	        linefold_finish(&lf) ;
	    } /* end if (linefold) */
	}
#else /* CF_LINEFOLD */
	{
	rs = bprintcleanliner(ofp,linelen,lp,ll) ;
	wlen += rs ;
	}
#endif /* CF_LINEFOLD */

ret0:

#if	CF_DEBUGS
	debugprintf("bprintcleanlines: ret rs=%d wlen=%u\n",
	    rs,wlen) ;
#endif /* CF_DEBUGS */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bprintcleanlines) */


/* local subroutines */


static int bprintcleanliner(ofp,linelen,lp,ll)
bfile		*ofp ;
int		linelen ;
const char	*lp ;
int		ll ;
{
	int		rs = SR_OK ;
	int		ml ;
	int		wlen = 0 ;
	int		f_end = FALSE ;

	while ((ll > 0) && isend(lp[ll - 1])) {
	    f_end = TRUE ;
	    ll -= 1 ;
	}

	while ((rs >= 0) && ((ll > 0) || f_end)) {

	    f_end = FALSE ;
	    ml = MIN(ll,linelen) ;

#ifdef	COMMENT /* what is this? */
	    if ((ml < ll) && isend(lp[ml-1])) {
	        ml += 1 ;
	        if ((lp[-1] == '\r') && (ml < ll) && (lp[0] == '\n')) {
	            ml += 1 ;
	        }
	    }
#endif /* COMMENT */

#if	CF_DEBUGS
	    debugprintf("bprintcleanlines: "
	        "bprintcleanline() ml=%u\n",ml) ;
#endif /* CF_DEBUGS */

	    rs = bprintcleanline(ofp,lp,ml) ;
	    wlen += rs ;

	    lp += ml ;
	    ll -= ml ;

	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bprintcleanliner) */


static int isend(int ch)
{
	int		f ;
	f = (ch == '\n') || (ch == '\r') ;
	return f ;
}
/* end subroutine (isend) */


