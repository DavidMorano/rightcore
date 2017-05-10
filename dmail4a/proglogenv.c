/* proglogenv */

/* log handling for Mail-Message-Envelopes */
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
#include	<ctype.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<format.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LOGCNAME
#define	LOGCNAME	"log"
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int proglogenv_begin(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip->f.optlogenv) {
	    const int	nlen = MAXNAMELEN ;
	    const char	*efn = LOGENVFNAME ;
	    char	nbuf[MAXNAMELEN+1] ;
	    if ((rs = snsds(nbuf,nlen,pip->searchname,efn)) >= 0) {
		cchar	*logcname = LOGCNAME ;
	        char	envfname[MAXPATHLEN+1] ;
	        if ((rs = mkpath3(envfname,pip->pr,logcname,nbuf)) >= 0) {
		    LOGFILE	*lfp = &pip->envsum ;
		    cchar	*logid = pip->logid ;
	            if ((rs = logfile_open(lfp,envfname,0,0666,logid)) >= 0) {
	        	pip->open.logenv = TRUE ;
	        	if (pip->debuglevel > 0) {
			    cchar	*pn = pip->progname ;
	            	    bprintf(pip->efp,"%s: envf=%s\n",pn,envfname) ;
			}
			rs = 1 ;
		    }
	        }
	    }
	} /* end if (option to log envelope information) */
	return rs ;
}
/* end subroutine (proglogenv_begin) */


int proglogenv_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->open.logenv) {
	    pip->open.logenv = FALSE ;
	    rs1 = logfile_close(&pip->envsum) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (proglogenv_end) */


int proglogenv_print(PROGINFO *pip,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (pip->open.logenv) {
	    LOGFILE	*lfp = &pip->envsum ;
	    rs = logfile_print(lfp,sp,sl) ;
	}
	return rs ;
}
/* end subroutine (proglogenv_print) */


/* vprintf-like thing */
int proglogenv_vprintf(PROGINFO *pip,cchar *fmt,va_list ap)
{
	const int	flen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		len = 0 ;
	char		fbuf[LINEBUFLEN+1] ;

	if (pip == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (pip->open.logenv) {
	    if ((rs = format(fbuf,flen,0x01,fmt,ap)) >= 0) {
	        rs = proglogenv_print(pip,fbuf,rs) ;
	        len = rs ;
	    }
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (proglogenv_vprintf) */


/* PRINTFLIKE2 */
int proglogenv_printf(PROGINFO *pip,const char fmt[],...)
{
	int		rs = SR_OK ;
	if (pip->open.logenv) {
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = proglogenv_vprintf(pip,fmt,ap) ;
	    va_end(ap) ;
	}
	return rs ;
}
/* end subroutine (proglogenv_printf) */


int proglogenv_flush(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (pip->open.logenv) {
	    LOGFILE	*lfp = &pip->envsum ;
	    rs = logfile_flush(lfp) ;
	}
	return rs ;
}
/* end subroutine (proglogenv_flush) */


