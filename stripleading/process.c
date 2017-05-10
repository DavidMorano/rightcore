/* process */

/* process a file */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* time-time debug print-outs */


/* revision history:

	= 96/03/01, David A­D­ Morano

	The subroutine was adapted from others programs that
	did similar types of functions.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This module just provides optional expansion of directories.
	The real work is done by the 'checkname' module.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<char.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NCOLSPERTAB	8		/* columns per tab-stop */


/* external subroutines */

extern int	ifloor(int,int) ;


/* external variables */


/* local structures */

struct expander {
	char		*linebuf ;
	int		linelen ;
} ;


/* forward references */

static int	process_exp(struct proginfo *,struct expander *,const char *) ;
static int	shaver(struct proginfo *,const char *,int,const char **) ;

static int	expander_start(struct expander *,int) ;
static int	expander_expand(struct expander *,const char *,int,
			const char **) ;
static int	expander_finish(struct expander *) ;


/* local variables */


/* exported subroutines */


int process(pip,fname)
struct proginfo	*pip ;
const char	fname[] ;
{
	struct expander	e ;

	const int	elen = (LINEBUFLEN + NCOLSPERTAB + 1) ;

	int	rs ;
	int	wlen = 0 ;


	if ((rs = expander_start(&e,elen)) >= 0) {

	    rs = process_exp(pip,&e,fname) ;
	    wlen = rs ;

	    expander_finish(&e) ;
	} /* end if (expander) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


/* local subroutines */


static int process_exp(pip,exp,fname)
struct proginfo	*pip ;
struct expander	*exp ;
const char	fname[] ;
{
	bfile	ifile, *ifp = &ifile ;

	int	rs ;
	int	ll, len ;
	int	wlen = 0 ;
	int	f_bol, f_eol ;

	const char	*lp ;

	char	linebuf[LINEBUFLEN + 1] ;


	if (fname == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("process: fname=%s\n",fname) ;
	    debugprintf("process: progmode=%u\n",pip->progmode) ;
	}
#endif

	if ((fname[0] != '\0') && (fname[0] != '-')) {
	    rs = bopen(ifp,fname,"r",0666) ;

	} else
	    rs = bopen(ifp,BFILE_STDIN,"r",0666) ;

	if (rs >= 0) {

	    f_bol = TRUE ;
	    while ((rs = breadline(ifp,linebuf,LINEBUFLEN)) > 0) {
	        len = rs ;

	        f_eol = (linebuf[len - 1] == '\n') ;

	        lp = linebuf ;
	        ll = len ;
	        if (f_bol) {

	            switch (pip->progmode) {

	            case progmode_stripleading:
	                lp = linebuf ;
	                while (CHAR_ISWHITE(*lp) && (*lp != '\n'))
	                    lp += 1 ;
	                ll = len - (lp - linebuf) ;
	                break ;

	            case progmode_shave:
	                ll = shaver(pip,linebuf,len,&lp) ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("process: shaver() rs=%d\n",ll) ;
#endif
	                if (ll < 0) {
	                    rs = expander_expand(exp,linebuf,len,&lp) ;
	                    ll = rs ;
#if	CF_DEBUG
	                    if (DEBUGLEVEL(2))
	                        debugprintf("process: expanded ll=%d >%t<\n",
	                            ll,lp,((ll > 0) ? (ll - 1) : ll)) ;
#endif
	                    if (rs > 0)
	                        ll = shaver(pip,lp,ll,&lp) ;
	                }
	                break ;

	            } /* end if (switch) */

	        } /* end if */

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process: writing ll=%u\n",ll) ;
#endif

	        rs = bwrite(pip->ofp,lp,ll) ;
	        wlen += rs ;
	        if (rs < 0)
	            break ;

	        f_bol = f_eol ;
	    } /* end while */

	    bclose(ifp) ;
	} /* end if (opened file) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process_exp) */


static int shaver(pip,lbuf,llen,rpp)
struct proginfo	*pip ;
const char	lbuf[] ;
int		llen ;
const char	**rpp ;
{
	int	rs = SR_OK ;
	int	i ;
	int	len = 0 ;

	const char	*lp = lbuf ;


	for (i = 0 ; (i < pip->shave) && (i < llen) ; i += 1) {

	    if (*lp == '\t') {
	        rs = SR_INVALID ;
	        break ;
	    }

	    if (*lp == '\n')
	        break ;

	    if (! CHAR_ISWHITE(*lp))
	        break ;

	    lp += 1 ;

	} /* end for */

	if (rpp != NULL)
	    *rpp = lp ;

	len = (lbuf+llen) - lp ;
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (shaver) */


static int expander_start(op,elen)
struct expander	*op ;
int		elen ;
{
	int	rs ;
	int	size = (elen + 1) ;

	char	*p ;


	if (op == NULL) return SR_FAULT ;
	if (elen < 0) return SR_INVALID ;

	rs = uc_malloc(size,&p) ;
	if (rs >= 0) {
	    op->linebuf = p ;
	    op->linelen = elen ;
	}

	return 0 ;
}
/* end subroutine (expander_start) */


static int expander_expand(op,linebuf,linelen,rpp)
struct expander	*op ;
const char	linebuf[] ;
int		linelen ;
const char	**rpp ;
{
	int	rs = SR_OK ;
	int	n, i, j ;
	int	len ;

	char	*sp, *overlastp ;


	sp = op->linebuf ;
	overlastp = op->linebuf + LINEBUFLEN ;
	for (i = 0 ; (i < linelen) && (sp < overlastp) ; i += 1) {

	    if (linebuf[i] == '\t') {

	        n = ifloor((i + NCOLSPERTAB),NCOLSPERTAB) - i ;

#if	CF_DEBUGS
	        debugprintf("expander_expand: n=%u\n",n) ;
#endif

	        for (j = 0 ; (j < n) && (sp < overlastp) ; j += 1)
	            *sp++ = ' ' ;

	    } else
	        *sp++ = linebuf[i] ;

	} /* end for */

	if (rpp != NULL)
	    *rpp = op->linebuf ;

	len = (sp - op->linebuf) ;
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (expander_expand) */


static int expander_finish(op)
struct expander	*op ;
{


	if (op->linebuf != NULL) {
	    uc_free(op->linebuf) ;
	    op->linebuf = NULL ;
	}

	op->linelen = 0 ;
	return SR_OK ;
}
/* end subroutine (expander_finish) */



