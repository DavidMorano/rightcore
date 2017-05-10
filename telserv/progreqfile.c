/* progreqfile */

/* create the REQ file */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 2008-06-23, David A­D­ Morano
        Hopefully, this subroutine will make life easier for everyone in the
        future! :-)

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine creates (devines) the "req" filename. This is the "req"
        file for shared (server) operations. When run in non-server mode, a
        private "req" file is created without using this subroutine.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"config.h"


/* local defines */

#ifndef	IPCDMODE
#define	IPCDMODE	0777
#endif

#define	O_SRVFLAGS	(O_RDWR | O_CREAT)

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snddd(char *,int,uint,uint) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,const char *,
			const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	dupup(int,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
#endif

extern int	progcheckdir(struct proginfo *,const char *,int,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int progreqfile(pip)
struct proginfo	*pip ;
{
	mode_t	operms ;

	int	rs = SR_OK ;
	int	cl ;
	int	fl = 0 ;

	const char	*cp ;

	char	fname[MAXPATHLEN + 1] ;
	char	cname[MAXNAMELEN + 1] ;


	fname[0] = '\0' ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("progreqfile: ent reqfname=%s\n",pip->reqfname) ;
#endif

	if ((pip->reqfname == NULL) || (pip->reqfname[0] == '+')) {
	    snsds(cname,MAXNAMELEN,pip->nodename,REQFEXT) ;
	    fl = mkpath4(fname,pip->pr,VARDNAME,pip->searchname,cname) ;
	} else {
	    if (pip->reqfname[0] != '/') {
		if (strchr(pip->reqfname,'/') != NULL)
	            fl = mkpath2(fname,pip->pr,pip->reqfname) ;
		else
	            fl = mkpath4(fname,pip->pr,VARDNAME,pip->searchname,
			pip->reqfname) ;
	    } else
	        fl = mkpath1(fname,pip->reqfname) ;
	}

	if ((fl > 0) && ((cl = sfdirname(fname,fl,&cp)) > 0)) {
	    operms = (IPCDMODE | 0555) ;
	    rs = progcheckdir(pip,cp,cl,operms) ;
	} /* end if */

	if ((rs >= 0) && (fl > 0)) {
	    rs = proginfo_setentry(pip,&pip->reqfname,fname,fl) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	debugprintf("progreqfile: ret rs=%d fl=%u\n",rs,fl) ;
	debugprintf("progreqfile: fname=%t\n",fname,fl) ;
	}
#endif

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (progreqfile) */


