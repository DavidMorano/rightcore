/* mailmsghdrfold */

/* manage folding of a line */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2009-04-10, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object module takes a line of text as input and breaks
	it up into pieces that are folded so as to fit in a specified
	number of print-output columns.

	Synopsis:

	int mailmsghdrfold_get(op,mcols,ncol,rpp)
	MAILMSGHDRFOLD	*op ;
	int		mcols, ncol ;
	const char	**rpp ;

	Arguments:

	op		object pointer
	mcols		number of total columns available for this line
	ncol		current column number (from beginning of line)
	rpp		pointer to resulting line

	Returns:

	<0		bad
	==0		done
	>0		length of line


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<ascii.h>
#include	<char.h>
#include	<localmisc.h>

#include	"mailmsghdrfold.h"


/* local defines */

#define	MAILMSGHDRFOLD_MAGIC	0x88431773

#undef	LINER
#define	LINER		struct liner

#undef	PARAMS
#define	PARAMS		struct params

#ifndef	VARCOLUMNS
#define	VARCOLUMNS	"COLUMNS"
#endif

#ifndef	COLUMNS
#define	COLUMNS		80		/* output cols (should be 80) */
#endif

#ifndef	NTABCOLS
#define	NTABCOLS	8
#endif


/* external subroutines */

extern int	charcols(int,int,int) ;
extern int	iceil(int,int) ;
extern int	cfdeci(const char *,int,int *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */

static int	nextpiece(int,const char *,int,int *) ;
static int	isend(int) ;


/* local variables */


/* exported subroutines */


int mailmsghdrfold_start(op,mcols,ln,sp,sl)
MAILMSGHDRFOLD	*op ;
const char	sp[] ;
int		sl ;
int		mcols ;
int		ln ;
{
	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	if (sp == NULL)
	    return SR_FAULT ;

	memset(op,0,sizeof(MAILMSGHDRFOLD)) ;
	op->mcols = mcols ;
	op->ln = ln ;

	if (sl < 0)
	    sl = strlen(sp) ;

	while ((sl >= 0) && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}

	while ((sl > 0) && isend(sp[sl-1]))
	    sl -= 1 ;

#if	CF_DEBUGS
	debugprintf("mailmsghdrfold_start: sl=%d sp=>%t<\n",
		sl,sp,strlinelen(sp,sl,40)) ;
#endif

	op->sp = sp ;
	op->sl = sl ;

	op->magic = MAILMSGHDRFOLD_MAGIC ;

	return rs ;
}
/* end subroutine (mailmsghdrfold_start) */


int mailmsghdrfold_finish(op)
MAILMSGHDRFOLD	*op ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != MAILMSGHDRFOLD_MAGIC)
	    return SR_NOTOPEN ;

	op->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (mailmsghdrfold_finish) */


int mailmsghdrfold_get(op,ncol,rpp)
MAILMSGHDRFOLD	*op ;
int		ncol ;
const char	**rpp ;
{
	int	rs = SR_OK ;
	int	mcols ;
	int	nc ;
	int	ncs = 0 ;
	int	pl ;
	int	sl ;
	int	ll = 0 ;
	int	f_big ;

	const char	*sp ;
	const char	*lp = NULL ;


	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILMSGHDRFOLD_MAGIC) return SR_NOTOPEN ;

	if (mcols < 1) return SR_INVALID ;
	if (ncol < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("mailmsghdrfold_get: mcols=%u ccol=%u\n",mcols,ncol) ;
#endif

	mcols = op->mcols ;
	sp = op->sp ;
	sl = op->sl ;

	while ((ll == 0) && (sl > 0)) {

#if	CF_DEBUGS
	debugprintf("mailmsghdrfold_get: sl=%d sp=>%t<\n",
		sl,sp,strlinelen(sp,sl,40)) ;
#endif

/* move up to the first non-whitespace character */

	    while (sl && CHAR_ISWHITE(sp[0])) {
	        sp += 1 ;
	        sl -= 1 ;
	    }

	    lp = sp ;
	    if (rpp != NULL)
	        *rpp = (char *) sp ;

/* continue */

	    f_big = FALSE ;
	    nc = ncol ;
	    while ((pl = nextpiece(nc,sp,sl,&ncs)) > 0) {

#if	CF_DEBUGS
	debugprintf("mailmsghdrfold_get: nc=%u ncs=%u\n",nc,ncs) ;
	debugprintf("mailmsghdrfold_get: pl=%d piece=>%t<\n",
		pl,sp,strlinelen(sp,pl,40)) ;
#endif

	        if ((nc + ncs) >= mcols) {
		    if (ll == 0) {
			f_big = TRUE ;
			ll = pl ;
		    }
	            break ;
		}

	        ll += pl ;
	        nc += ncs ;

	        sp += pl ;
	        sl -= pl ;

	    } /* end while */

	    if (f_big) {
	        sp += pl ;
	        sl -= pl ;
	    }

#if	CF_DEBUGS
	debugprintf("mailmsghdrfold_get: line=>%t<\n",
		lp,strlinelen(lp,ll,40)) ;
#endif

	    while (sl && CHAR_ISWHITE(sp[0])) {
	        sp += 1 ;
	        sl -= 1 ;
	    }

	    if ((sl > 0) && (sp[0] == '\n')) {
	        sp += 1 ;
	        sl -= 1 ;
	    }

	} /* end while */

	op->sp = sp ;
	op->sl = sl ;

#if	CF_DEBUGS
	debugprintf("mailmsghdrfold_get: line=>%t<\n",
		lp,strlinelen(lp,ll,50)) ;
	debugprintf("mailmsghdrfold_get: ret rs=%d ll=%u\n", rs,ll) ;
#endif

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (mailmsghdrfold_get) */


/* private subroutines */


static int nextpiece(ncol,sp,sl,ncp)
int		ncol ;
const char	*sp ;
int		sl ;
int		*ncp ;
{
	int	ncs = 0 ;
	int	cl ;
	int	n ;
	int	pl = 0 ;

	const char	*cp ;


	cp = sp ;
	cl = sl ;

/* skip over whitespace */

	while (cl && CHAR_ISWHITE(cp[0])) {
	    n = charcols(NTABCOLS,ncol,cp[0]) ;
	    cp += 1 ;
	    cl -= 1 ;
	    ncs += n ;
	    ncol += n ;
	} /* end while */

/* skip over the non-whitespace */

	while (cl && cp[0] && (! CHAR_ISWHITE(cp[0]))) {
	    n = charcols(NTABCOLS,ncol,cp[0]) ;
	    cp += 1 ;
	    cl -= 1 ;
	    ncs += n ;
	    ncol += n ;
	} /* end while */

/* done */

	*ncp = ncs ;
	pl = (cp - sp) ;

#if	CF_DEBUGS
	debugprintf("mailmsghdrfold/nextpiece: ret pl=%u ncs=%u\n",pl,ncs) ;
#endif

	return pl ;
}
/* end subroutine (nextpiece) */


static int isend(ch)
int	ch ;
{
	return ((ch == '\n') || (ch == '\r')) ;
}
/* end subroutine (isend) */



