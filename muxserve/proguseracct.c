/* proguseracct */

/* handle some login-based stuff */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_LINEFOLD	0		/* use 'linefold(3dam)' */
#define	CF_CLEAN	0		/* clean lines */
#define	CF_CLEANBEFORE	1		/* clean before (else after) */


/* revision history:

	= 1998-02-23, David A­D­ Morano
	This was written up (from scratch) for the new FINGERS program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We're going to make this pretty cheap since we do not get a lot of
        Finger requests! That pretty much says it right there.

        OK, we are at least not going to let (or try not to let) garbage
        characters get out to the network. Since we are readling files that are
        maintained by users on the local system, those files can have junk in
        them. Sending junk out to the network does not present the image (a nice
        one) that we want to protray. So we try to clean up any junkd that we
        find in user files (like "project" and "plan" files).

        Also, FINGER responses are generally assumed to occupy no more than 80
        columns of output (not necessarily on a terminal of any kind either). So
        we also try to fix-clean this up.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<filebuf.h>
#include	<ascii.h>
#include	<linefold.h>
#include	<getax.h>
#include	<passwdent.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"clientinfo.h"


/* local defines */

#define	LOGBUFLEN	(LOGNAMELEN + 20)

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	O_FLAGS		(O_CREAT | O_RDWR | O_TRUNC)

#ifndef	CHAR_ISEND
#define	CHAR_ISEND(c)	(((c) == '\r') || ((c) == '\n'))
#endif

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#undef	NINDENT
#define	NINDENT		4

#undef	NBLANKS
#define	NBLANKS		8

#ifndef	TO_OPEN
#define	TO_OPEN		20		/* seconds */
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	isprintlatin(int) ;
extern int	isNotPresent(int) ;

extern int	proglog_printf(PROGINFO *,cchar *,...) ;
extern int	proglog_flush(PROGINFO *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	proguseracct_usersrv(PROGINFO *,CLIENTINFO *,
PASSWDENT *) ;
static int	proguseracct_pop(PROGINFO *,int,PASSWDENT *,
const char *,const char *) ;
static int	proguseracct_projinfo(PROGINFO *,int,
PASSWDENT *) ;
static int	proguseracct_copyover(PROGINFO *,int,int) ;
static int	proguseracct_prints(PROGINFO *,FILEBUF *,
char *,int, const char *,int) ;
static int	proguseracct_print(PROGINFO *,FILEBUF *,int,int,
const char *,int) ;

static int	filebuf_writeblanks(FILEBUF *,int) ;

#if	CF_LINEFOLD
#else
static int	getline(int,const char *,int) ;
#endif

#if	CF_CLEAN && CF_CLEANBEFORE
static int	mkclean(char *,const char *,int) ;
static int	isourbad(int) ;
#endif /* CF_CLEANBEFORE */

#ifdef	COMMENT
static int	isend(int) ;
#endif


/* local variables */

static cchar	blanks[NBLANKS] = "        " ;


/* exported subroutines */


int proguseracctmat(PROGINFO *pip,PASSWDENT *pep,char *pwbuf,int pwlen,
		cchar *svcname)
{
	int		rs ;

	if (svcname == NULL) return SR_FAULT ;

	if (svcname[0] == '\0') return SR_INVALID ;

	rs = getpw_name(pep,pwbuf,pwlen,svcname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("proguseracctmat: ret rs=%d svc=%s\n",rs,svcname) ;
#endif

	return rs ;
}
/* end subroutine (proguseracctmat) */


int proguseracctexec(PROGINFO *pip,CLIENTINFO *cip,PASSWDENT *pep)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		ofd ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("proguseracctexec: login=%s\n",pep->pw_name) ;
	    debugprintf("proguseracctexec: subsvc=%s\n",cip->subservice) ;
	}
#endif

	ofd = cip->fd_output ;
	if (ofd < 0)
	    return SR_INVALID ;

	if ((cip->subservice != NULL) && (cip->subservice[0] != '\0')) {

	    rs = proguseracct_usersrv(pip,cip,pep) ;
	    if (rs > 0) c = rs ;

	} else {

/* project file */

	    if (rs >= 0) {
	        rs1 = proguseracct_pop(pip,ofd,pep,PROJFNAME,"project") ;
	        if (rs1 <= 0)
	            rs1 = proguseracct_projinfo(pip,ofd,pep) ;
	        if (rs1 > 0)
	            c += 1 ;
	    } /* end if (project) */

/* plan file */

	    if (rs >= 0) {
	        rs1 = proguseracct_pop(pip,ofd,pep,PLANFNAME,"plan") ;
	        if (rs1 > 0)
	            c += 1 ;
	    } /* end if (plan) */

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("proguseracctexec: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proguseracctexec) */


/* local subroutines */


static int proguseracct_usersrv(PROGINFO *pip,CLIENTINFO *cip,PASSWDENT *pep)
{
	const int	ofd = cip->fd_output ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		fd = -1 ;
	int		to = TO_OPEN ;
	int		wlen = 0 ;
	int		f_served = FALSE ;
	const char	*usersrv = pip->usersrv ;
	char		tmpfname[MAXPATHLEN + 1], *ssbuf = tmpfname ;

	if ((usersrv == NULL) || (usersrv[0] == '\0'))
	    usersrv = USERSRV ;

	if ((rs1 = mkpath2(tmpfname,pep->pw_dir,usersrv)) >= 0) {
	    const int	of = (O_RDWR | O_NDELAY | O_NOCTTY) ;
	    rs1 = uc_opene(tmpfname,of,0666,to) ;
	    fd = rs1 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("proguseracct_usersrv: uc_opene() rs=%d\n",rs1) ;
#endif

	if (rs1 >= 0) {

	    rs1 = sncpy2(ssbuf,MAXPATHLEN,cip->subservice,"\r\n") ;

	    if (rs1 >= 0) rs1 = uc_writen(fd,ssbuf,rs1) ;

	    if (rs1 >= 0) {
	        f_served = TRUE ;
	        rs = proguseracct_copyover(pip,ofd,fd) ;
	        wlen = rs ;
	    }

	    u_close(fd) ;

	    if (rs >= 0) {
	        if (pip->open.logprog)
	            proglog_printf(pip,"ufinger wlen=%u",wlen) ;
	        if (pip->debuglevel > 0)
	            bprintf(pip->efp,"%s: ufinger wlen=%u",
	                pip->progname,wlen) ;
	    }

	} /* end if (opened source file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("proguseracct_usersrv: ret rs=%d f=%u\n",rs,f_served) ;
#endif

	return (rs >= 0) ? f_served : rs ;
}
/* end subroutine (proguseracct_usersrv) */


static int proguseracct_pop(PROGINFO *pip,int ofd,PASSWDENT *pep,
		cchar *fname,cchar *desc)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] != '\0') {
	    const int	to = TO_OPEN ;
	    int		fd = -1 ;
	    char	tmpfname[MAXPATHLEN + 1] ;

	    if ((rs1 = mkpath2(tmpfname,pep->pw_dir,fname)) >= 0) {
	        const int	of = (O_RDONLY | O_NDELAY | O_NOCTTY) ;
	        rs1 = uc_opene(tmpfname,of,0666,to) ;
	        fd = rs1 ;
	    }

	    if (rs1 >= 0) {

	        c = 1 ;
	        rs = proguseracct_copyover(pip,ofd,fd) ;

	        u_close(fd) ;

	        if ((rs >= 0) && pip->open.logprog && (desc != NULL)) {
	            proglog_printf(pip,"%s=%u",desc,rs) ;
		}

	    } /* end if (opened source file) */

	} /* end if (non-empty file-name) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proguseracct_pop) */


static int proguseracct_projinfo(PROGINFO *pip,int ofd,PASSWDENT *pep)
{
	struct project	pj ;
	const int	pjlen = getbufsize(getbufsize_pj) ;
	int		rs ;
	int		c = 0 ;
	char		*pjbuf ;

	if ((rs = uc_malloc((pjlen+1),&pjbuf)) >= 0) {
	    cchar	*n = pep->pw_name ;
	    if ((rs = uc_getdefaultproj(n,&pj,pjbuf,pjlen)) >= 0) {
	        const int	outlen = (pjlen*2) ;
	        if (pj.pj_comment != NULL) {
	            int		ol ;
	            char	outbuf[outlen+1] ;

	            ol = sncpy1(outbuf,outlen,pj.pj_comment) ;
	            if ((ol > 0) && (ol < outlen)) {
	                c = 1 ;
	                outbuf[ol++] = '\n' ;
	                rs = uc_writen(ofd,outbuf,ol) ;
	            }

	        } /* end if (non-null) */
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	    uc_free(pjbuf) ;
	} /* end if (memory-allocation) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proguseracct_projinfo) */


static int proguseracct_copyover(PROGINFO *pip,int ofd,int fd)
{
	FILEBUF		local, net ;
	const int	clen = LINEBUFLEN ;
	const int	to = TO_READ ;
	int		rs = SR_OK ;
	int		rs1 = 0 ;
	int		size ;
	int		cbl ;
	int		wlen = 0 ;
	char		*cbuf = NULL ;

	cbl = ((pip->linelen >= 5) ? pip->linelen : COLUMNS) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("proguseracct_copyover: cbl=%u<\n",cbl) ;
#endif

	size = (clen + 2) ; /* plus two characters: NL and NUL */
	if ((rs = uc_malloc(size,&cbuf)) >= 0) {

	    if ((rs = filebuf_start(&net,ofd,0L,0,0)) >= 0) {

	        if ((rs = filebuf_start(&local,fd,0L,0,0)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 2] ;

	            while ((rs = filebuf_readline(&local,lbuf,llen,to)) > 0) {
	                int	len = rs ;
	                int	cll ;
	                cchar	*clp ;

#if	CF_CLEAN
#if	CF_CLEANBEFORE
	                {
	                    int	i, ch ;
	                    clp = lbuf ;
	                    cll = mkclean(cbuf,lbuf,len) ;
	                    for (i = 0 ; i < cll ; i += 1) {
	                        ch = cbuf[i] ;
	                        lbuf[i] = (char *) (ch != '\t') ? ch : ' ' ;
	                    }
	                }
#else
	                clp = lbuf ;
	                cll = len ;
#endif /* CF_CLEANBEFORE */
#else /* CF_CLEAN */
	                clp = lbuf ;
	                cll = len ;
#endif /* CF_CLEAN */

#if	CF_DEBUG
	                if (DEBUGLEVEL(5))
	                    debugprintf("proguseracct_copyover: line=>%t<\n",
	                        clp,strnlen(clp,cll,40)) ;
#endif

	                rs1 = proguseracct_prints(pip,&net,cbuf,cbl,clp,cll) ;
	                wlen += rs1 ;
	                if (rs1 < 0) break ;

	            } /* end while */

	            filebuf_finish(&local) ;
	        } /* end if (local) */

	        filebuf_finish(&net) ;
	    } /* end if (net) */

	    uc_free(cbuf) ;
	} /* end if (memory-allocation) */

	if (rs1 < 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if (pip->debuglevel > 0) {
	        fmt = "%s: error on network write (%d)\n" ;
	        bprintf(pip->efp,fmt,pn,rs1) ;
	    }
	    if (pip->open.logprog) {
	        fmt = "error on network write (%d)" ;
	        proglog_printf(pip,fmt,rs1) ;
	    }
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (proguseracct_copyover) */


static int proguseracct_prints(PROGINFO *pip,FILEBUF *fbp,char *cbuf,int cbl,
		cchar *lbuf,int llen)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		ll ;
	int		ncols = cbl ;
	int		ln = 0 ;
	int		ind = NINDENT ;
	int		wlen = 0 ;
	const char	*lp ;

	ncols = MIN(cbl,COLUMNS) ;

#if	CF_LINEFOLD
	{
	    LINEFOLD	liner ;

	    if ((rs = linefold_start(&liner,ncols,ind,lbuf,llen)) >= 0) {

	        for (ln = 0 ; (ll = linefold_get(&liner,ln,&lp)) >= 0 ; 
	            ln += 1) {

#if	CF_CLEAN
#if	CF_CLEANBEFORE
#else
	            if (ll > 0) {
	                len = mkclean(cbuf,lp,ll) ;
	                lp = cbuf ;
	                ll = len ;
	            }
#endif /* CF_CLEANBEFORE */
#endif /* CF_CLEAN */

	            rs1 = proguseracct_print(pip,fbp,ind,ln++,lp,ll) ;
	            wlen += rs1 ;

	            if (rs1 < 0) break ;
	        } /* end while */

	        linefold_finish(&liner) ;
	    } /* end if (linefold) */

	} /* end block */
#else /* CF_LINEFOLD */
	{
	    int		sl = llen ;
	    int		cll ;
	    int		i = 0 ;
	    cchar	*sp = lbuf ;
	    cchar	*clp ;

	    while ((ll = getline(ncols,sp,sl)) >= 0) {
	        if ((ll == 0) && (i > 0)) break ;
	        lp = sp ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("proguseracct_prints: line=>%t<\n",
	                lp,strnlen(lp,ll,40)) ;
#endif

#if	CF_CLEAN
	        cll = mkclean(cbuf,lp,ll) ;
	        clp = cbuf ;
#else
	        cll = ll ;
	        clp = lp ;
#endif /* CF_CLEAN */


#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("proguseracct_prints: clean=>%t<\n",
	                clp,strnlen(clp,cll,40)) ;
#endif

	        rs1 = proguseracct_print(pip,fbp,ind,ln++,clp,cll) ;
	        wlen += rs1 ;
	        if (rs1 < 0) break ;

	        sl -= ((lp + ll) - sp) ;
	        sp = (lp + ll) ;

	        i += 1 ;
	        ncols = (cbl-ind) ;
	    } /* end while */

	} /* end block */
#endif /* CF_LINEFOLD */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (proguseracct_prints) */


static int proguseracct_print(PROGINFO *pip,FILEBUF *fbp,int indent,
		int ln,cchar *lp,int ll)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	int		f_eol ;

	if (ll > 0) {
	    if (indent < 0) indent = 0 ;

	    f_eol = ((ll > 0) && (lp[ll-1] == '\n')) ;

	    if ((ln > 0) && (indent > 0)) {
	        rs = filebuf_writeblanks(fbp,indent) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = filebuf_write(fbp,lp,ll) ;
	        wlen += rs ;
	    }

	    if ((rs >= 0) && (! f_eol)) {
	        rs = filebuf_write(fbp,"\n",1) ;
	        wlen += 1 ;
	    }

	} /* end if (positive) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (proguseracct_print) */


static int filebuf_writeblanks(FILEBUF *fbp,int indent)
{
	int		rs = SR_OK ;
	int		ml ;
	int		wlen = 0 ;

	while ((rs >= 0) && (wlen < indent)) {
	    ml = MIN((indent-wlen),NBLANKS) ;
	    rs = filebuf_write(fbp,blanks,ml) ;
	    wlen += rs ;
	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (proguseracct_writeblanks) */


#if	CF_LINEFOLD
#else

static int getline(int llen,cchar *sp,int sl)
{
	int		len = 0 ;
	cchar		*tp ;

	if (sl > 0) {

	    len = MIN(sl,llen) ;
	    if ((tp = strnpbrk(sp,len,"\r\n")) != NULL) {
	        len = ((tp + 1) - sp) ;
	    }

	    if ((len > 0) && (len < sl) && CHAR_ISEND(sp[len])) {
	        len += 1 ;
	        if ((len < sl) && (sp[len-1] == '\r') && (sp[len] == '\n'))
	            len += 1 ;
	    }

	} /* end if (positive) */

	return len ;
}
/* end subroutine (getline) */

#endif /* CF_LINEFOLD */


#if	CF_CLEAN && CF_CLEANBEFORE

static int mkclean(char *cbuf,cchar *lbuf,int llen)
{
	int		i ;
	int		f_eol = FALSE ;

	for (i = 0 ; (i < llen) ; i += 1) {
	    cbuf[i] = lbuf[i] ;
	    if (isourbad(lbuf[i] & UCHAR_MAX)) {
	        cbuf[i] = '_' ;
	    }
	} /* end for */

	if ((i > 0) && (lbuf[i-1] == '\n')) {
	    f_eol = TRUE ;
	}

	if ((! f_eol) && (lbuf[i] != '\n')) {
	    cbuf[i++] = '\n' ;
	}

	return i ;
}
/* end subroutine (mkclean) */

static int isourbad(int ch)
{
	int		f = TRUE ;
	ch &= UCHAR_MAX ;
	switch (ch) {
	case '\n':
	case '\t':
	    f = FALSE ;
	    break ;
	case CH_SO:
	case CH_SI:
	case CH_SS2:
	case CH_SS3:
	    f = TRUE ;
	    break ;
	default:
	    f = (! isprintlatin(ch)) ;
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (isourbad) */

#endif /* CF_CLEANBEFORE */


#ifdef	COMMENT
static int isend(int ch)
{
	int		f = FALSE ;
	f = (ch == '\n') || (ch == '\r') ;
	return f ;
}
/* end subroutine (isend) */
#endif /* COMMENT */


