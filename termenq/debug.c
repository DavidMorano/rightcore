/* debug */

/* debugging stubs */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-15, David A­D­ Morano
	This was written to debug the REXEC program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This modeule provides debugging support for the REXEC program.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stropts.h>
#include	<poll.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#define	PRINTBUFLEN	(COLUMNS + 2)
#define	HEXBUFLEN	100

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	snopenflags(char *,int,int) ;
extern int	snpollflags(char *,int,int) ;
extern int	getdig(int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	nprintf(const char *,const char *,...) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugprint(cchar *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct debug_oflags {
	int		m ;
	char		*s ;
} ;


/* forward subroutines */

int		mkhexstr(char *,int,const void *,int) ;

static int	checkbasebounds(const char *,int,void *) ;


/* external variables */


/* local variables */


/* exported subroutines */


#if	CF_DEBUGS
int debuginit()
{
	int		rs = SR_OK ;


	return rs ;
}
/* end subroutine (debuginit) */
#endif /* CF_DEBUGS */


char *d_reventstr(int revents,char *bp,int bl)
{
	snpollflags(bp,bl,revents) ;
	return bp ;
}
/* end subroutine (d_reventstr) */


/* who is open? */
void d_whoopen(int *s)
{
	int		rs ;
	int		i ;

	if (s != NULL) {
	    debugprintf("d_whoopen: %s\n",s) ;
	}

	for (i = 0 ; i < 20 ; i += 1) {
	    if ((rs = u_fcntl(i,F_GETFL,0)) >= 0) {
	        debugprintf("d_whoopen: open on %d accmod=%08x\n",
	            i,(rs & O_ACCMODE)) ;
	    }
	}

}
/* end subroutine (d_whoopen) */


/* return a count of the number of open files */
int d_openfiles()
{
	struct ustat	sb ;
	int		i ;
	int		count = 0 ;

	for (i = 0 ; i < 2048 ; i += 1) {
	    if (u_fstat(i,&sb) >= 0) {
	        count += 1 ;
	    }
	} /* end for */

	return count ;
}
/* end subroutine (d_openfiles) */


int d_ispath(cchar *p)
{

	if (p == NULL) return FALSE ;

#ifdef	DEBFILE
	nprintf(DEBFILE,"d_ispath: PATH=>%W<\n",
	    p,strnlen(p,30)) ;
#endif

	return ((*p == '/') || (*p == ':')) ;
}
/* end subroutine (d_ispath) */


int gdb()
{
	return 0 ;
}
/* end subroutine (gdb) */


int mkhexstr(char *dbuf,int dlen,const void *vp,int vl)
{
	int		sl = vl ;
	int		i ;
	int		ch ;
	int		j = 0 ;
	cchar		*sp = (cchar *) vp ;
	if (sl < 0) sl = strlen(sp) ;
	for (i = 0 ; (dlen >= 3) && (i < sl) ; i += 1) {
	    ch = MKCHAR(sp[i]) ;
	    if (i > 0) dbuf[j++] = ' ' ;
	    dbuf[j++] = getdig((ch>>4)&15) ;
	    dbuf[j++] = getdig((ch>>0)&15) ;
	    dlen -= ((i > 0) ? 3 : 2) ;
	} /* end for */
	dbuf[j] = '\0' ;
	return j ;
}
/* end subroutine (mkhexstr) */


int mkhexnstr(char *hbuf,int hlen,int maxcols,cchar *sbuf,int slen)
{
	int		n = 0 ;

	if (maxcols < 0) maxcols = COLUMNS ;

	if (slen < 0) slen = strlen(sbuf) ;
	n = MIN((maxcols / 3),slen) ;
	mkhexstr(hbuf,hlen,sbuf,n) ;

	return n ;
}
/* end subroutine (mkhexnstr) */


int debugprinthex(cchar *ids,int maxcols,cchar *sp,int sl)
{
	const int	plen = PRINTBUFLEN ;
	int		rs ;
	int		idlen = 0 ;
	int		wlen = 0 ;
	char		pbuf[PRINTBUFLEN + 1] ;

	if (ids != NULL) idlen = strlen(ids) ;

	if (maxcols < 0) maxcols = COLUMNS ;

	if (idlen > 0) maxcols -= (idlen + 1) ;

	if ((rs = mkhexnstr(pbuf,plen,maxcols,sp,sl)) >= 0) {
	    if (idlen > 0) {
	        rs = debugprintf("%t %s\n",ids,idlen,pbuf) ;
	    } else {
	        rs = debugprintf("%s\n",pbuf) ;
	    }
	    wlen = rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (debugprinthex) */


int debugprinthexblock(cchar *ids,int maxcols,const void *vp,int vl)
{
	int		rs = SR_OK ;
	int		idlen = 0 ;
	int		sl = vl ;
	int		wlen = 0 ;
	cchar		*sp = (cchar *) vp ;
	char		printbuf[PRINTBUFLEN + 1] ;

	if (ids != NULL) idlen = strlen(ids) ;

	if (maxcols < 0) maxcols = COLUMNS ;

	if (sl < 0) sl = strlen(sp) ;

	while ((rs >= 0) && (sl > 0)) {
	    char	*pbp = printbuf ;
	    int		pbl = PRINTBUFLEN ;
	    int		cols = maxcols ;

	    if (ids != NULL) {
	        if ((idlen+2) < pbl) {
		    int	i = strwcpy(pbp,ids,idlen) - pbp ;
	            pbp[i++] = ':' ;
	            pbp[i++] = ' ' ;
	            pbp += i ;
	            pbl -= i ;
	            cols -= i ;
	        } else {
	            rs = SR_OVERFLOW ;
		}
	    }

	    if (rs >= 0) {
	        const int	n = (cols / 3) ;
		int		cslen ;
	        cslen = MIN(n,sl) ;
	        if ((rs = mkhexstr(pbp,pbl,sp,cslen)) >= 0) {
	            sp += cslen ;
	            sl -= cslen ;
	            rs = debugprint(printbuf,-1) ;
	            wlen += rs ;
		}
	    }

	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (debugprinthexblock) */


int hexblock(cchar *ids,cchar *ap,int n)
{
	const int	hexlen = HEXBUFLEN ;
	int		i, sl ;
	char		hexbuf[HEXBUFLEN + 3] ;

	if (ids != NULL)
	    debugprint(ids,-1) ;

	for (i = 0 ; i < n ; i += 1) {
	    sl = mkhexstr(hexbuf,hexlen,ap,4) ;
	    hexbuf[sl++] = '\n' ;
	    hexbuf[sl] = '\0' ;
	    ap += 4 ;
	    debugprint(hexbuf,-1) ;
	} /* end for */

	return n ;
}
/* end subroutine (hexblock) */


/* audit a HOSTENT structure */
int heaudit(struct hostent *hep,cchar *buf,int buflen)
{
	int		rs = SR_OK ;
	int		i ;
	char		**cpp ;

	if (hep == NULL) return SR_FAULT ;
	if (buf == NULL) return SR_FAULT ;

	if (buflen < 0) return SR_INVALID ;

	if (rs >= 0) {
	    rs = checkbasebounds(buf,buflen,hep->h_name) ;
	}

	if (rs >= 0) {
	    cpp = hep->h_aliases ;
	    if (cpp != NULL) {
	        if ((rs = checkbasebounds(buf,buflen,cpp)) >= 0) {
	            for (i = 0 ; cpp[i] != NULL ; i += 1) {
	                rs = checkbasebounds(buf,buflen,(cpp + i)) ;
	                if (rs >= 0)
	                    rs = checkbasebounds(buf,buflen,cpp[i]) ;
	                if (rs < 0)
	                    break ;
	            }
	        }
	    }
	}

	if (rs >= 0) {
	    cpp = hep->h_aliases ;
	    if (cpp != NULL) {
	        if ((rs = checkbasebounds(buf,buflen,cpp)) >= 0) {
	            for (i = 0 ; cpp[i] != NULL ; i += 1) {
	                rs = checkbasebounds(buf,buflen,(cpp + i)) ;
	                if (rs >= 0)
	                    rs = checkbasebounds(buf,buflen,cpp[i]) ;
	                if (rs < 0)
	                    break ;
	            }
	        }
	    }
	}

	return rs ;
}
/* end subroutine (heaudit) */


char *stroflags(char *buf,int oflags)
{
	int		rs = snopenflags(buf,TIMEBUFLEN,oflags) ;
	return (rs >= 0) ? buf : NULL ;
}
/* end subroutine (stroflags) */


/* local subroutines */


static int checkbasebounds(cchar *bbuf,int blen,void *vp)
{
	int		rs = SR_OK ;
	cchar		*tp = (cchar *) vp ;

	if (tp >= bbuf) {
	    if (tp >= (bbuf + blen)) rs = SR_BADFMT ;
	} else {
	    rs = SR_BADFMT ;
	}

	return rs ;
}
/* end subroutine (checkbasebounds) */


int debugprintfsize(cchar *id,int fd)
{
	USTAT		sb ;
	int		rs ;
	if ((rs = u_fstat(fd,&sb)) >= +0) {
	    ulong	fs = sb.st_size ;
#if	defined(_I32LPx)
	    debugprintf("debugprintfsize: I32LPx\n") ;
#endif
	    debugprintf("debugprintfsize: %s size=%lu\n",id,fs) ;
	}
	return rs ;
}
/* end subroutine (debugprintstat) */


