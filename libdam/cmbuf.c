/* cmbuf */

/* Connection Maager Buffer */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was adopted for use from the DWD program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a communications connection manager object.  This object
	abstracts the details of a particular connection from the calling
	program.


*******************************************************************************/


#define	CMBUF_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"cmbuf.h"


/* local defines */

#define	CMBUF_MAGIC	31815926

#ifndef	ARGSBUFLEN
#define	ARGSBUFLEN	((6 * MAXPATHLEN) + 35)
#endif

#ifndef	ARGBUFLEN
#define	ARGBUFLEN	((2 * MAXPATHLEN) + 20)
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */


/* external variables */


/* forward references */

static int	isend(int) ;


/* local variables */


/* exported subroutines */


int cmbuf_start(bdp,buf,buflen)
CMBUF		*bdp ;
const char	buf[] ;
int		buflen ;
{


	if (bdp == NULL)
	    return SR_FAULT ;

	if (buf == NULL)
	    return SR_FAULT ;

	if (buflen <= 0)
	    return SR_INVALID ;

	bdp->buf = (char *) buf ;
	bdp->bp = (char *) buf ;
	bdp->bl = 0 ;
	bdp->buflen = buflen ;
	bdp->magic = CMBUF_MAGIC ;
	return 0 ;
}
/* end subroutine (cmbuf_start) */


int cmbuf_space(bdp,bdrp)
CMBUF		*bdp ;
CMBUF_SPACE	*bdrp ;
{


	if (bdp == NULL)
	    return SR_FAULT ;

	if (bdp->magic != CMBUF_MAGIC)
	    return SR_NOTOPEN ;

	if (bdrp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("cmbuf_space: 1 buflen=%d bl=%d\n",bdp->buflen,bdp->bl) ;
#endif

	if (bdp->bl > 0)
	    memmove(bdp->buf,bdp->bp,bdp->bl) ;

	bdp->bp = bdp->buf ;

#if	CF_DEBUGS
	debugprintf("cmbuf_space: 2 buflen=%d bl=%d\n",bdp->buflen,bdp->bl) ;
#endif

	bdrp->bp = bdp->buf + bdp->bl ;
	bdrp->bl = bdp->buflen - bdp->bl ;

#if	CF_DEBUGS
	debugprintf("cmbuf_space: ret bl=%d\n",bdrp->bl) ;
#endif

	return bdrp->bl ;
}
/* end subroutine (cmbuf_space) */


int cmbuf_added(bdp,len)
CMBUF		*bdp ;
int		len ;
{


	if (bdp == NULL)
	    return SR_FAULT ;

	if (bdp->magic != CMBUF_MAGIC)
	    return SR_NOTOPEN ;

	bdp->bl += len ;
	return bdp->bl ;
}
/* end subroutine (cmbuf_added) */


int cmbuf_getline(bdp,llen,lpp)
CMBUF		*bdp ;
int		llen ;
const char	**lpp ;
{
	int	rs = SR_OK ;
	int	bl ;
	int	maxll ;
	int	len = 0 ;
	int	f_eol = FALSE ;
	int	f_again = TRUE ;

	const char	*sbp, *ebp ;

	char	*bp ;


	if (bdp == NULL)
	    return SR_FAULT ;

	if (bdp->magic != CMBUF_MAGIC)
	    return SR_NOTOPEN ;

	if (lpp == NULL)
	    return SR_FAULT ;

	if (llen < 0) return SR_INVALID ;

	maxll = MIN(llen,bdp->buflen) ;

#if	CF_DEBUGS
	debugprintf("cmbuf_getline: llen=%d maxll=%u\n",llen,maxll) ;
#endif

	bp = bdp->bp ;
	bl = bdp->bl ;

#if	CF_DEBUGS
	debugprintf("cmbuf_getline: bl=%u\n",bl) ;
#endif

	*lpp = bdp->bp ;
	sbp = bdp->bp ;
	ebp = (bdp->bp + bl) ;
	while ((bp < ebp) && ((bp - sbp) < maxll)) {

	    f_eol = isend(bp[0]) ;
	    bp += 1 ;
	    if (f_eol) break ;

	} /* end while */

#if	CF_DEBUGS
	debugprintf("cmbuf_getline: f_eol=%u\n",f_eol) ;
#endif

	len = (bp - sbp) ;
	if (f_eol || (len == maxll))
	    f_again = FALSE ;

	if ((! f_again) && (bp < ebp)) {
	    if ((! f_eol) && isend(bp[0])) {
		bp += 1 ;
		len += 1 ;
	    }
	    if ((len > 0) && (bp[-1] == '\r') && 
		(bp < ebp) && (bp[0] == '\n')) {
		    bp += 1 ;
		    len += 1 ;
	    }
	} /* end if */

	if (f_again) {
	    rs = SR_AGAIN ;
	} else {
	    bdp->bl -= len ;
	    bdp->bp = bp ;
	}

#if	CF_DEBUGS
	debugprintf("cmbuf_getline: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (cmbuf_getline) */


int cmbuf_getlastline(bdp,lpp)
CMBUF		*bdp ;
const char	**lpp ;
{
	int	rs = SR_OK ;
	int	len = 0 ;


	if (bdp == NULL)
	    return SR_FAULT ;

	if (bdp->magic != CMBUF_MAGIC)
	    return SR_NOTOPEN ;

	len = bdp->bl ;
	if (lpp != NULL)
	    *lpp = bdp->bp ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (cmbuf_getlastline) */


int cmbuf_finish(bdp)
CMBUF		*bdp ;
{


	if (bdp == NULL)
	    return SR_FAULT ;

	if (bdp->magic != CMBUF_MAGIC)
	    return SR_NOTOPEN ;

	bdp->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (cmbuf_finish) */


/* local subroutines */


static int isend(ch)
int	ch ;
{
	int	f ;


	f = (ch == '\n') || (ch == '\r') ;

	return f ;
}
/* end subroutine (isend) */



