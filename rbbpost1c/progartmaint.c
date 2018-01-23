/* progartmaint */

/* perform article maintenance on the bb-newgroup spool area */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine performs article maintenance on the articles int the
	PCS BB-newsgroup spool area.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<fsdir.h>
#include	<char.h>
#include	<mailmsg.h>
#include	<mailmsg_enver.h>
#include	<mailmsghdrs.h>
#include	<ema.h>
#include	<emainfo.h>
#include	<dater.h>
#include	<mhcom.h>
#include	<comparse.h>
#include	<nulstr.h>
#include	<sbuf.h>
#include	<buffer.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	STACKADDRLEN
#define	STACKADDRLEN	(2 * MAXHOSTNAMELEN)
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	HDRNAMELEN
#define	HDRNAMELEN	80
#endif

#ifndef	MSGLINELEN
#define	MSGLINELEN	MAX(LINEBUFLEN,(2 * 1024))
#endif

#ifndef	LOGFMTLEN
#define	LOGFMTLEN	LOGFILE_FMTLEN
#endif

#ifndef	MAXMSGLINELEN
#define	MAXMSGLINELEN	76
#endif

#ifndef	TABLEN
#define	TABLEN		8
#endif

#ifndef	DATEBUFLEN
#define	DATEBUFLEN	80
#endif

#undef	BUFLEN
#define	BUFLEN		(2 * 1024)

#undef	NBLANKS
#define	NBLANKS		20

#define	PROCDATA	struct procdata
#define	PROCDATA_FL	struct procdata_flags

#define	ARTINFO		struct artinfo
#define	ARTINFO_FL	struct artinfo_flags


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfsub(const char *,int,const char *,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	strwcmp(cchar *,cchar *,int) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	mkbestaddr(char *,int,const char *,int) ;
extern int	pathadd(char *,int,const char *) ;
extern int	removes(cchar *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	hasuc(const char *,int) ;
extern int	isprintlatin(int) ;
extern int	hasNotDots(const char *,int) ;
extern int	isOneOf(const int *,int) ;
extern int	isNotPresent(int) ;
extern int	isNotValid(int) ;
extern int	isNotAccess(int) ;

extern int	mailmsg_loadline(MAILMSG *,const char *,int) ;
extern int	mailmsg_loadfile(MAILMSG *,bfile *) ;
extern int	ema_haveaddr(EMA *,const char *,int) ;
extern int	emaentry_getbestaddr(EMAENTRY *,const char **) ;

extern int	hdrextnum(const char *,int) ;

extern int	progngdname(PROGINFO *,char *,const char *,int) ;

extern int	proglog_printf(PROGINFO *,cchar *,...) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strwset(char *,int,int) ;
extern char	*timestr_edate(time_t,char *) ;
extern char	*timestr_hdate(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct procdata_flags {
	uint		dummy:1 ;
} ;

struct procdata {
	char		*tdname ;
	MAILMSG		*msgp ;
	PROCDATA_FL	f ;
	int		tdlen ;
} ;

struct artinfo_flags {
	uint		ngema:1 ;
} ;

struct artinfo {
	ARTINFO_FL	f, open ;
	EMA		ngema ;
	char		*tdname ;
	const char	*anp ;
	int		anl ;
	int		ngroups ;
} ;


/* forward references */

static int	procdata_start(PROCDATA *,char *,int) ;
static int	procdata_finish(PROCDATA *) ;

static int	artinfo_start(ARTINFO *,char *,const char *,int) ;
static int	artinfo_ngs(ARTINFO *,MAILMSG *,EMA **) ;
static int	artinfo_finish(ARTINFO *) ;

static int	procmsg(PROGINFO *,PROCDATA *,cchar *,int) ;

static int	procmsger(PROGINFO *,PROCDATA *,const char *,int) ;
static int	procmsgexpires(PROGINFO *,PROCDATA *,ARTINFO *) ;
static int	procmsgmaint(PROGINFO *,PROCDATA *,ARTINFO *) ;
static int	procmsgdel(PROGINFO *,PROCDATA *,ARTINFO *) ;


/* local variables */


/* exported subroutines */


int progartmaint(PROGINFO *pip,struct tdinfo *tip)
{
	PROCDATA	pd ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progartmaint: ent\n") ;
#endif

	if (pip->open.logprog) {
	    cchar	*fmt = "mode expire=%u maint=%u" ;
	    proglog_printf(pip,fmt,pip->f.artexpires,pip->f.artmaint) ;
	}

	if ((rs = procdata_start(&pd,tip->tdname,tip->tdlen)) >= 0) {
	    FSDIR	artdir ;
	    FSDIR_ENT	de ;

	    if ((rs = fsdir_open(&artdir,tip->tdname)) >= 0) {
	        while ((rs = fsdir_read(&artdir,&de)) > 0) {
	            int		el = rs ;
	            cchar	*ep = de.name ;
	            if (hasNotDots(ep,el)) {
	                c += 1 ;
	                rs = procmsg(pip,&pd,ep,el) ;
	            } /* end if (hasNotDots) */
	            if (rs < 0) break ;
	        } /* end while (reading dir entries) */
	        rs1 = fsdir_close(&artdir) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end subroutine (fsdir_open) */

	    rs1 = procdata_finish(&pd) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end subroutine (procdata) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progartmaint: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (progartmaint) */


/* local subroutines */


/* process the current message */
static int procmsg(PROGINFO *pip,PROCDATA *pdp,cchar *ep,int el)
{
	MAILMSG		amsg, *msgp = &amsg ;
	bfile		afile, *afp = &afile ;
	const int	tdlen = pdp->tdlen ;
	int		rs ;
	int		rs1 ;
	char		*tdname = pdp->tdname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progartmaint/procmsg: ent fn=%s\n",tdname) ;
	    debugprintf("progartmaint/procmsg: fn=%t\n",ep,el) ;
	}
#endif

	if ((rs = pathadd(tdname,tdlen,ep)) >= 0) {
	    if (strwcmp("core",ep,el) == 0) {
		rs = removes(tdname) ;
	    } else {
	        if ((rs = bopen(afp,tdname,"r",0666)) >= 0) {
		    USTAT	sb ;
		    if ((rs = bstat(afp,&sb)) >= 0) {
		        if (S_ISREG(sb.st_mode)) {
	        	    if ((rs = mailmsg_start(msgp)) >= 0) {
	            	        pdp->msgp = msgp ;

			        if ((rs = mailmsg_loadfile(msgp,afp)) >= 0) {
	                	    rs = procmsger(pip,pdp,ep,el) ;
	            	        }

	            	        rs1 = mailmsg_finish(msgp) ;
	            	        if (rs >= 0) rs = rs1 ;
	            	        pdp->msgp = NULL ;
			    } /* end if (mailmsg) */
		        } /* end if (S_ISREG) */
		    } /* end if (bstat) */
	            rs1 = bclose(afp) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (b-file) */
	    } /* end if (name of file) */
	} /* end if (path-add) */
	tdname[tdlen] = '\0' ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progartmaint/procmsg: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsg) */


static int procmsger(PROGINFO *pip,PROCDATA *pdp,cchar *ep,int el)
{
	ARTINFO		ai, *aip = &ai ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progartmaint/procmsger: ent e=%t\n",ep,el) ;
#endif

	if ((rs = artinfo_start(aip,pdp->tdname,ep,el)) >= 0) {
	    int	f = TRUE ;

	    if ((rs >= 0) && pip->f.artexpires && f) {
	        rs = procmsgexpires(pip,pdp,aip) ;
	        f = rs ;
	    }

	    if ((rs >= 0) && pip->f.artmaint && f) {
	        rs = procmsgmaint(pip,pdp,aip) ;
	        f = rs ;
	    }

	    rs1 = artinfo_finish(aip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (artinfo) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progartmaint/procmsger: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsger) */


static int procmsgexpires(PROGINFO *pip,PROCDATA *pdp,ARTINFO *aip)
{
	MAILMSG		*msgp = pdp->msgp ;
	const time_t	dt = pip->daytime ;
	int		rs ;
	int		c = 1 ;
	const char	*hdr = HN_EXPIRES ;
	const char	*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    DATER	*tdp = &pip->td ;
	    int		vl = rs ;
	    if ((rs = dater_setmsg(tdp,vp,vl)) >= 0) {
	        time_t	mtime ;
	        if ((rs = dater_gettime(tdp,&mtime)) >= 0) {
	            if (dt >= mtime) {
	                c = 0 ;
	                rs = procmsgdel(pip,pdp,aip) ;
	            }
	        } /* end if (dater-gettime) */
	    } else if (rs == SR_INVALID) {
	        rs = SR_OK ;
	    }
	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progartmaint/procmsgexpires: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmsgexpires) */


static int procmsgmaint(PROGINFO *pip,PROCDATA *pdp,ARTINFO *aip)
{
	EMA		*emap ;
	EMA_ENT		*ep ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progartmaint/procmsgmaint: ent\n") ;
#endif

	if ((rs = artinfo_ngs(aip,pdp->msgp,&emap)) > 0) {
	    int	i ;
	    for (i = 0 ; ema_get(emap,i,&ep) >= 0 ; i += 1) {
	        const char	*np ;
	        int		nl ;
	        if ((rs = emaentry_getbestaddr(ep,&np)) > 0) {
	            char	abuf[MAXPATHLEN+1] ;
	            nl = rs ;
	            if ((rs = progngdname(pip,abuf,np,nl)) > 0) {
	                NULSTR		a ;
	                const int	alen = rs ;
	                const int	al = aip->anl ;
	                const char	*ap = aip->anp ;
	                const char	*aname ;
	                if ((rs = nulstr_start(&a,ap,al,&aname)) >= 0) {
	                    if ((rs = pathadd(abuf,alen,aname)) >= 0) {

	                        if ((rs = uc_access(abuf,R_OK)) >= 0) {
	                            c += 1 ;
	                        } else if (isNotAccess(rs)) {
	                            uc_unlink(abuf) ;
	                            rs = SR_OK ;
	                        } else if (isNotPresent(rs)) {
	                            rs = SR_OK ;
	                        }

	                    } /* end if (path-add) */
	                    rs1 = nulstr_finish(&a) ;
			    if (rs >= 0) rs = rs1 ;
	                } /* end if (aname) */
	            } else if ((rs == 0) || (rs == SR_NOTFOUND)) {
	                rs = SR_OK ;
	            }
	        } /* end if (emainfo-getbestaddr) */
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (artinfo-ngs) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progartmaint/procmsgmaint: mid rs=%d c=%u\n",rs,c) ;
#endif

	if ((rs >= 0) && (c == 0)) {
	    NULSTR	a ;
	    const int	alen = pdp->tdlen ;
	    const int	al = aip->anl ;
	    const char	*ap = aip->anp ;
	    const char	*aname ;
	    char	*abuf = pdp->tdname ;
	    if ((rs = nulstr_start(&a,ap,al,&aname)) >= 0) {
	        if ((rs = pathadd(abuf,alen,aname)) >= 0) {

	            rs = uc_unlink(abuf) ;

	        } /* end if (path-add) */
	        rs1 = nulstr_finish(&a) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (nulstr) */
	} /* end if (article-stage) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progartmaint/procmsgmaint: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmsgmaint) */


static int procmsgdel(PROGINFO *pip,PROCDATA *pdp,ARTINFO *aip)
{
	EMA		*emap ;
	EMA_ENT		*ep ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progartmaint/procmsgdel: ent\n") ;
#endif

	if ((rs = artinfo_ngs(aip,pdp->msgp,&emap)) > 0) {
	    int		i ;
	    for (i = 0 ; ema_get(emap,i,&ep) >= 0 ; i += 1) {
	        int	nl ;
	        cchar	*np ;
	        if ((rs = emaentry_getbestaddr(ep,&np)) > 0) {
	            char	abuf[MAXPATHLEN+1] ;
	            nl = rs ;
	            if ((rs = progngdname(pip,abuf,np,nl)) >= 0) {
	                NULSTR		a ;
	                const int	alen = rs ;
	                const int	al = aip->anl ;
	                const char	*ap = aip->anp ;
	                const char	*aname ;
	                if ((rs = nulstr_start(&a,ap,al,&aname)) >= 0) {
	                    if ((rs = pathadd(abuf,alen,aname)) >= 0) {

	                        rs = uc_unlink(abuf) ;
	                        if (rs == SR_NOENT) rs = SR_OK ;

	                    } /* end if (path-add) */
	                    rs1 = nulstr_finish(&a) ;
			    if (rs >= 0) rs = rs1 ;
	                } /* end if (aname) */
	            } else if ((rs == 0) || (rs == SR_NOTFOUND)) {
	                rs = SR_OK ;
	            } /* end if (progngdname) */
	        } /* end if (emainfo-getbestaddr) */
	        if (rs < 0) break ;
	    } /* end end */
	} /* end if (artinfo-ngs) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progartmaint/procmsgdel: mid rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    NULSTR	a ;
	    const int	alen = pdp->tdlen ;
	    const int	al = aip->anl ;
	    const char	*ap = aip->anp ;
	    const char	*aname ;
	    char	*abuf = pdp->tdname ;
	    if ((rs = nulstr_start(&a,ap,al,&aname)) >= 0) {
	        if ((rs = pathadd(abuf,alen,aname)) >= 0) {

	            rs = uc_unlink(abuf) ;

	        } /* end if (path-add) */
	        nulstr_finish(&a) ;
	    } /* end if (nulstr) */
	} /* end if (article-stage) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progartmaint/procmsgdel: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgdel) */


static int procdata_start(PROCDATA *pdp,char *tdname,int tdlen)
{
	memset(pdp,0,sizeof(PROCDATA)) ;
	pdp->tdname = tdname ;
	pdp->tdlen = tdlen ;
	return SR_OK ;
}
/* end subroutine (procdata_start) */


static int procdata_finish(PROCDATA *pdp)
{
	if (pdp == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (procdata_finish) */


static int artinfo_start(ARTINFO *aip,char *tdname,cchar *anp,int anl)
{
	memset(aip,0,sizeof(ARTINFO)) ;
	aip->tdname = tdname ;
	aip->anp = anp ;
	aip->anl = anl ;
	return SR_OK ;
}
/* end subroutine (artinfo_start) */


static int artinfo_finish(ARTINFO *aip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (aip->open.ngema) {
	    aip->open.ngema = FALSE ;
	    rs1 = ema_finish(&aip->ngema) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (artinfo_finish) */


static int artinfo_ngs(ARTINFO *aip,MAILMSG *msgp,EMA **rpp)
{
	int		rs = SR_OK ;

	if (! aip->f.ngema) {
	    int		vl ;
	    const char	*hdr = HN_NEWSGROUPS ;
	    const char	*vp ;
	    aip->f.ngema = TRUE ;
	    if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	        EMA	*ngp = &aip->ngema ;
	        vl = rs ;
	        if ((rs = ema_starter(ngp,vp,vl)) >= 0) {
	            aip->open.ngema = TRUE ;
	            aip->ngroups = rs ;
	        } /* end if (newsgroups EMA) */
	    } else if (rs == SR_NOTFOUND) {
	        rs = SR_OK ;
	    }
	} else {
	    if (aip->open.ngema) rs = aip->ngroups ;
	}

	if (rpp != NULL) *rpp = NULL ;
	if ((rs >= 0) && aip->open.ngema && (rpp != NULL)) {
	    *rpp = &aip->ngema ;
	}

	return rs ;
}
/* end if (artinfo_ngs) */


