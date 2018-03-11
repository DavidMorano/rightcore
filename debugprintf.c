/* debugprintf */

/* special debug printing */


#define	CF_DEBUGN	0		/* debug print-outs */
#define	CF_LINELEN	0		/* use |strlinelen(3dam)| */
#define	CF_USEMALLOC	1		/* use |uc_malloc(3uc)| */


/* revision history:

	= 1983-03-07, David A­D­ Morano
	This subroutine was written for PPI development.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine does a printf-like function but for the special error
        output facility. It calls 'format(3dam)' as might be expected but
        besides that it tries to be as simple as possible, so that it depends on
        as little other stuff as possible.

	Notes about recent UNIX® deficiencies:

        On most more recent UNIXi® systems, writes (even |write(2)| writes) are
        not atomic when the file descriptor is in APPEND mode. This stupid
        behavior that came about with more recent versions of UNIX®, like
        F*ckSolaris® for example. This tupid behavior was first notices (long
        ago) when writes in APPEND mode were made to files located on Network
        File System (NFS) mounted file-systems. But since those early days, this
        behavior has somehow spread to even mess up when writing to files on
        local file-systems.

	Notes:

	Q. Does this subroutine need to be multi-thread-safe?

	A. Of course!

        Q. Why do we need any mutex-locks in here at all? Isn't this code
        completely multi-thread-safe already?!

        A. We need a mutex-lock around the |write(2)| call because that call is
        not atomic on most UNIX®i®! Specifically, the implementation does not
        update the file-pointer atomically along with the associated write of
        data to the file. Yes, Virginia, many UNIX®i® are actually quite messed
        up when it comes to multi-thread-safety! We fix the flaws in most all
        OSes (all OSes at this time are indeed flawed; Solaris® is aware of the
        problem and may or may not fix their implementation) by using our own
        mutex-lock around the |write(2)| call.

        Q. Why are all UNIXi® that exist in the world right now not
        multi-thread-safe with their own OS system calls?

        A. Your guess is as good as mine, but the short answer is that I did not
        write the code for the various UNIX®i® in the world -- even though I
        should have! It really does seem that if I did not write a certain piece
        of code, that code is likely buggy -- regardless of how many people have
        already suffered due to those bugs.

	= On the use of "uc_malloc(3uc)| and the CF_USEMALLOC compile-time flag

        Normally we want to use the heap for "large" buffers. We need a buffer
        of about 2k bytes. This used to be a "small" buffer in the old days when
        programs were mostly single-threaded. But now-a-days since almost all
        programs and subroutines run in a multi-thread environment, the amount
        of stack space availble for "small" buffers is not that large any
        longer; hence the need for dynamic allocation to the heap (not to the
        stack) is now desired. Using |uc_malloc(3uc)| is the default, but if for
        some reason you need to be independent of that subsystem, a non-dynamic
        buffer version is available by setting the CF_USEMALLOC compile-time
        flag to zero (0).


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<ptm.h>
#include	<format.h>
#include	<localmisc.h>


/* local defines */

#define	NDF		"debugprintf.deb"

#ifndef	FD_STDERR
#define	FD_STDERR	3
#endif

#define	FD_BADERR	4
#define	FD_MAX		256		/* maximum FDs we'll consider */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	45		/* can hold int128_t in decimal */
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	DEBUGPRINT	struct debugprint_head

#define	O_FLAGS		(O_WRONLY | O_CREAT | O_APPEND)


/* external subroutines */

extern int	cfdecui(const char *,int,uint *) ;
extern int	hasalldig(const char *,int) ;
extern int	isprintlatin(int) ;
extern int	strlinelen(const char *,int,int) ;
extern int	msleep(int) ;

#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;


/* external variables */


/* local structures */

struct debugprint_head {
	PTM		m ;		/* data mutex */
	volatile int	f_init ;	/* race-condition, blah, blah */
	volatile int	f_initdone ;
	int		fd ;
	int		size ;
} ;


/* forward references */

static int	debugprinters(cchar *,int) ;

int		debugprint_init() ;
void		debugprint_fini() ;

static void	debugprint_atforkbefore() ;
static void	debugprint_atforkafter() ;

int		debugprint(const char *,int) ;
int		debugclose() ;

static int	debugprinter(cchar *,int) ;
static int	cthexi(char *,int) ;
static int	snwcpyprintclean(char *,int,cchar *,int) ;
static int	hasprintbad(cchar *,int) ;
static int	isprintbad(int) ;

static char	*convdeci(LONG,char *) ;


/* local variables */

static DEBUGPRINT	ef ; /* zero-initialized */

static const char	cthextable[] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 0
} ;


/* exported subroutines */


int debugprint_init()
{
	DEBUGPRINT	*uip = &ef ;
	int		rs = 1 ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        void	(*b)() = debugprint_atforkbefore ;
	        void	(*a)() = debugprint_atforkafter ;
	        if ((rs = uc_atfork(b,a,a)) >= 0) {
	            if ((rs = uc_atexit(debugprint_fini)) >= 0) {
	                rs = 0 ;
	                uip->f_initdone = TRUE ;
	            }
	            if (rs < 0)
	                uc_atforkrelease(b,a,a) ;
	        } /* end if (uc_atfork) */
	        if (rs < 0)
	            ptm_destroy(&uip->m) ;
	    } /* end if (ptm_create) */
	    if (rs < 0)
	        uip->f_init = FALSE ;
	} else {
	    while ((rs >= 0) && uip->f_init && (! uip->f_initdone)) {
	        rs = msleep(1) ;
	    }
	    if ((rs >= 0) && (! uip->f_init)) rs = SR_LOCKLOST ;
	}
	return rs ;
}
/* end subroutine (debugprint_init) */


void debugprint_fini()
{
	DEBUGPRINT	*uip = &ef ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    if (uip->fd > 0) {
	        u_close(uip->fd) ;
	        uip->fd = 0 ; /* special case (use zero) */
	    }
	    {
	        void	(*b)() = debugprint_atforkbefore ;
	        void	(*a)() = debugprint_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(DEBUGPRINT)) ;
	} /* end if (was initialized) */
}
/* end subroutine (debugprint_fini) */


int debugprintf(const char fmt[],...)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

#if	CF_DEBUGN
	nprintf(NDF,"debugprintf: ent\n") ;
#endif

	if (fmt == NULL) return SR_FAULT ;

	if (ef.fd >= 0) {
	    const int	fm = FORMAT_ONOOVERR ;
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    char	lbuf[LINEBUFLEN + 1] ;
	    {
	        va_list	ap ;
	        va_begin(ap,fmt) ;
	        if ((len = format(lbuf,llen,fm,fmt,ap)) >= 0) {
	            rs = debugprint(lbuf,len) ;
	            wlen += rs ;
	        } else {
	            rs = SR_TOOBIG ;
		}
	        va_end(ap) ;
	    }
	} else {
	    rs = SR_NOTOPEN ;
	}

#if	CF_DEBUGN
	nprintf(NDF,"debugprintf: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (debugprintf) */


int debugvprintf(const char fmt[],va_list ap)
{
	const int	fm = (FORMAT_OCLEAN | FORMAT_ONOOVERR) ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (fmt == NULL) return SR_FAULT ;

	if (ef.fd >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    char	lbuf[LINEBUFLEN + 1] ;
	    if ((len = format(lbuf,llen,fm,fmt,ap)) >= 0) {
	        rs = debugprint(lbuf,len) ;
	        wlen += rs ;
	    } else {
	        rs = SR_TOOBIG ;
	    }
	} else {
	    rs = SR_NOTOPEN ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (debugvprintf) */


int debugprintdeci(cchar *s,int v)
{
	int		rs = SR_OK ;
	int		llen = LINEBUFLEN ;
	char		lbuf[LINEBUFLEN + 1] ;

	if (s != NULL) {
	    const int	diglen = DIGBUFLEN ;
	    int		ll = llen ;
	    int		sl ;
	    cchar	*sp ;
	    char	*lp = lbuf ;
	    char	digbuf[DIGBUFLEN + 1] ;

	if (rs >= 0) {
	    sp = s ;
	    sl = strlen(s) ;
	    if (sl <= ll) {
	        lp = strwcpy(lp,sp,sl) ;
	        ll -= sl ;
	    } else {
	        rs = SR_OVERFLOW ;
	    }
	}

	if (rs >= 0) {
	    sp = convdeci(v,(digbuf+diglen)) ;
	    sl = ((digbuf+diglen) - sp) ;
	    if (sl <= ll) {
	        lp = strwcpy(lp,sp,sl) ;
	        ll -= sl ;
	    } else {
	        rs = SR_OVERFLOW ;
	    }
	}

	if (rs >= 0) {
	    sp = "\n" ;
	    sl = 1 ;
	    if (sl <= ll) {
	        lp = strwcpy(lp,sp,sl) ;
	        ll -= sl ;
	    } else {
	        rs = SR_OVERFLOW ;
	    }
	}

	if (rs >= 0) {
	    rs = debugprint(lbuf,(lp - lbuf)) ;
	}

	} /* end if (non-null) */

	return rs ;
}
/* end subroutine (debugprintdeci) */


int debugprinthexi(cchar *s,int v)
{
	int		rs = SR_OK ;
	int		ll ;
	int		sl ;
	const char	*sp ;
	char		digbuf[DIGBUFLEN + 1] ;
	char		lbuf[LINEBUFLEN + 1] ;
	char		*lp ;

	if (s == NULL) goto ret0 ;

	ll = LINEBUFLEN ;
	lp = lbuf ;

	if (rs >= 0) {
	    sp = s ;
	    sl = strlen(s) ;
	    if (sl <= ll) {
	        lp = strwcpy(lp,sp,sl) ;
	        ll -= sl ;
	    } else
	        rs = SR_OVERFLOW ;
	}

	if (rs >= 0) {
	    sp = digbuf ;
	    rs = cthexi(digbuf,v) ;
	    sl = rs ;
	    if (sl <= ll) {
	        lp = strwcpy(lp,sp,sl) ;
	        ll -= sl ;
	    } else
	        rs = SR_OVERFLOW ;
	}

	if (rs >= 0) {
	    sp = "\n" ;
	    sl = 1 ;
	    if (sl <= ll) {
	        lp = strwcpy(lp,sp,sl) ;
	        ll -= sl ;
	    } else
	        rs = SR_OVERFLOW ;
	}

	if (rs >= 0) {
	    rs = debugprint(lbuf,(lp - lbuf)) ;
	}

ret0:
	return rs ;
}
/* end subroutine (debugprinthexi) */


int debugprintnum(cchar *s,int v)
{

	return debugprintdeci(s,v) ;
}
/* end subroutine (debugprintnum) */


int debugsetfd(int fd)
{
	int		rs = SR_NOTOPEN ;

	ef.fd = 0 ; /* special case (use zero) */
	if (fd >= 0) {
	    struct ustat	sb ;
	    if ((fd < FD_MAX) && ((rs = u_fstat(fd,&sb)) >= 0)) {
	        ef.fd = fd ;
	        ef.size = sb.st_size ;
	    }
	}

	if (ef.fd >= 0) {
	    u_fchmod(ef.fd ,0666) ;
	}

	return rs ;
}
/* end subroutine (debugsetfd) */


int debugopen(const char *fname)
{
	int		rs = SR_OK ;
	int		fd = -1 ;

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	debugclose() ;

	if (hasalldig(fname,-1)) {
	    uint	v ;
	    rs = cfdecui(fname,-1,&v) ;
	    fd = v ;
	} else {
	    if ((rs = u_open(fname,O_FLAGS,0666)) >= 0) {
	        fd = rs ;
	        if ((rs = uc_moveup(fd,3)) >= 0) {
	            fd = rs ;
	        } else {
	            u_close(fd) ;
	        }
	    }
	} /* end if */

	if (rs >= 0) {
	    struct ustat	sb ;
	    ef.fd = fd ;
	    ef.size = 0 ;
	    if ((rs = u_fstat(ef.fd,&sb)) >= 0) {
	        ef.size = sb.st_size ;
	        u_fchmod(ef.fd,0666) ;
	    }
	} /* end if */

	return (rs >= 0) ? ef.fd : rs ;
}
/* end subroutine (debugopen) */


int debugclose()
{
	if (ef.fd > 0) {
	    u_close(ef.fd) ;
	    ef.fd = 0 ; /* special case (use zero) */
	}
	ef.size = 0 ;
	return 0 ;
}
/* end subroutine (debugclose) */


int debuggetfd()
{

	return ef.fd ;
}
/* end subroutine (debuggetfd) */


/* low level debug-print function */
int debugprint(cchar *sbuf,int slen)
{
	int		rs ;
	if (sbuf == NULL) return SR_FAULT ;
	if (ef.fd <= 0) return SR_NOTOPEN ;
	rs = debugprinter(sbuf,slen) ;
	return rs ;
}
/* end subroutine (debugprint) */


#if	CF_USEMALLOC

int debugprinter(cchar *sbuf,int slen)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	int		f_needeol = FALSE ;
	char		*abuf = NULL ;

#if	CF_LINELEN
	slen = strlinelen(sbuf,slen,LINEBUFLEN) ; /* some protection */
#else
	if (slen < 0) slen = strlen(sbuf) ;
	if (slen > LINEBUFLEN) slen = LINEBUFLEN ;
#endif /* CF_LINELEN */

/* preparation and check if need EOL */

	if ((slen == 0) || (sbuf[slen-1] != '\n')) {
	    f_needeol = TRUE ;
	} else {
	    slen -= 1 ;
	}

/* scan for bad characters */

	if (f_needeol || hasprintbad(sbuf,slen)) {
	    const int	alen = (slen+2) ; /* additional room for added EOL */
	    if ((rs = uc_malloc((alen+1),&abuf)) >= 0) {
	        if ((rs = snwcpyprintclean(abuf,(alen-2),sbuf,slen)) >= 0) {
		    sbuf = abuf ;
		    slen = rs ;
		    abuf[slen++] = '\n' ;
	        }
		if (rs < 0) {
		    uc_free(abuf) ;
		    abuf = NULL ;
		}
	    } /* end if (memory-allocation) */
	} else
	    slen += 1 ;

/* write the line-buffer out */

	if (rs >= 0) {
	    rs = debugprinters(sbuf,slen) ;
	    wlen = rs ;
	} /* end if (ok) */

	if (abuf != NULL) uc_free(abuf) ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (debugprinter) */

#else /* CF_USEMALLOC */

#ifdef	lint
int debugprinter(cchar *sbuf,int slen)
{
	const int	alen = (LINEBUFLEN+2) ; /* room for added EOL */
	int		rs = SR_OK ;
	int		wlen = 0 ;
	int		f_needeol = FALSE ;
	char		abuf[LINEBUFLEN+3] ; /* room for added EOL */

#if	CF_LINELEN
	slen = strlinelen(sbuf,slen,LINEBUFLEN) ; /* some protection */
#else
	if (slen < 0) slen = strlen(sbuf) ;
	if (slen > LINEBUFLEN) slen = LINEBUFLEN ;
#endif /* CF_LINELEN */

/* preparation and check if need EOL */

	if ((slen == 0) || (sbuf[slen-1] != '\n')) {
	    f_needeol = TRUE ;
	} else {
	    slen -= 1 ;
	}

/* scan for bad characters */

	if (f_needeol || hasprintbad(sbuf,slen)) {
	    if ((rs = snwcpyprintclean(abuf,(alen-2),sbuf,slen)) >= 0) {
		sbuf = abuf ;
		slen = rs ;
		abuf[slen++] = '\n' ;
	    }
	} else
	    slen += 1 ;

/* write the line-buffer out */

	if (rs >= 0) {
	    rs = debugprinters(sbuf,slen) ;
	    wlen = rs ;
	} /* end if (ok) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (debugprinter) */
#else /* lint */
int debugprinter(cchar *sbuf,int slen)
{
	int		rs = SR_OK ;
	int		alen ;
	int		wlen = 0 ;
	int		f_needeol = FALSE ;

#if	CF_LINELEN
	slen = strlinelen(sbuf,slen,LINEBUFLEN) ; /* some protection */
#else
	if (slen < 0) slen = strlen(sbuf) ;
	if (slen > LINEBUFLEN) slen = LINEBUFLEN ;
#endif /* CF_LINELEN */

/* preparation and check if need EOL */

	if ((slen == 0) || (sbuf[slen-1] != '\n')) {
	    f_needeol = TRUE ;
	} else {
	    slen -= 1 ;
	}

	alen = (slen+2) ; /* room for added EOL */
	{
	    char	abuf[alen+1] ;

/* scan for bad characters */

	    if (f_needeol || hasprintbad(sbuf,slen)) {
	        if ((rs = snwcpyprintclean(abuf,(alen-2),sbuf,slen)) >= 0) {
		    sbuf = abuf ;
		    slen = rs ;
		    abuf[slen++] = '\n' ;
	        }
	    } else
	        slen += 1 ;

/* write the line-buffer out */

	    if (rs >= 0) {
	        rs = debugprinters(sbuf,slen) ;
	        wlen = rs ;
	    } /* end if (ok) */

	} /* end block (dynamic stack buffer allocation) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (debugprinter) */
#endif /* lint */

#endif /* CF_USEMALLOC */


/* local subroutines */


static int debugprinters(cchar *sbuf,int slen)
{
	int		rs ;
	int		wlen = 0 ;
	if ((rs = debugprint_init()) >= 0) {
	    DEBUGPRINT	*uip = &ef ;
	    if ((rs = ptm_lock(&uip->m)) >= 0) { /* single */
		int	cmd = F_LOCK ;
		if ((rs = uc_lockf(ef.fd,cmd,0L)) >= 0) {
		    USTAT	sb ;
		    if ((rs = u_fstat(ef.fd,&sb)) >= 0) {

	                if (S_ISREG(sb.st_mode) && (sb.st_size != ef.size)) {
	                    offset_t	uoff = sb.st_size ;
	                    ef.size = sb.st_size ;
	                    u_seek(ef.fd,uoff,SEEK_SET) ;
	                }

	                if ((rs = u_write(ef.fd,sbuf,slen)) >= 0) {
	                    wlen = rs ;
	                    ef.size += wlen ;
	                }

		    } /* end if (u_fstat) */
		    cmd = F_UNLOCK ;
		    uc_lockf(ef.fd,cmd,0L) ;
		} /* end if (uc_lockf) */
	        ptm_unlock(&uip->m) ;
	    } /* end if (ptm) */
	} /* end if (debugprint_init) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (debugprinters) */


static void debugprint_atforkbefore()
{
	DEBUGPRINT	*uip = &ef ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (debugprint_atforkbefore) */


static void debugprint_atforkafter()
{
	DEBUGPRINT	*uip = &ef ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (debugprint_atforkafter) */


static char *convdeci(LONG num,char *endptr)
{
	ULONG		unum = (ULONG) num ;
	char		*bp ;
	if (num < 0) unum = (- unum) ;
	bp = ulltostr(unum,endptr) ;
	if (num < 0) *--bp = '-' ;
	return bp ;
}
/* end subroutine (convdeci) */


static int cthexi(char *buf,int val)
{
	const int	n = (2 * sizeof(int)) ;
	int		i  ;
	for (i = (n - 1) ; i >= 0 ; i -= 1) {
	    buf[i] = cthextable[val & 0x0F] ;
	    val >>= 4 ;
	} /* end for */
	buf[n] = '\0' ;
	return n ;
}
/* end subroutine (cthexi) */


static int snwcpyprintclean(char *dbuf,int dlen,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		ch ;
	int		dl = 0 ;
	while (dlen-- && sl && *sp) {
	    ch = MKCHAR(*sp) ;
	    if (isprintbad(ch)) {
	        if (ch == '\n') {
		    ch = '¬' ;
	        } else {
		    ch = '¿' ;
		}
	    }
	    dbuf[dl++] = (char) ch ;
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */
	if ((sl != 0) && (*sp != '\0')) rs = SR_OVERFLOW ;
	dbuf[dl] = '\0' ;
	return (rs >= 0) ? dl : rs ;
}
/* end subroutine (snwcpyprintclean) */


static int hasprintbad(cchar *sp,int sl)
{
	int		f = FALSE ;
	while (sl && *sp) {
	    f = isprintbad(sp[0] & 0xff) ;
	    if (f) break ;
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */
	return f ;
}
/* end subroutine (hasprintbad) */


static int isprintbad(int ch)
{
	return (! isprintlatin(ch)) ;
}
/* end subroutine (isprintbad) */


