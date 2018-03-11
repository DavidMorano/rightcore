/* shio */

/* the SHell-IO hack */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUGN	0		/* special */
#define	CF_FLUSHPART	0		/* partial flush */
#define	CF_SFSWAP	0		/* use 'sfswap(3ast)' */
#define	CF_STOREFNAME	0		/* store the file-name */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a hack to make the SFIO stuff useable in an interchangeable way
	when not in an SFIO environment.  This generally occurs when a
	subroutine is used both in a SHELL builtin as well as stand-alone.

	Notes:

	1. If we are *outside* of KSH, we want to switch to using the BFILE
	(our own) I/O even if the file we are to operate on is one of the
	"standard" (in, out, err) files.  We do this so that we do not have to
	use the crazy SFIO where we do not really need it.  When we are
	*inside* KSH, we have to use SFIO because KSH passes us special
	"memory-implemented" (my term) files.  We need to use the memory based
	files passed us from KSH in order for the KSH variable assignments and
	other special operations for input and output to work correctly.  We
	implement knowing whether we are "inside" or "outside" of KSH by using
	the |shio_outside()| subroutine.  That subroutine finds out whether we
	are inside or outside only once (that is what it is all about) so that
	we remember that condition for the remainder of our stay loaded into
	this nice computer.  We increasingly live in a
	terminate-and-stay-resident world; we have to start programming for
	that.

	2. How would we ever be "outside" of KSH?  You may ask?  We are just a
	shared-object.  What if someone other than KSH calls us?  In that case,
	we would be executing, but we would be executing "outside" of KSH.
	This does happen.  Servers and other programs call us (not uncommonly)
	without KSH being in the mix anywhere.  For example, when any of the
	"commands" are called from outside of KSH, like from another shell that
	is not KSH, we will be executing "outside" of KSH.  So this is actually
	much more common than one might think at first.


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
#include	<dlfcn.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<bfile.h>
#include	<sigblock.h>
#include	<outstore.h>
#include	<localmisc.h>

#include	"shio.h"


/* local defines */

#define	SHIO_MODESTRLEN	20
#define	SHIO_KSHSYM	"sh_main"
#define	SHIO_PERSISTENT	struct shio_persistent

#ifndef	SFIO_OFF
#define	SFIO_OFF	0
#endif

#ifndef	SFIO_ON
#define	SFIO_ON		1
#endif

#define	SFIO_MAXREADLEN	100

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	NULLDEV
#define	NULLDEV		"/dev/null"
#endif

#define	ISCONT(b,bl)	\
	(((bl) >= 2) && ((b)[(bl) - 1] == '\n') && ((b)[(bl) - 2] == '\\'))

#define	NDF		"shio.deb"


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	msleep(int) ;
extern int	tcgetlines(int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_SFIO
extern int	sfreadline(Sfio_t *,char *,int) ;
extern int	sfreadlinetimed(Sfio_t *,char *,int,int) ;
extern int	sfisterm(Sfio_t *) ;
#endif

extern int	format(char *,int,int,const char *,va_list) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* local structures */

struct shio_persistent {
	volatile uint	f_init ;
	volatile uint	f_outside ;
} ;


/* forward references */

static int	shio_bopene(SHIO *,int,cchar *,cchar *,mode_t,int) ;
static int	shio_bclose(SHIO *) ;

#if	CF_SFIO
static int	shio_outside() ;
static int	shio_sfiscook(SHIO *) ;
static int	shio_sfflush(SHIO *) ;
static int	shio_shprintln(SHIO *,cchar *,int) ;
static int	shio_sfwrite(SHIO *,cchar *,int) ;
static int	shio_sfcookline(SHIO *op,int f) ;
static int	shio_sfcookbegin(SHIO *) ;
static int	shio_sfcookend(SHIO *) ;
static int	shio_sfcheckwrite(SHIO *,cchar *,int) ;
static int	shio_sfcookdump(SHIO *) ;
static int	shio_sfcookflush(SHIO *) ;
static int	shio_sfcookwrite(SHIO *,cchar *,int) ;
#endif /* CF_SFIO */

static int	hasnl(const char *,int) ;
static int	isNotSeek(int) ;
static int	isInterrupt(volatile int **) ;


/* static writable data */

#if	CF_SFIO
static struct shio_persistent	shio_data ; /* initialized to all zeros */
#endif /* CF_SFIO */


/* local variables */

static const char	*stdfnames[] = {
	STDINFNAME,
	STDOUTFNAME,
	STDERRFNAME,
	STDNULLFNAME,
	"*NULL*",
	NULL
} ;

enum stdfnames {
	stdfname_stdin,
	stdfname_stdout,
	stdfname_stderr,
	stdfname_stdnull,
	stdfname_oldnull,
	stdfname_overlast
} ;

static const int	seekrs[] = {
	SR_NOTSEEK,
	0
} ;


/* exported subroutines */


int shio_opene(SHIO *op,cchar *fname,cchar *ms,mode_t om,int to)
{
	int		rs = SR_OK ;
	int		fni ;
#if	CF_SFIO
	Sfio_t		*fp = NULL ;
#endif
	const char	*ofname = NULL ;

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;
	if (ms == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;
	if (ms[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("shio_opene: fname=%s\n",fname) ;
	debugprintf("shio_opene: modstr=%s\n",ms) ;
#endif

/* start in */

	memset(op,0,sizeof(SHIO)) ;

	fni = matstr(stdfnames,fname,-1) ;

#if	CF_DEBUGS
	debugprintf("shio_opene: fni=%d\n",fni) ;
#endif

	op->f.stdfname = (fni >= 0) ;
	switch (fni) {
	case stdfname_stdin:
	    ofname = BFILE_STDIN ;
#if	CF_SFIO
	    fp = sfstdin ;
#endif
	    break ;
	case stdfname_stdout:
	    ofname = BFILE_STDOUT ;
#if	CF_SFIO
	    fp = sfstdout ;
#endif
	    break ;
	case stdfname_stderr:
	    ofname = BFILE_STDERR ;
#if	CF_SFIO
	    fp = sfstderr ;
#endif
	    break ;
	case stdfname_stdnull:
	case stdfname_oldnull:
	    op->f.nullfile = TRUE ;
	    break ;
	default:
	    ofname = fname ;
	    break ;
	} /* end switch */

	if (op->f.nullfile) goto ret1 ;

#if	CF_SFIO

#if	CF_DEBUGS
	debugprintf("shio_opene: operating w/ SFIO \n") ;
#endif

	if (fp) {
	    if (shio_outside()) fp = NULL ; /* if outside switch to BFILE */
	}

	if ((fni >= 0) && (fp != NULL)) {
	    int		rs1 ;

#if	CF_DEBUGS
	    debugprintf("shio_opene: builtin-SFIO\n") ;
#endif

	    op->f.sfio = TRUE ;
#if	CF_SFSWAP
	    rs1 = sfsync(fp) ;
	    if (rs1 >= 0) {
	        op->fp = sfswap(fp,NULL) ;
	        rs = (op->fp != NULL) ? SR_OK : SR_MFILE ;
	    } else
	        rs = SR_BADF ;
#else
	    op->fp = fp ;
#endif /* CF_SFSWAP */

	    if (rs >= 0) {
	        if ((rs1 = sffileno(op->fp)) >= 0) {
	            if (isatty(rs1)) {
	                op->f.terminal = TRUE ;
	                op->f.bufline = TRUE ;
	            }
	        }
#if	CF_DEBUGN
	        nprintf(NDF,"shio_opene: rs1=%d f_term=%u f_bufline=%u\n",
	            rs1,op->f.terminal,op->f.bufline) ;
#endif
	        if (op->f.bufline) {
	            rs = shio_sfcookbegin(op) ;
	        } /* end if (buffered) */
	    } /* end if (ok) */

	} else {

#if	CF_DEBUGS
	    debugprintf("shio_opene: builtin-BFILE\n") ;
#endif

	    rs = shio_bopene(op,fni,fname,ms,om,to) ;

#if	CF_DEBUGS
	    debugprintf("shio_opene: shio_bopene() rs=%d\n",rs) ;
#endif

	} /* end if */

#else /* CF_SFIO */

#if	CF_DEBUGS
	debugprintf("shio_opene: regular-BFILE\n") ;
#endif

	rs = shio_bopene(op,fni,ofname,ms,om,to) ;

#endif /* CF_SFIO */

#if	CF_SFIO
#else

	if (rs >= 0) {
	    rs = bcontrol(op->fp,BC_ISLINEBUF,0) ;
	    op->f.bufline = (rs > 0) ;
	}

	if (rs >= 0) {
	    rs = bcontrol(op->fp,BC_ISTERMINAL,0) ;
	    op->f.terminal = (rs > 0) ;
	}

#endif /* CF_SFIO */

#if	CF_STOREFNAME
	if (rs >= 0) {
	    const char	*cp ;
	    if ((rs = uc_mallocstrw(ofname,-1,&cp)) >= 0) {
	        op->fname = cp ;
	    }
	}
#endif /* CF_STOREFNAME */

ret1:
	if (rs >= 0)
	    op->magic = SHIO_MAGIC ;

#if	CF_DEBUGS
	debugprintf("shio_opene: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (shio_opene) */


int shio_open(SHIO *op,cchar *fname,cchar *ms,mode_t om)
{

	return shio_opene(op,fname,ms,om,-1) ;
}
/* end subroutine (shio_open) */


int shio_opentmp(SHIO *op,mode_t om)
{
	int		rs ;
	const char	*tfn = "shioXXXXXXXXXX" ;
	const char	*tmpdname = getenv(VARTMPDNAME) ;
	char		template[MAXPATHLEN+1] ;

	if (tmpdname == NULL) tmpdname = TMPDNAME ;

	if ((rs = mkpath2(template,tmpdname,tfn)) >= 0) {
	    SIGBLOCK	blocker ;
	    if ((rs = sigblock_start(&blocker,NULL)) >= 0) {
	        char	tbuf[MAXPATHLEN+1] ;
	        if ((rs = mktmpfile(tbuf,om,template)) >= 0) {
	            rs = shio_opene(op,tbuf,"rw",om,-1) ;
	            uc_unlink(tbuf) ;
	        } /* end if (mktmpfile) */
	        sigblock_finish(&blocker) ;
	    } /* end if (sigblock) */
	} /* end if (mkpath) */

	return rs ;
}
/* end subroutine (shio_opentmp) */


int shio_close(SHIO *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SHIO_MAGIC) return SR_NOTOPEN ;

	if (op->f.nullfile) goto ret1 ;

#if	CF_DEBUGS
	if (op->fname != NULL)
	    debugprintf("shio_close: fname=%s\n",op->fname) ;
#endif

#if	CF_SFIO
	if (op->outstore != NULL) {
	    rs1 = shio_sfcookend(op) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (outstore) */
#endif /* CF_SFIO */

#if	CF_STOREFNAME
	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}
#endif /* CF_STOREFNAME */

#if	CF_SFIO
	if (op->f.sfio) {
	    int	rs1 = SR_BADF ;
#if	CF_DEBUGS
	    debugprintf("shio_close: builtin-SFIO\n") ;
#endif
	    if (op->fp != NULL) {
#if	CF_SFSWAP
	        rs1 = sfclose(op->fp) ;
#else
	        rs1 = sfsync(op->fp) ;
#endif
	    }
	    rs = (rs1 >= 0) ? SR_OK : SR_BADF ;
	} else {
#if	CF_DEBUGS
	    debugprintf("shio_close: builtin-BFILE\n") ;
#endif
	    rs1 = shio_bclose(op) ;
	    if (rs >= 0) rs = rs1 ;
	}
#else
#if	CF_DEBUGS
	debugprintf("shio_close: regular-BFILE\n") ;
#endif
	rs1 = shio_bclose(op) ;
	if (rs >= 0) rs = rs1 ;
#endif /* CF_SFIO */

ret1:
	op->fp = NULL ;
	op->magic = 0 ;
	return rs ;
}
/* end subroutine (shio_close) */


int shio_reade(SHIO *op,void *abuf,int alen,int to,int opts)
{
	int		rs = SR_OK ;
	int		rlen = alen ;
	char		*rbuf = (char *) abuf ;

	if (op == NULL) return SR_FAULT ;
	if (abuf == NULL) return SR_FAULT ;

	if (op->magic != SHIO_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("shio_reade: f.sfio=%u f.bufline=%u\n",
	    op->f.sfio,op->f.bufline) ;
	debugprintf("shio_reade: to=%d opts=%04x\n",to,opts) ;
	debugprintf("shio_reade: FM_TIMED=%u\n",
	    ((opts & FM_TIMED) ? 1 : 0)) ;
#endif

	if (op->f.nullfile) goto ret0 ;

	if (rlen == 0) goto ret0 ;

#if	CF_SFIO
	if (op->f.sfio) {
	    Sfio_t	*fp = op->fp ;
	    Sfio_t	*streams[2] ;
	    time_t	ti_now = time(NULL) ;
	    time_t	ti_start ;

	    rbuf[0] = '\0' ;
	    ti_start = ti_now ;
	    streams[0] = fp ;
	    streams[1] = NULL ;
	    while (rs >= 0) {
	        if ((rs = sfpoll(streams,1,1000)) < 0) rs = SR_INTR ;
#if	CF_DEBUGS
	        debugprintf("shio_reade: sfpoll() rs=%d\n",rs) ;
#endif
	        if (rs > 0) {
	            int v = sfvalue(fp) ;
#if	CF_DEBUGS
	            debugprintf("shio_reade: sfvalue() v=%04x\n",v) ;
#endif
	            if (v & SF_READ) {
	                if (op->f.bufline || op->f.bufnone) {
	                    char	*p ;
	                    p = sfgetr(op->fp,'\n',0) ;
#if	CF_DEBUGS
	                    debugprintf("shio_reade: sfgetr() p=%p\n",p) ;
#endif
	                    if (p != NULL) {
	                        if ((v = sfvalue(fp)) < 0) rs = SR_HANGUP ;
#if	CF_DEBUGS
	                        debugprintf("shio_reade: sfvalue() v=%d\n",v) ;
#endif
	                        if (rs >= 0) {
	                            rs = snwcpy(rbuf,rlen,p,v) ;
				}
	                    } else {
	                        p = sfgetr(op->fp,'\n',SF_LASTR) ;
	                        if (p != NULL) {
	                            if ((v = sfvalue(fp)) < 0) rs = SR_HANGUP ;
	                            if (rs >= 0)
	                                rs = snwcpy(rbuf,rlen,p,v) ;
	                        } else {
	                            rs = SR_OK ;
	                            break ;
	                        }
	                    }
	                } else {
	                    if ((rs = sfread(op->fp,rbuf,rlen)) < 0)
	                        rs = SR_HANGUP ;
#if	CF_DEBUGS
	                    debugprintf("shio_reade: sfread() rs=%d\n",rs) ;
	                    if (rs >= 0)
	                        debugprintf("shio_reade: rbuf=>%t<\n",rbuf,rs) ;
#endif
	                }
	                break ;
	            } else {
	                msleep(10) ;
		    }
	        } else if (rs < 0) {
	            break ;
		}
	        if (to >= 0) {
	            ti_now = time(NULL) ;
	            if ((ti_now - ti_start) >= to) {
	                rs = SR_TIMEDOUT ;
	                break ;
	            }
	        }
	    } /* end while */
	} else {
	    rs = breade(op->fp,rbuf,rlen,to,opts) ;
	}
#else /* CF_SFIO */
	rs = breade(op->fp,rbuf,rlen,to,opts) ;

#if	CF_DEBUGS
	debugprintf("shio_reade: breade() rs=%d\n",rs) ;
#endif

#endif /* CF_SFIO */

ret0:

#if	CF_DEBUGS
	debugprintf("shio_reade: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (shio_reade) */


int shio_read(SHIO *op,void *ubuf,int ulen)
{

	return shio_reade(op,ubuf,ulen,-1,0) ;
}
/* end subroutine (shio_read) */


int shio_readlinetimed(SHIO *op,char *lbuf,int llen,int to)
{
	int		rs = SR_OK ;
	int		rl = 0 ;

#if	CF_DEBUGS
	debugprintf("shio_readlinetimed: ent llen=%d to=%d\n",llen,to) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

	if (op->magic != SHIO_MAGIC) return SR_NOTOPEN ;

	if (op->f.nullfile) goto ret0 ;

#if	CF_SFIO
	if (op->f.sfio) {
	    rl = sfreadlinetimed(op->fp,lbuf,llen,to) ;
	    rs = (rl >= 0) ? rl : SR_HANGUP ;
	} else {
	    rs = breadlinetimed(op->fp,lbuf,llen,to) ;
	    rl = rs ;
	}
#else /* CF_SFIO */
	rs = breadlinetimed(op->fp,lbuf,llen,to) ;
	rl = rs ;
#endif /* CF_SFIO */

ret0:

#if	CF_DEBUGS
	debugprintf("shio_readlinetimed: ret rs=%d rl=%u\n",rs,rl) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (shio_readlinetimed) */


int shio_readline(SHIO *op,char *lbuf,int llen)
{

	return shio_readlinetimed(op,lbuf,llen,-1) ;
}
/* end subroutine (shio_readline) */


int shio_readlines(SHIO *fp,char *lbuf,int llen,int *lcp)
{
	int		rs = SR_OK ;
	int		i = 0 ;
	int		lines = 0 ;
	int		f_cont = FALSE ;

#if	CF_DEBUGS
	debugprintf("shio_readlines: ent llen=%d\n",llen) ;
#endif

	if (fp == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

	lbuf[0] = '\0' ;
	while ((lines == 0) || (f_cont = ISCONT(lbuf,i))) {

	    if (f_cont) i -= 2 ;

	    rs = shio_readline(fp,(lbuf + i),(llen-i)) ;
	    if (rs <= 0) break ;
	    i += rs ;

	    lines += 1 ;
	} /* end while */

	if (lcp != NULL) *lcp = lines ;

#if	CF_DEBUGS
	debugprintf("shio_readlines: ret rs=%d len=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (shio_readlines) */


int shio_write(SHIO *op,const void *lbuf,int llen)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

	if (op->magic != SHIO_MAGIC) return SR_NOTOPEN ;

	if (op->f.nullfile) {
	    wlen = llen ;
	    goto ret0 ;
	}

#if	CF_DEBUGS
	debugprintf("shio_write: f.sfio=%u f.bufline=%u\n",
	    op->f.sfio,op->f.bufline) ;
	if (llen < 0) llen = strlen(lbuf) ;
	debugprintf("shio_write: wbuf=>%t<\n",
	    lbuf,strlinelen(lbuf,llen,60)) ;
#endif

#if	CF_SFIO
	if (op->f.sfio) {
	    if (llen < 0) llen = strlen(lbuf) ;
	    rs = shio_sfcheckwrite(op,lbuf,llen) ;
	    wlen = rs ;
	} else {
	    rs = bwrite(op->fp,lbuf,llen) ;
	    wlen = rs ;
#if	CF_FLUSHPART
	    if ((rs >= 0) && op->f.bufline && hasnl(lbuf,llen)) {
	        rs = bflush(op->fp) ;
	    }
#endif
	}
#else /* CF_SFIO */
	rs = bwrite(op->fp,lbuf,llen) ;
	wlen = rs ;
#if	CF_FLUSHPART
	if ((rs >= 0) && op->f.bufline && hasnl(lbuf,llen)) {
	    rs = bflush(op->fp) ;
	}
#endif
#endif /* CF_SFIO */

ret0:

#if	CF_DEBUGS
	debugprintf("shio_write: ret rs=%d\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (shio_write) */


int shio_println(SHIO *op,cchar *lbuf,int llen)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

	if (op->magic != SHIO_MAGIC) return SR_NOTOPEN ;

	if (! op->f.nullfile) {

#if	CF_SFIO
	if (op->f.sfio) {
	    if (llen < 0) llen = strlen(lbuf) ;
	    rs = shio_shprintln(op,lbuf,llen) ;
	    wlen += rs ;
	} else {
	    rs = bprintln(op->fp,lbuf,llen) ;
	    wlen += rs ;
	}
#else /* CF_SFIO */
	rs = bprintln(op->fp,lbuf,llen) ;
	wlen += rs ;
#endif /* CF_SFIO */

	} else {
	    wlen = llen ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (shio_println) */


int shio_print(SHIO *op,cchar *lbuf,int llen)
{
	return shio_println(op,lbuf,llen) ;
}
/* end subroutine (shio_print) */


int shio_printline(SHIO *op,cchar *lbuf,int llen)
{
	return shio_println(op,lbuf,llen) ;
}
/* end subroutine (shio_printline) */


int shio_printf(SHIO *op,cchar *fmt,...)
{
	int		rs ;
	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = shio_vprintf(op,fmt,ap) ;
	    va_end(ap) ;
	}
	return rs ;
}
/* end subroutine (shio_printf) */


int shio_vprintf(SHIO *op,const char *fmt,va_list ap)
{
	const int	llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("shio_vprintf: fmt=>%t<\n",
	    fmt,strlinelen(fmt,-1,60)) ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SHIO_MAGIC) return SR_NOTOPEN ;

	if (op->f.nullfile) goto ret0 ;

	if ((rs = format(lbuf,llen,0,fmt,ap)) > 0) {
	    int	len = rs ;

#if	CF_DEBUGS
	    debugprintf("shio_vprintf: format() rs=%d\n",rs) ;
	    if (rs >= 0)
	        debugprintf("shio_vprintf: r=>%t<\n",
	            lbuf,strlinelen(lbuf,len,60)) ;
#endif

#if	CF_SFIO
	    if (op->f.sfio) {
	        rs = shio_sfcheckwrite(op,lbuf,len) ;
	        wlen = rs ;
	        if ((rs >= 0) && op->f.bufline && hasnl(lbuf,len)) {
	            sfsync(op->fp) ;
	        }
	    } else {
	        if ((rs = bwrite(op->fp,lbuf,len)) >= 0) {
	            wlen = rs ;
	            if (op->f.bufline && hasnl(lbuf,len)) {
	                rs = bflush(op->fp) ;
		    }
	        }
	    }
#else /* CF_SFIO */
	    if ((rs = bwrite(op->fp,lbuf,len)) >= 0) {
	        wlen = rs ;
	        if (op->f.bufline && hasnl(lbuf,len)) {
	            rs = bflush(op->fp) ;
		}
	    }
#endif /* CF_SFIO */

	} /* end if */

ret0:

#if	CF_DEBUGS
	debugprintf("shio_vprintf: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (shio_vprintf) */


int shio_putc(SHIO *op,int ch)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SHIO_MAGIC) return SR_NOTOPEN ;

	if (op->f.nullfile) goto ret0 ;

#if	CF_SFIO
	if (op->f.sfio) {
	    char	buf[2] ;
	    buf[0] = ch ;
	    rs = shio_sfcheckwrite(op,buf,1) ;
	} else {
	    rs = bputc(op->fp,ch) ;
	    if ((rs >= 0) && op->f.bufline && (ch == '\n')) {
	        bflush(op->fp) ;
	    }
	}
#else /* CF_SFIO */
	if ((rs = bputc(op->fp,ch)) >= 0) {
	    if (op->f.bufline && (ch == '\n')) {
	        bflush(op->fp) ;
	    }
	}
#endif /* CF_SFIO */

ret0:
	return (rs >= 0) ? 1 : rs ;
}
/* end subroutine (shio_putc) */


int shio_seek(SHIO *op,offset_t o,int w)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SHIO_MAGIC) return SR_NOTOPEN ;

	if (op->f.nullfile) goto ret0 ;

#if	CF_SFIO
	if (op->f.sfio) {
	    if ((rs = shio_sfcookflush(op)) >= 0) {
	        Sfoff_t	sfo ;
	        sfo = sfseek(op->fp,(Sfoff_t) o,w) ;
	        rs = (sfo >= 0) ? ((int) (sfo&INT_MAX)) : SR_NOTSEEK ;
	    }
	} else
	    rs = bseek(op->fp,o,w) ;
#else
	rs = bseek(op->fp,o,w) ;
#endif /* CF_SFIO */

ret0:
	return rs ;
}
/* end subroutine (shio_seek) */


int shio_flush(SHIO *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SHIO_MAGIC) return SR_NOTOPEN ;

	if (! op->f.nullfile) {

#if	CF_SFIO
	    if (op->f.sfio) {
	        rs = shio_sfcookflush(op) ;
	    } else
	        rs = bflush(op->fp) ;
#else
	    rs = bflush(op->fp) ;
#endif

	} /* end if (output enabled) */

	return rs ;
}
/* end subroutine (shio_flush) */


int shio_control(SHIO *op,int cmd,...)
{
	int		rs = SR_OK ;
	int		fd ;
	int		f ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SHIO_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("shio_control: cmd=%u\n",cmd) ;
#endif

	{
	    va_list	ap ;
	    va_begin(ap,cmd) ;
	    switch (cmd) {
	    case SHIO_CNOP:
	        break ;
	    case SHIO_CSETBUFWHOLE:
	        if (! op->f.nullfile) {
	            f = (int) va_arg(ap,int) ;
#if	CF_SFIO
	            if (op->f.sfio) {
	                if ((rs = shio_sfcookline(op,FALSE)) >= 0) {
	                    int	flags = SF_WHOLE ;
	                    int	sfcmd = (f) ? SFIO_ON : SFIO_OFF ;
	                    rs = sfset(op->fp,flags,sfcmd) ;
	                }
	            } else {
	                rs = bcontrol(op->fp,BC_SETBUFWHOLE,TRUE) ;
		    }
#else
	            rs = bcontrol(op->fp,BC_SETBUFWHOLE,TRUE) ;
#endif /* CF_SFIO */
	        }
	        break ;
	    case SHIO_CSETBUFLINE:
	        if (! op->f.nullfile) {
	            f = (int) va_arg(ap,int) ;
	            op->f.bufline = f ;
#if	CF_SFIO
	            if (op->f.sfio) {
	                if ((rs = shio_sfcookline(op,f)) >= 0) {
	                    int	flags = SF_LINE ;
	                    int	sfcmd = (f) ? SFIO_ON : SFIO_OFF ;
	                    rs = sfset(op->fp,flags,sfcmd) ;
	                }
	            } else {
	                rs = bcontrol(op->fp,BC_SETBUFLINE,f) ;
		    }
#else
	            rs = bcontrol(op->fp,BC_SETBUFLINE,f) ;
#endif /* CF_SFIO */
	        }
	        break ;
	    case SHIO_CSETBUFNONE:
	        if (! op->f.nullfile) {
	            op->f.bufnone = TRUE ;
	            op->f.bufline = FALSE ;
#if	CF_SFIO
	            if (op->f.sfio) {
	                if ((rs = shio_sfcookline(op,FALSE)) >= 0) {
	                    int	flags = (SF_LINE | SF_WHOLE) ;
	                    rs = sfset(op->fp,flags,SFIO_OFF) ;
	                }
	            } else {
	                rs = bcontrol(op->fp,BC_SETBUFNONE,TRUE) ;
		    }
#else
	            rs = bcontrol(op->fp,BC_SETBUFNONE,TRUE) ;
#endif /* CF_SFIO */
	        }
	        break ;
	    case SHIO_CSETBUFDEF:
	        if (! op->f.nullfile) {
	            f = (int) va_arg(ap,int) ;
	            op->f.bufline = op->f.terminal ;
#if	CF_SFIO
	            if (op->f.sfio) {
	                if ((rs = shio_sfcookline(op,TRUE)) >= 0) {
	                    int	flags ;
	                    flags = SF_WHOLE ;
	                    if (! op->f.terminal) flags |= SF_LINE ;
	                    int	sfcmd = (f) ? SFIO_ON : SFIO_OFF ;
	                    rs = sfset(op->fp,flags,sfcmd) ;
	                }
	            } else {
	                rs = bcontrol(op->fp,BC_SETBUFDEF,TRUE) ;
		    }
#else
	            rs = bcontrol(op->fp,BC_LINEBUF,0) ;
#endif /* CF_SFIO */
	        }
	        break ;
	    case SHIO_CSETFLAGS:
	        if (! op->f.nullfile) {
	            f = (int) va_arg(ap,int) ;
	            op->f.bufline = f ;
#if	CF_SFIO
	            if (op->f.sfio) {
	                int	flags = SF_LINE ;
	                int	sfcmd = (f) ? SFIO_ON : SFIO_OFF ;
	                rs = sfset(op->fp,flags,sfcmd) ;
	            } else {
	                rs = bcontrol(op->fp,BC_LINEBUF,0) ;
		    }
#else
	            rs = bcontrol(op->fp,BC_LINEBUF,0) ;
#endif /* CF_SFIO */
	        }
	        break ;
	    case SHIO_CFD:
	        if (! op->f.nullfile) {
#if	CF_SFIO
	            if (op->f.sfio) {
	                rs = sffileno(op->fp) ;
	                if (rs < 0)
	                    rs = SR_NOTOPEN ;
	            } else {
	                rs = bcontrol(op->fp,BC_FD,&fd) ;
	                if (rs >= 0)
	                    rs = fd ;
	            }
#else
	            if ((rs = bcontrol(op->fp,BC_FD,&fd)) >= 0) {
	                rs = fd ;
		    }
#endif /* CF_SFIO */
	            {
	                int	*aip = (int *) va_arg(ap,int *) ;
	                if (aip != NULL) *aip = rs ;
	            }
	        }
	        break ;
	    case SHIO_CNONBLOCK:
	        if (! op->f.nullfile) {
	            int	v = (int) va_arg(ap,int) ;
	            int	f ;
	            f = (v > 0) ;
#if	CF_SFIO
	            if (op->f.sfio) {
	                rs = SR_OK ;
	            } else {
	                rs = bcontrol(op->fp,BC_NONBLOCK,f) ;
	            }
#else
	            rs = bcontrol(op->fp,BC_NONBLOCK,f) ;
#endif /* CF_SFIO */
	        }
	        break ;
	    case SHIO_CSTAT:
	        {
	            struct ustat *sbp = (struct ustat *) va_arg(ap,void *) ;
	            if (! op->f.nullfile) {
#if	CF_SFIO
	                if (op->f.sfio) {
	                    rs = SR_OK ;
	                } else {
	                    rs = bcontrol(op->fp,BC_STAT,sbp) ;
	                }
#else
	                rs = bcontrol(op->fp,BC_STAT,sbp) ;
#endif /* CF_SFIO */
	            } else {
	                memset(sbp,0,sizeof(struct ustat)) ;
		    }
	        }
	        break ;
	    default:
	        rs = SR_INVALID ;
	        break ;
	    } /* end switch */
	    va_end(ap) ;
	} /* end block */

#if	CF_DEBUGS
	debugprintf("shio_control: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (shio_control) */


int shio_getfd(SHIO *op)
{
	int		rs = SR_OK ;
	int		fd = -1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SHIO_MAGIC) return SR_NOTOPEN ;

	if (! op->f.nullfile) {

#if	CF_SFIO
	if (op->f.sfio) {
	    rs = sffileno(op->fp) ;
	    fd = rs ;
	    if (rs < 0)
	        rs = SR_NOTOPEN ;
	} else {
	    rs = bcontrol(op->fp,BC_FD,&fd) ;
	}
#else /* CF_SFIO */
	rs = bcontrol(op->fp,BC_FD,&fd) ;
#if	CF_DEBUGS
	debugprintf("shio_getfd: bcontrol() rs=%d fd=%u\n",rs,fd) ;
#endif
#endif /* CF_SFIO */

	} else
	    rs = SR_NOSYS ;

#if	CF_DEBUGS
	debugprintf("shio_getfd: mid rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (shio_getfd) */


int shio_getlines(SHIO *op)
{
	int		rs ;
	if ((rs = shio_isterm(op)) > 0) {
	   if ((rs = shio_getfd(op)) >= 0) {
	        rs = tcgetlines(rs) ;
	   }
	}
	return rs ;
}
/* end subroutine (shio_getlines) */


int shio_readintr(SHIO *op,void *ubuf,int ulen,int to,volatile int **ipp)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("shio_readintr: ent ulen=%u to=%d\n",ulen,to) ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SHIO_MAGIC) return SR_NOTOPEN ;

	if (op->f.nullfile) goto ret0 ;

	if (to < 0) to = INT_MAX ;

	while (rs >= 0) {
	    int	rto = (to >= 0) ? MIN(to,1) : to ;

	    if (isInterrupt(ipp)) break ;

	    rs = shio_reade(op,ubuf,ulen,rto,FM_TIMED) ;

#if	CF_DEBUGS
	    debugprintf("shio_readintr: shio_reade() rs=%d\n",rs) ;
#endif

	    if (rs != SR_TIMEDOUT) break ;

	    if (to > 0) to -= 1 ;
	    if (to == 0) break ;
	    rs = SR_OK ;

	} /* end while */

#if	CF_DEBUGS
	debugprintf("shio_readintr: mid rs=%d\n",rs) ;
	if (rs >= 0) {
	    debugprintf("shio_readintr: rbuf=>%t<\n",
	        ubuf,strlinelen(ubuf,rs,60)) ;
	}
#endif

	if (rs >= 0) {
	    if (isInterrupt(ipp)) rs = SR_INTR ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("shio_readintr: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (shio_readintr) */


int shio_reserve(SHIO *op,int amount)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SHIO_MAGIC) return SR_NOTOPEN ;

#if	CF_SFIO
	if (op->f.sfio) {
	    rs = SR_OK ;
	} else {
	    rs = breserve(op->fp,amount) ;
	}
#else
	rs = breserve(op->fp,amount) ;
#endif /* CF_SFIO */

	return rs ;
}
/* end subroutine (shio_reserve) */


int shio_isterm(SHIO *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SHIO_MAGIC) return SR_NOTOPEN ;

	if (! op->f.nullfile) {
#if	CF_SFIO
	    if (op->f.sfio) {
	        int	rc = sfisterm(op->fp) ;
	        rs = (rc >= 0) ? rc : SR_BADF ;
	    } else {
	        rs = bisterm(op->fp) ;
	    }
#else
	    rs = bisterm(op->fp) ;
#endif /* CF_SFIO */
	}

	return rs ;
}
/* end subroutine (shio_isterm) */


int shio_isseekable(SHIO *op)
{
	const int	w = SEEK_CUR ;
	int		rs ;
	int		f = FALSE ;

	if ((rs = shio_seek(op,0L,w)) >= 0) {
	    f = TRUE ;
	} else if (isNotSeek(rs))
	    rs = SR_OK ;

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (shio_isseekable) */


int shio_stat(SHIO *op,struct ustat *sbp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (sbp == NULL) return SR_FAULT ;

	if (op->magic != SHIO_MAGIC) return SR_NOTOPEN ;

	rs = shio_control(op,SHIO_CSTAT,sbp) ;
	return rs ;
}
/* end subroutine (shio_stat) */


int shio_writefile(SHIO *op,const char *fname)
{
	bfile		ifile, *ifp = &ifile ;
	const mode_t	om = 0664 ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (op->magic != SHIO_MAGIC) return SR_NOTOPEN ;

	if ((rs = bopen(ifp,fname,"r",om)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    char	lbuf[LINEBUFLEN+1] ;
	    while ((rs = bread(ifp,lbuf,llen)) > 0) {
	        rs = shio_write(op,lbuf,rs) ;
	        wlen += rs ;
	        if (rs < 0) break ;
	    } /* end while */
	    rs1 = bclose(ifp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bopen) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (shio_writefile) */


/* private subroutines */


static int shio_bopene(SHIO *op,int fni,cchar *fname,cchar *ms,mode_t om,int to)
{
	const int	size = sizeof(bfile) ;
	int		rs ;
	char		nms[SHIO_MODESTRLEN + 1] ;
	void		*p ;

#if	CF_DEBUGS
	debugprintf("shio_bopene: stat-size=%u\n",sizeof(struct ustat)) ;
	debugprintf("shio_bopene: ustat-size=%u\n",sizeof(struct ustat)) ;
	debugprintf("shio_bopene: offset-size=%u\n",sizeof(offset_t)) ;
	debugprintf("shio_bopene: ino-size=%u\n",sizeof(uino_t)) ;
	debugprintf("shio_bopene: dev-size=%u\n",sizeof(dev_t)) ;
	debugprintf("shio_bopene: mode-size=%u\n",sizeof(mode_t)) ;
	debugprintf("shio_bopene: LONG-size=%u\n",sizeof(LONG)) ;
	debugprintf("shio_bopene: BFILEOFF-size=%u\n",sizeof(BFILE_OFF)) ;
	debugprintf("shio_bopene: BFILESTAT-size=%u\n",sizeof(BFILE_STAT)) ;
#endif

#if	CF_DEBUGS
	debugprintf("shio_bopene: ent fni=%d\n",fni) ;
	if (fni < 0) {
	    debugprintf("shio_bopene: fname=%s\n",fname) ;
	} else
	    debugprintf("shio_bopene: fname=*FD%u*\n",fni) ;
#endif

	if ((rs = uc_malloc(size,&p)) >= 0) {
	    op->bfp = p ;
	    op->fp = p ;
	    if (fni >= 0) {
	        if ((rs = sncpy2(nms,SHIO_MODESTRLEN,"d",ms)) >= 0) {
	            rs = bopene(op->fp,fname,nms,om,to) ;
	        }
	    } else {
	        rs = bopene(op->fp,fname,ms,om,to) ;
	    }
	    if (rs < 0) {
	        uc_free(op->bfp) ;
	        op->bfp = NULL ;
	        op->fp = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("shio_bopene: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (shio_bopene) */


static int shio_bclose(SHIO *op)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("shio_bclose: ent\n") ;
#endif

	if (op->fp != NULL) {
#if	CF_DEBUGS
	    debugprintf("shio_bclose: bclose()\n") ;
#endif
	    rs = bclose(op->fp) ;
#if	CF_DEBUGS
	    debugprintf("shio_bclose: bclose() rs=%d\n",rs) ;
#endif
	    if (op->bfp != NULL) {
#if	CF_DEBUGS
	        debugprintf("shio_bclose: uc_free() a=%p\n",op->bfp) ;
#endif
	        uc_free(op->bfp) ;
	        op->bfp = NULL ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("shio_bclose: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (shio_bclose) */


#if	CF_SFIO
static int shio_shprintln(SHIO *op,cchar *lbuf,int llen)
{
	int		rs ;
	int		wlen = 0 ;
	if (llen < 0) llen = strlen(lbuf) ;
	if ((rs = shio_sfiscook(op)) > 0) {
	    if ((rs = shio_sfcookwrite(op,lbuf,llen)) >= 0) {
	        wlen += rs ;
	        if ((llen == 0) || (lbuf[llen-1] != '\n')) {
		    rs = shio_sfcookwrite(op,"\n",1) ;
	            wlen += rs ;
		}
	    }
	} else if (rs == 0) {
	    if (llen > 0) {
	        rs = shio_sfwrite(op,lbuf,llen) ;
	        wlen += rs ;
	    }
	    if ((rs >= 0) && ((llen == 0) || (lbuf[llen-1] != '\n'))) {
	        char	eol[2] ;
	        eol[0] = '\n' ;
	        eol[1] = '\0' ;
	        shio_sfwrite(op,eol,1) ;
	        wlen += rs ;
	    }
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (shio_shprintln) */
#endif /* CF_SFIO */


#if	CF_SFIO

static int shio_sfiscook(SHIO *op)
{
	int		rs = SR_OK ;
	int		f = FALSE ;
	f = f || op->f.terminal ;
	f = f || op->f.bufline ;
	f = f || op->f.bufnone ;
	if (f) {
	    rs = shio_sfcookline(op,f) ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (shio_sfiscook) */

static int shio_sfflush(SHIO *op)
{
	sfsync(op->fp) ;
	return SR_OK ;
}
/* end subroutine (shio_sfflush) */

static int shio_sfcookline(SHIO *op,int f)
{
	int		rs ;
	if (f) {
	    rs = shio_sfcookbegin(op) ;
	} else {
	    rs = shio_sfcookend(op) ;
	}
	return rs ;
}
/* end subroutine (shio_sfcookline) */

static int shio_sfcookbegin(SHIO *op)
{
	int		rs = SR_OK ;
	if (op->outstore == NULL) {
	    const int	size = sizeof(OUTSTORE) ;
	    void	*p ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        OUTSTORE	*osp = (OUTSTORE *) p ;
	        op->outstore = p ;
	        rs = outstore_start(osp) ;
	        if (rs < 0) {
	            uc_free(op->outstore) ;
	            op->outstore = NULL ;
	        }
	    } /* end if (m-a) */
	} /* end if (needed) */
	return rs ;
}
/* end subroutine (shio_sfcookbegin) */

static int shio_sfcookend(SHIO *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->outstore != NULL) {
	    OUTSTORE	*osp = op->outstore ;
	    rs1 = shio_sfcookdump(op) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = outstore_finish(osp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs = uc_free(op->outstore) ;
	    if (rs >= 0) rs = rs1 ;
	    op->outstore = NULL ;
	}
	return rs ;
}
/* end subroutine (shio_sfcookend) */

static int shio_sfcookdump(SHIO *op)
{
	int		rs = SR_OK ;
	if (op->outstore != NULL) {
	    OUTSTORE	*osp = (OUTSTORE *) op->outstore ;
	    const char	*cp ;
	    if ((rs = outstore_get(osp,&cp)) > 0) {
	        if ((rs = shio_sfwrite(op,cp,rs)) >= 0) {
	            rs = outstore_clear(osp) ;
	        }
	    }
	}
	return rs ;
}
/* end subroutine (shio_sfcookflush) */

static int shio_sfcookflush(SHIO *op)
{
	int		rs = SR_OK ;
	if (op->outstore != NULL) {
	    if ((rs = shio_sfcookdump(op)) >= 0) {
	        rs = shio_sfflush(op) ;
	    }
	} else {
	    rs = shio_sfflush(op) ;
	}
	return rs ;
}
/* end subroutine (shio_sfcookflush) */

static int shio_sfcheckwrite(SHIO *op,cchar *lbuf,int llen)
{
	int		rs ;
	if (llen < 0) llen = strlen(lbuf) ;
	if ((rs = shio_sfiscook(op)) > 0) {
	    rs = shio_sfcookwrite(op,lbuf,llen) ;
	} else if (rs == 0) {
	    rs = shio_sfwrite(op,lbuf,llen) ;
	}
	return (rs >= 0) ? llen : rs ;
}
/* end subroutine (shio_sfcheckwrite) */

static int shio_sfcookwrite(SHIO *op,cchar *lbuf,int llen)
{
	int		rs = SR_OK ;
	if ((op->outstore != NULL) && (llen > 0)) {
	    OUTSTORE	*osp = (OUTSTORE *) op->outstore ;
	    int		ll = llen ;
	    cchar	*tp ;
	    cchar	*lp = lbuf ;
	    if ((tp = strnchr(lp,ll,CH_NL)) != NULL) {
	        if ((rs = outstore_strw(osp,lp,((tp+1)-lp))) >= 0) {
	            cchar	*cp ;
	            if ((rs = outstore_get(osp,&cp)) > 0) {
	                if ((rs = shio_sfwrite(op,cp,rs)) >= 0) {
	                    rs = outstore_clear(osp) ;
	                }
	            }
	            ll -= ((tp+1)-lp) ;
	            lp = (tp+1) ;
	            if ((rs >= 0) && (ll > 0)) {
	                while ((tp = strnchr(lp,ll,CH_NL)) != NULL) {
	                    rs = shio_sfwrite(op,lp,(tp+1-lp)) ;
	                    ll -= ((tp+1)-lp) ;
	                    lp = (tp+1) ;
	                    if (rs < 0) break ;
	                } /* end while */
	            } /* end if (handle remaining) */
		    if ((rs >= 0) && op->f.bufline) {
	        	rs = shio_sfflush(op) ;
		    }
	        } /* end if (outstore_strw) */
	    } /* end if (had a NL) */
	    if ((rs >= 0) && (ll > 0)) {
	        rs = outstore_strw(osp,lp,ll) ;
	    }
	} else if (llen > 0) {
	    rs = shio_sfwrite(op,lbuf,llen) ;
	}
	return (rs >= 0) ? llen : rs ;
}
/* end subroutine (shio_sfcookwrite) */
#endif /* CF_SFIO */


#if	CF_SFIO
static int shio_sfwrite(SHIO *op,cchar *lbuf,int llen)
{
	int		rs ;
	if ((rs = sfwrite(op->fp,lbuf,llen)) < 0) rs = SR_PIPE ;
	return rs ;
}
/* end subroutine (shio_sfwrite) */
#endif /* CF_SFIO */


#if	CF_SFIO
/* are we outside of KSH? */
static int shio_outside()
{
	SHIO_PERSISTENT	*pdp = &shio_data ;
	int		f = pdp->f_outside ;
	if (! pdp->f_init) { /* race is OK here */
	    if (dlsym(RTLD_DEFAULT,SHIO_KSHSYM) == NULL) {
	        pdp->f_outside = TRUE ;
	        f = TRUE ;
	    }
	    pdp->f_init = TRUE ;
	} /* end if (needed initialization) */
	return f ;
}
/* end subroutine (shio_outside) */
#endif /* CF_SFIO */


static int hasnl(cchar *sp,int sl)
{
	return (strnrchr(sp,sl,'\n') != NULL) ;
}
/* end subroutine (hasnl) */


static int isNotSeek(int rs)
{
	return isOneOf(seekrs,rs) ;
}
/* end subroutine (isNotSeek) */


static int isInterrupt(volatile int **ipp)
{
	int		f = FALSE ;

	if (ipp != NULL) {
	    int	i = 0 ;
	    for (i = 0 ; ipp[i] != NULL ; i += 1) {
	        f = (ipp[i][0] != 0) ;
	        if (f) break ;
	    }
	}

	return f ;
}
/* end subroutine (isInterrupt) */


