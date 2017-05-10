/* proglogzone */

/* log handling for Time-Zones stastics */
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

	Here we do some logging of time-zone information.


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
#include	<estrings.h>
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

extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	getserial(const char *) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	mkuiname(char *,int,USERINFO *) ;
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


int proglogzone_begin(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip->f.optlogzone) {
	    const int	nlen = MAXNAMELEN ;
	    cchar	*sn = pip->searchname ;
	    cchar	*zone = LOGZONEFNAME ;
	    char	nbuf[MAXNAMELEN+1] ;
	    if ((rs = snsds(nbuf,nlen,sn,zone)) >= 0) {
		cchar	*logcname = LOGCNAME ;
	        char	zfname[MAXPATHLEN+1] ;
	        if ((rs = mkpath3(zfname,pip->pr,logcname,nbuf)) >= 0) {
		    LOGZONES		*lzp = &pip->lz ;
		    const mode_t	om = 0666 ;
		    const int		of = O_RDWR ;
		    const int		zl = rs ;
	            if ((rs = logzones_open(lzp,zfname,of,om)) >= 0) {
			cchar	**vpp = &pip->zfname ;
			pip->open.logzone = TRUE ;
	        	rs = proginfo_setentry(pip,vpp,zfname,zl) ;
	        	if (pip->debuglevel > 0) {
			    cchar	*pn = pip->progname ;
	            	    bprintf(pip->efp,"%s: zf=%s\n",pn,zfname) ;
			}
			if (rs < 0)
			    logzones_close(lzp) ;
		    } else if (isNotPresent(rs))
			rs = SR_OK ;
		}
	    }
	} /* end if (log-zones) */
	return rs ;
}
/* end subroutine (proglogzone_begin) */


int proglogzone_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->open.logzone) {
	    pip->open.logzone = FALSE ;
	    rs1 = logzones_close(&pip->lz) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (proglogzone_end) */


int proglogzone_update(PROGINFO *pip,cchar *zn,int zl,int zoff,cchar *zs)
{
	int		rs = SR_OK ;
	if (pip->open.logzone) {
	    logzones_update(&pip->lz,zn,zl,zoff,zs) ;
	}
	return rs ;
}
/* end subroutine (proglogzone_update) */


