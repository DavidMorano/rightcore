/* progstampcheck */

/* generic (pretty much) front end program subroutine */
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

	Check our own (program) time-stamp.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	GETFNAME_TYPELOCAL
#define	GETFNAME_TYPELOCAL	0	/* search locally first */
#define	GETFNAME_TYPEROOT	1	/* search programroot area first */
#endif

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	getserial(const char *) ;
extern int	getfname(const char *,const char *,int,char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	procupdate(struct proginfo *) ;


/* local variables */


/* exported subroutines */


int progstampcheck(pip)
struct proginfo	*pip ;
{
	int	rs = SR_OK ;
	int	f_process = FALSE ;

	if (pip->stampfname == NULL)
	    return SR_FAULT ;

	if (pip->stampfname[0] != '\0') {
	    const char	*sf = pip->stampfname ;
	    struct ustat	sb ;
	    pip->daytime = time(NULL) ;
	    f_process = TRUE ;
	    if (u_stat(sf,&sb) >= 0) {
		f_process = ((pip->daytime - sb.st_mtime) >= pip->intmin) ;
	    } /* end if (stat) */
	    if (f_process)
		rs = procupdate(pip) ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progstampcheck: ret rs=%d f_proc=%u\n",
		rs,f_processf) ;
#endif

	return (rs >= 0) ? f_process : rs ;
}
/* end subroutine (progstampcheck) */


/* local subroutines */


static int procupdate(pip)
struct proginfo	*pip ;
{
	bfile	tsfile, *tfp = &tsfile ;
	int	rs ;
	const char	*sf = pip->stampfname ;

	if ((rs = bopen(tfp,sf,"wct",0666)) >= 0) {
	    const int	elen = MAXNAMELEN ;
	    const char	*nn = pip->nodename ;
	    const char	*un = pip->username ;
	    char	timebuf[TIMEBUFLEN+1] ;
	    char	ebuf[MAXNAMELEN+1] ;

	    if ((rs = sncpy3(ebuf,elen,nn,"!",un)) >= 0) {
		const char	*name = pip->name ;
		const char	*fmt = "%s %s\n" ;
	        timestr_logz(pip->daytime,timebuf) ;
		if ((name != NULL) && (name[0] != '\0'))
	            fmt = "%s %s (%s)\n" ;
	        rs = bprintf(tfp,fmt,timebuf,ebuf,name) ;
	    }

	    bclose(tfp) ;
	} /* end if (opened) */

	return rs ;
}
/* end subroutine (procupdate) */


