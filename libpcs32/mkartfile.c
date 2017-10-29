/* mkartfile */

/* make an article file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Make an article file.

	Synopsis:

	int mkartfile(rbuf,om,dname,prefix,serial)
	const char	rbuf[] ;
	mode_t		om ;
	const char	dname[] ;
	const char	prefix[] ;
	int		serial ;

	Arguments:

	rbuf		result buffer (must be MAXPATHLEN+1 in size)
	on		open mode
	dname		directory
	prefix		prefix to created file
	serial		a serial number

	Returns:

	>=0	success and length of created file name
	<0	failure w/ error number


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sigblock.h>
#include	<base64.h>
#include	<localmisc.h>


/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(c)	((c) & 0xff) ;
#endif

#undef	RANDBUFLEN
#define	RANDBUFLEN	16


/* external subroutines */

extern int	pathadd(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* forward reference */

static int	mktry(char *,int,const char *,uint,uint,uint,mode_t) ;
static int	mkoutname(char *,const char *,uint,uint,uint) ;


/* local variables */


/* exported subroutines */


int mkartfile(char *rbuf,mode_t om,cchar *dname,cchar *prefix,int serial)
{
	const time_t	dt = time(NULL) ;
	const mode_t	operm = (om & S_IAMB) ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("mkartfile: ent\n") ;
#endif

	if (rbuf == NULL) return SR_FAULT ;
	if (dname == NULL) return SR_FAULT ;
	if (prefix == NULL) return SR_FAULT ;

	if (dname[0] == '\0') return SR_INVALID ;
	if (prefix[0] == '\0') return SR_INVALID ;
	if (serial < 0) return SR_INVALID ;

	if ((rs = mkpath1(rbuf,dname)) >= 0) {
	    uint	ts ;
	    const int	rlen = rs ;
	    for (ts = dt ; rs >= 0 ; ts += 1) {
		uint	ss ;
	        for (ss = serial ; (rs >= 0) && (ss < 256) ; ss += 1) {
		    uint	es ;
		    for (es = 0 ; (rs >= 0) && (es < 4) ; es += 1) {
			rs = mktry(rbuf,rlen,prefix,ts,ss,es,operm) ;
		        if (rs > 0) break ;
			if (rs == SR_EXISTS) rs = SR_OK ;
		    } /* end for (end-stamp) */
		    if (rs > 0) break ;
	        } /* end for (serial-stamp) */
		serial = 0 ;
		if (rs > 0) break ;
	    } /* end for (time-stamp) */
	} /* end if (mkpath) */

#if	CF_DEBUGS
	debugprintf("mkartfile: ret rs=%d\n",rs) ;
	debugprintf("mkartfile: rbuf=%s\n",rbuf) ;
#endif

	return rs ;
}
/* end subroutine (mkartfile) */


/* local subroutines */


static int mktry(char *rp,int rl,cchar *pre,uint ts,uint ss,uint es,mode_t om)
{
	const int	of = (O_CREAT|O_EXCL|O_CLOEXEC) ;
	int		rs ;
	int		len = 0 ;
	char		tfname[MAXNAMELEN+1] ;

	if ((rs = mkoutname(tfname,pre,ts,ss,es)) >= 0) {
	    if ((rs = pathadd(rp,rl,tfname)) >= 0) {
		len = rs ;
	        if ((rs = u_open(rp,of,om)) >= 0) {
		    u_close(rs) ;
		}
	    } /* end if (path-add) */
	} /* end if (made art-filename) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mktry) */


static int mkoutname(char *rfname,cchar *pre,uint ts,uint ss,uint es)
{
	uint64_t	bits = 0 ;
	char		*bp = rfname ;

#if	CF_DEBUGS
	debugprintf("mkartfile/mkoutname: pre=%s\n",pre) ;
#endif

/* load up to 7 prefix characters into output buffer */

	bp = strwcpy(bp,pre,7) ;

/* load the 42 bits into the 'bits' variable */

	{
	    {
		bits |= (ts & UINT_MAX) ;
	    }
	    {
	        bits <<= 8 ;
	        bits |= (ss & 0xff) ;
	    }
	    {
	        bits <<= 2 ;
	        bits |= (es & 3) ;
	    }
	} /* end block (make bits) */

/* encode the 'bits' above into the output buffer using BASE-64 encoding */

	{
	    const int	n = 7 ; /* number of chars (7x6=42 bits) */
	    int		i ;
	    int		ch ;
	    for (i = (n-1) ; i >= 0 ; i -= 1) {
	        int	bi = (bits & 63) ;
	        ch = base64_et[bi] ;
	        if (ch == '+') {
		    ch = 'Þ' ;
		} else if (ch == '/') {
		    ch = 'þ' ;
		}
	        bp[i] = ch ;
	        bits >>= 6 ;
	    } /* end for */
	    bp += n ;
	}

/* done */

	*bp = '\0' ;

#if	CF_DEBUGS
	debugprintf("mkartfile/mkoutname: rbuf=%s\n",rfname) ;
#endif

	return (bp - rfname) ;
}
/* end subroutine (mkoutname) */


