/* nprintf */

/* 'Named File' printf subroutine */
/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine performs a 'printf' like function but to the named file
	which is passed as the first argument.

	Synopsis:

	int nprintf(fname,fmt,...)
	const char	fname[] ;
	const char	fmt[] ;
	...		

	Arguments:

	filename	file to print to
	format		standard format string
	...		enverything else

	Returns:

	>=0		length of data (in bytes) written to file
	<0		failure


	+ Notes:

	Q. Does this subroutine have to be multi-thread-safe?
	A. In short, of course!

	Q. What do we not hve to place a mutex lock around the
	|write(2)| subroutine?
        A. Because we think that because we open the file a-fresh, getting a
        unique file-pointer, we *think* that the |write(2)| shoule be atomic,
        thus making this subroutine multi-thread-safe.


	+ Note on locking:

	There is no problem using (for example) |uc_lockf(3uc)| for establishing	the lock on the file.  The problem comes in with the associated un-lock
        component. Since the file advances the file-pointer (file-offset) value,
        the assocated un-lock does not unlock the proper file section, but
        rather a section beyong what was written. So we use |uc_lockfile(3uc)|
        instead to just lock and unlock the entire file.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"format.h"


/* local defines */

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#define	PRINTBUFLEN	512
#define	FBUFLEN		512

#define	SUBINFO		struct subinfo


/* external subroutines */

extern int	mkhexstr(char *,int,const void *,int) ;
extern int	rmeol(cchar *,int) ;

extern char	*strwcpy(char *,cchar *,int) ;


/* local structures */

typedef const void	cvoid ;

struct subinfo {
	cchar		*fn ;
	cchar		*id ;
	char		*bp ;
	int		mc ;
	int		bl ;
	int		wl ;
	int		blen ;
	int		ilen ;
} ;


/* forward references */

static int subinfo_start(SUBINFO *,char *,cchar *,cchar *,int) ;
static int subinfo_finish(SUBINFO *) ;
static int subinfo_wrline(SUBINFO *,cchar *,int) ;
static int subinfo_flushover(SUBINFO *,int) ;
static int subinfo_write(SUBINFO *,cchar *,int) ;


/* local variables */


/* exported subroutines */


int nprint(cchar *fn,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (fn == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (fn[0] == '\0') return SR_INVALID ;

	if (sl < 0) sl = strlen(sp) ;

	if (sl > 0) {
	    const int	of = (O_WRONLY | O_APPEND) ;
	    if ((rs = u_open(fn,of,0666)) >= 0) {
	        const int	fd = rs ;
		const int	cmd = F_LOCK ;
		if ((rs = uc_lockfile(fd,cmd,0L,0L,-1)) >= 0) {
	            if ((rs = uc_writen(fd,sp,sl)) >= 0) {
		        len = rs ;
			if ((sl > 0) && (sp[sl-1] != '\n')) {
			    char	nbuf[2] = "\n" ;
			    rs = uc_writen(fd,nbuf,1) ;
			    len += rs ;
			}
		    } /* end if (uc_writen) */
		} /* end if (uc_lockfile) */
	        u_close(fd) ;
	    } /* end if (file) */
	} /* end if (positive) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (nprint) */


int nprintf(cchar *fn,cchar *fmt,...)
{
	int		rs = SR_OK ;
	int		fl = 0 ;

	if (fn == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (fn[0] == '\0') return SR_INVALID ;

	if (fmt[0] != '\0') {
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    const int	flen = FBUFLEN ;
	    char	fbuf[FBUFLEN + 1] ;
	    if ((fl = format(fbuf,flen,1,fmt,ap)) > 0) {
		rs = nprint(fn,fbuf,fl) ;
	    }
	    va_end(ap) ;
	} /* end if (non-nul) */

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (nprintf) */


int nprinthexblock(cchar *fn,cchar *id,int mc,const void *vp,int vl)
{
	SUBINFO		si ;
	int		rs ;
	int		sl = vl ;
	int		wlen = 0 ;
	cchar		*sp = (cchar *) vp ;
	char		b[PRINTBUFLEN + 1] ;

	if (fn == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	if (fn[0] == '\0') return SR_INVALID ;

	if (mc < 0) mc = COLUMNS ;
	if (sl < 0) sl = strlen(sp) ;

	if ((rs = subinfo_start(&si,b,fn,id,mc)) >= 0) {
	    while (sl > 0) {
		rs = subinfo_wrline(&si,sp,sl) ;
		sp += rs ;
		sl -= rs ;
		if (rs < 0) break ;
	    } /* end while */
	    wlen = subinfo_finish(&si) ;
	    if (rs >= 0) rs = wlen ;
	} /* end if (subinfo) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (nprinthexblock) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,char *bp,cchar *fn,cchar *id,int mc)
{
	memset(sip,0,sizeof(SUBINFO)) ;
	sip->bp = bp ;
	sip->fn = fn ;
	sip->mc = mc ;
	sip->blen = PRINTBUFLEN ;
	if (id != NULL) {
	    sip->id = id ;
	    sip->ilen = strlen(id) ;
	}
	return SR_OK ;
}
/* end subroutine (subfino_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	if (sip->bl > 0) {
	    if ((rs = nprint(sip->fn,sip->bp,sip->bl)) >= 0) {
	        sip->wl += rs ;
		sip->bl = 0 ;
	    }
	}
	return (rs >= 0) ? sip->wl : rs ;
}
/* end subroutine (subfino_finish) */


static int subinfo_wrline(SUBINFO *sip,cchar *sp,int sl)
{
	const int	mlen = MIN((3*sl),(sip->mc-sip->ilen+1)) ;
	int		rs ;
	int		ul = 0 ;

	if ((rs = subinfo_flushover(sip,mlen)) >= 0) {
	    if (sip->id != NULL) {
		rs = subinfo_write(sip,sip->id,sip->ilen) ;
	    }
	    if (rs >= 0) {
		const int	alen = (sip->blen - sip->bl) ;
		const int	n = (mlen / 3) ;
	        char		*bp = (sip->bp + sip->bl) ;
	        if ((rs = mkhexstr(bp,alen,sp,n)) >= 0) {
		    sip->bl += rs ;
		    sip->bp[sip->bl++] = '\n' ;
		    ul = n ;
		}
	    }
	} /* end if (subinfo_flushover) */

	return (rs >= 0) ? ul : rs ;
}
/* end subroutine (subinfo_wrline) */


static int subinfo_flushover(SUBINFO *sip,int mlen)
{
	int		rs = SR_OK ;
	if (mlen > (sip->blen-sip->bl)) {
	    char	*bp = (sip->bp + sip->bl) ;
	    if ((rs = nprint(sip->fn,bp,sip->bl)) >= 0) {
	        sip->wl += rs ;
		sip->bl = 0 ;
	    }
	}
	return rs ;
}
/* end subroutine (subfino_flushover) */


static int subinfo_write(SUBINFO *sip,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	if (sl < (sip->blen-sip->bl)) {
	    char	*bp = (sip->bp + sip->bl) ;
	    rs = (strwcpy(bp,sp,sl) - sip->bp) ;
	    sip->bl += rs ;
	} else {
	    rs = SR_OVERFLOW ;
	}
	return rs ;
}
/* end subroutine (subfino_write) */


