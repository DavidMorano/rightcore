/* proglog */

/* handle program logging */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */
#define	CF_DEBUG	0		/* switchable print-outs */


/* revision history:

	= 2008-09-01, David A­D­ Morano
        This subroutine was borrowed and modified from previous generic
        front-end 'main' subroutines!

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Manage the general process logging.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<format.h>
#include	<nulstr.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matpstr(cchar **,int,cchar *,int) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	getserial(const char *) ;
extern int	mkuiname(char *,int,USERINFO *) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,cchar *,cchar *) ;
extern int	logfile_printfold(LOGFILE *,cchar *,cchar *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

int		proglog_begin(PROGINFO *,USERINFO *) ;
int		proglog_end(PROGINFO *) ;
int		proglog_intro(PROGINFO *,USERINFO *) ;
int		proglog_checksize(PROGINFO *) ;

static int	proglog_file(PROGINFO *) ;


/* local variables */


/* exported subroutines */


int proglog_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;
	int		f_opened = FALSE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("proglogopen: f_logprog=%u logfname=%s\n",
	        pip->f.logprog,pip->lfname) ;
#endif

	if (pip->f.logprog) {
	    if ((rs = proglog_file(pip)) >= 0) {
	        if (pip->lfname != NULL) {
		    LOGFILE	*lhp = &pip->lh ;
		    const char	*pn = pip->progname ;
	            const char	*lf = pip->lfname ;
	            const char	*li = pip->logid ;
	            if ((rs = logfile_open(lhp,lf,0,0666,li)) >= 0) {
	                f_opened = TRUE ;
	                pip->open.logprog = TRUE ;
	                if (pip->debuglevel > 0) {
	                    bprintf(pip->efp,"%s: lf=%s\n",pn,lf) ;
			}
			if ((rs = proglog_intro(pip,uip)) >= 0) {
	                    rs = proglog_checksize(pip) ;
			}
	            } else if (isNotPresent(rs)) {
	                rs = SR_OK ;
		    }
	        } /* end if */
	    } /* end if (log-file) */
	} /* end if (enabled) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("proglogopen: ret rs=%d f_opened=%u\n",rs,f_opened) ;
#endif

	return (rs >= 0) ? f_opened : rs ;
}
/* end subroutine (proglog_begin) */


int proglog_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->open.logprog) {
	    LOGFILE	*lhp = &pip->lh ;
	    pip->open.logprog = FALSE ;
	    rs1 = logfile_close(lhp) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (proglog_end) */


int proglog_intro(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("proglogintro: f_log=%u f_open=%u\n",
	        pip->f.logprog,pip->open.logprog) ;
#endif

	if (pip->open.logprog && pip->f.logprog) {
	    LOGFILE	*lhp = &pip->lh ;

	    if (uip != NULL) {
		const time_t	dt = pip->daytime ;
		cchar		*pn = pip->progname ;
		cchar		*ver = pip->version ;
	        rs = logfile_userinfo(lhp,uip,dt,pn,ver) ;
	    }

	    if (rs >= 0) {
	        rs = logfile_printf(lhp,"pr=%s\n",pip->pr) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("proglogintro: logfile_xx() rs=%d\n",rs) ;
#endif

	} /* end if (enabled and open) */

	return rs ;
}
/* end subroutine (proglog_intro) */


int proglog_checksize(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip->open.logprog && (pip->logsize > 0)) {
	    LOGFILE	*lhp = &pip->lh ;
	    rs = logfile_checksize(lhp,pip->logsize) ;
	}
	return rs ;
}
/* end subroutine (proglog_checksize) */


int proglog_check(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip->open.logprog) {
	    LOGFILE	*lhp = &pip->lh ;
	    rs = logfile_check(lhp,pip->daytime) ;
	}
	return rs ;
}
/* end subroutine (proglog_check) */


int proglog_getid(PROGINFO *pip,char *rbuf,int rlen)
{
	if (pip == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	return sncpy1(rbuf,rlen,pip->logid) ;
}
/* end subroutine (proglog_getid) */


int proglog_setid(PROGINFO *pip,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	if (pip == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (pip->open.logprog) {
	    NULSTR	ns ;
	    cchar	*logid ;
	    if ((rs = nulstr_start(&ns,sp,sl,&logid)) >= 0) {
		{
	            LOGFILE	*lhp = &pip->lh ;
	            rs = logfile_setid(lhp,logid) ;
		    c = rs ;
		}
		rs1 = nulstr_finish(&ns) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (nulstr) */
	} /* end if (log-open) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proglog_setid) */


int proglog_print(PROGINFO *pip,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (pip->open.logprog) {
	    LOGFILE	*lhp = &pip->lh ;
	    rs = logfile_print(lhp,sp,sl) ;
	}
	return rs ;
}
/* end subroutine (proglog_print) */


/* vprintf-like thing */
int proglog_vprintf(PROGINFO *pip,cchar *fmt,va_list ap)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (pip->open.logprog) {
	    const int	flen = LINEBUFLEN ;
	    char	fbuf[LINEBUFLEN+1] ;
	    if ((rs = format(fbuf,flen,0x01,fmt,ap)) >= 0) {
	        rs = proglog_print(pip,fbuf,rs) ;
	        wlen = rs ;
	    }
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (proglog_vprintf) */


/* PRINTFLIKE2 */
int proglog_printf(PROGINFO *pip,cchar *fmt,...)
{
	int		rs = SR_OK ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("proglog_printf: fmt=>%s<\n",fmt) ;
	    debugprintf("proglog_printf: open=%u\n",pip->open.logprog) ;
	}
#endif
	if (pip->open.logprog) {
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = proglog_vprintf(pip,fmt,ap) ;
	    va_end(ap) ;
	}
	return rs ;
}
/* end subroutine (proglog_printf) */


int proglog_printfold(PROGINFO *pip,cchar *pre,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (pre == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (pip->open.logprog) {
	    LOGFILE	*lhp = &pip->lh ;
	    rs = logfile_printfold(lhp,pre,sp,sl) ;
	}
	return rs ;
}
/* end subroutine (proglog_printfold) */


int proglog_ssprint(PROGINFO *pip,cchar *id,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (pip->open.logprog) {
	    LOGFILE	*lhp = &pip->lh ;
	    if (id != NULL) {
	        if ((rs = logfile_setid(lhp,id)) >= 0) {
	            if ((rs = logfile_print(lhp,sp,sl)) >= 0) {
	    	        rs = logfile_setid(lhp,pip->logid) ;
		    }
	        }
	    } else {
	        rs = logfile_print(lhp,sp,sl) ;
	    }
	}
	return rs ;
}
/* end subroutine (proglog_ssprint) */


/* vprintf-like thing */
int proglog_ssvprintf(PROGINFO *pip,cchar *id,cchar *fmt,va_list ap)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (pip->open.logprog) {
	    const int	flen = LINEBUFLEN ;
	    char	fbuf[LINEBUFLEN+1] ;
	    if ((rs = format(fbuf,flen,0x01,fmt,ap)) >= 0) {
	        rs = proglog_ssprint(pip,id,fbuf,rs) ;
	        wlen = rs ;
	    }
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (proglog_ssvprintf) */


/* PRINTFLIKE2 */
int proglog_ssprintf(PROGINFO *pip,cchar *id,cchar *fmt,...)
{
	int		rs = SR_OK ;
	if (pip->open.logprog) {
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = proglog_ssvprintf(pip,id,fmt,ap) ;
	    va_end(ap) ;
	}
	return rs ;
}
/* end subroutine (proglog_ssprintf) */


int proglog_flush(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (pip->open.logprog) {
	    LOGFILE	*lhp = &pip->lh ;
	    rs = logfile_flush(lhp) ;
	}
	return rs ;
}
/* end subroutine (proglog_flush) */


/* local subroutines */


static int proglog_file(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->f.logprog) {
	    int		cl = -1 ;
	    cchar	*cp = pip->lfname ;
	    if ((cp == NULL) || (cp[0] == '+')) {
	        cp = pip->searchname ;
	        cl = -1 ;
	    }
	    if (cp[0] != '-') {
	        char	tbuf[MAXPATHLEN + 1] ;

	        pip->have.logprog = TRUE ;
	        if (cp[0] != '/') {
	            if (strchr(cp,'/') != NULL) {
	                cl = mkpath2(tbuf,pip->pr,cp) ;
	            } else {
			cchar	*logcname = LOGCNAME ;
	                cl = mkpath3(tbuf,pip->pr,logcname,cp) ;
	            }
	            cp = tbuf ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("proglogfile: found logfname=%t\n",cp,cl) ;
#endif

	        if (cp != NULL) {
	            cchar	**vpp = &pip->lfname ;
	            rs = proginfo_setentry(pip,vpp,cp,cl) ;
	        }

	    } else {
	        pip->f.logprog = FALSE ;
	    }
	} /* end if (opened) */

	return rs ;
}
/* end subroutine (proglog_file) */


