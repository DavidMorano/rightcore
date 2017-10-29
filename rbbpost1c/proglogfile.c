/* progloger */

/* log handling (general-program) */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */
#define	CF_DEBUG	0		/* switchable print-outs */


/* revision history:

	= 2008-09-01, David A­D­ Morano

	This subroutine was borrowed and modified from previous generic
	front-end 'main' subroutines.


*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Here we do some log-file handling.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<format.h>
#include	<userinfo.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LOGCNAME
#define	LOGCNAME	"log"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,cchar *,cchar *) ;
extern int	isNotPresent(int) ;

extern int	proglogfname(PROGINFO *,char *,cchar *,cchar *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int progloger_begin(PROGINFO *pip,cchar *lfn,USERINFO *uip)
{
	int		rs = SR_OK ;
	int		f_opened = FALSE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progloger_begin: f_log=%u lfn=%s\n",
	        pip->f.logprog,lfn) ;
#endif

	if (pip->f.logprog) {
	    const char	*logcname = LOGCNAME ;
	    char	lbuf[MAXPATHLEN+1] ;
	    if ((rs = proglogfname(pip,lbuf,logcname,lfn)) >= 0) {
	        const char	**vpp = &pip->lfname ;
	        int		pl = rs ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progloger_begin: logfname=%s\n",lbuf) ;
#endif
	        if ((rs = proginfo_setentry(pip,vpp,lbuf,pl)) >= 0) {
	            lfn = pip->lfname ;
	            if ((lfn != NULL) && (lfn[0] != '-')) {
	                LOGFILE		*lhp = &pip->lh ;
	                const char	*logid = pip->logid ;
	                if ((rs = logfile_open(lhp,lfn,0,0666,logid)) >= 0) {
			    const time_t	dt = pip->daytime ;
	                    cchar		*pn = pip->progname ;
			    cchar		*sn = pip->searchname ;
			    cchar		*vn = pip->version ;
	                    f_opened = TRUE ;
	                    pip->open.logprog = TRUE ;

	                    if (pip->debuglevel > 0)
	                        bprintf(pip->efp,"%s: lf=%s\n",pn,lfn) ;

	                    if (pip->logsize > 0)
	                        logfile_checksize(lhp,pip->logsize) ;

	                    logfile_userinfo(lhp,uip,dt,sn,vn) ;

	                } else if (isNotPresent(rs)) {
	                    rs = SR_OK ;
			}
	            } /* end if */
	        } /* end if */
	    } /* end if (proglogfname) */
	} /* end if (logprog) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progloger_begin: ret rs=%d f_opened=%u\n",
	        rs,f_opened) ;
#endif

	return (rs >= 0) ? f_opened : rs ;
}
/* end subroutine (progloger_begin) */


int progloger_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->open.logprog) {
	    pip->open.logprog = FALSE ;
	    rs1 = logfile_close(&pip->lh) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (progloger_end) */


int progloger_print(PROGINFO *pip,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (pip->open.logprog) {
	    rs = logfile_print(&pip->lh,sp,sl) ;
	}
	return rs ;
}
/* end subroutine (progloger_print) */


/* vprintf-like thing */
int progloger_vprintf(PROGINFO *pip,cchar *fmt,va_list ap)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (pip->open.logprog) {
	    const int	flen = LINEBUFLEN ;
	    char	fbuf[LINEBUFLEN+1] ;
	    if ((rs = format(fbuf,flen,0x01,fmt,ap)) >= 0) {
	        rs = progloger_print(pip,fbuf,rs) ;
	        wlen = rs ;
	    }
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progloger_vprintf) */


/* PRINTFLIKE2 */
int progloger_printf(PROGINFO *pip,cchar fmt[],...)
{
	int		rs = SR_OK ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	debugprintf("progloger_printf: fmt=>%s<\n",fmt) ;
	debugprintf("progloger_printf: open=%u\n",pip->open.logprog) ;
	}
#endif
	if (pip->open.logprog) {
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = progloger_vprintf(pip,fmt,ap) ;
	    va_end(ap) ;
	}
	return rs ;
}
/* end subroutine (progloger_printf) */


int progloger_flush(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (pip->open.logprog) {
	    rs = logfile_flush(&pip->lh) ;
	}
	return rs ;
}
/* end subroutine (progloger_flush) */


