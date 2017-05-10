/* mksublogid */

/* make a sub (or serial) log-ID */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine makes a composite (also called a "serial") log-ID for
	the PCSPOLL program (or other programs that have sub-jobs associated
	with them and where they require a prefix log ID).  A composite log-id
	has the form:
		<base>.<serial>
	where:
		<base>		is a base-string
		<serial>	is a serial-string (string of digits)

	The serial part is a maximum of six digits.  The base-string is
	adjusted to fit until it reaches a minimum of three characters.  The
	serial-string is then adjusted down to a minimum before failure is
	returned.

	Synopsis:

	int mksublogid(dbuf,dlen,bname,v)
	char		dbuf[] ;
	int		dlen ;
	const char	bname[] ;
	int		v ;

	Arguments:

	dbuf		destination buffer (for result)
	dlen		length of destination buffer (desired length)
	bname		base-name string
	v		value

	Returns:

	<0		bad
	>=0		length of resulting string


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ctdec.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */

#define	DEFLOGIDLEN	15		/* same as LOGIDLEN? */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define	MINBAS		3		/* minimum base-name characters */
#define	MINDIG		2		/* minimum */
#define	MIDBAS		4		/* minimum base-name characters */
#define	MIDDIG		5		/* minimum */
#define	MAXBAS		5
#define	MAXDIG		6		/* same as PID_MAX? */

#define	MIDDLECHAR	'.'


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsdd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	cfdeci(const char *,int,int *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	mkjoin(char *,int,const char *,int,const char *,int) ;


/* local variables */


/* exported subroutines */


int mksublogid(char *dbuf,int dlen,cchar *bname,int v)
{
	int		rs = SR_OK ;
	int		max ;
	int		al ;
	int		bl ;
	int		vl = 0 ;
	const char	*bp ;
	const char	*vp ;
	char		vbuf[DIGBUFLEN + 1] ;

	if (dbuf == NULL) return SR_FAULT ;
	if (bname == NULL) return SR_FAULT ;

	if (v < 0) return SR_DOM ;

	if (dlen < 0) dlen = DEFLOGIDLEN ;

	vbuf[0] = '\0' ;
	vp = vbuf ;
	if (v >= 0) {
	    rs = ctdeci(vbuf,DIGBUFLEN,v) ;
	    vl = rs ;
	}

#if	CF_DEBUGS
	debugprintf("mksublogid: ctdeci() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    bp = bname ;
	    bl = strlen(bname) ;

/* start possible various size reductions */

	    if ((bl + 1 + vl) > dlen) {
	        max = MAXDIG ;
	        if (vl > max) {
	            al = (vl - max) ;
	            vp += al ;
	            vl -= al ;
	        }
	    } /* end if */

	    if ((bl + 1 + vl) > dlen) {
	        max = MAXBAS ;
	        if (bl > max) {
	            al = (bl - max) ;
	            bl -= al ;
	        }
	    } /* end if */

	    if ((bl + 1 + vl) > dlen) {
	        max = MIDDIG ;
	        if (vl > max) {
	            al = (vl - max) ;
	            vp += al ;
	            vl -= al ;
	        }
	    } /* end if */

	    if ((bl + 1 + vl) > dlen) {
	        max = MIDBAS ;
	        if (bl > max) {
	            al = (bl - max) ;
	            bl -= al ;
	        }
	    } /* end if */

	    if ((bl + 1 + vl) > dlen) {
	        max = MINDIG ;
	        if (vl > max) {
	            al = (vl - max) ;
	            vp += al ;
	            vl -= al ;
	        }
	    } /* end if */

	    if ((bl + 1 + vl) > dlen) {
	        max = MINBAS ;
	        if (bl > max) {
	            al = (bl - max) ;
	            bl -= al ;
	        }
	    } /* end if */

	    rs = mkjoin(dbuf,dlen,bp,bl,vp,vl) ;

	} /* end if (ok) */

	return rs ;
}
/* end subroutine (mksublogid) */


/* local subroutines */


static int mkjoin(char *dbuf,int dlen,cchar *bp,int bl,cchar *vp,int vl)
{
	int		rs ;
	int		i = 0 ;

	if ((rs = storebuf_strw(dbuf,dlen,i,bp,bl)) >= 0) {
	    i += rs ;
	    if ((vl >= 0) && (vp[0] != '\0')) {
	        if ((rs = storebuf_char(dbuf,dlen,i,MIDDLECHAR)) >= 0) {
	    	    i += rs ;
	            rs = storebuf_strw(dbuf,dlen,i,vp,vl) ;
	            i += rs ;
		}
	    } /* end if (trailing part) */
	} /* end if (base part) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkjoin) */


