/* slave */

/* execute the slave mode operation */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1993-10,01, David Morano

	This program was originally written.

	= 1996-02-01, David Morano

	This program was pretty extensively modified to take
	much more flexible combinations of user supplied paramters.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<rpc/rpc.h>
#include	<unistd.h>
#include	<signal.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<baops.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<varsub.h>
#include	<paramopt.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"configfile.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	BUFLEN		(MAXPATHLEN + (LINEBUFLEN * 2))


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	matstr() ;
extern int	configread(), configfree() ;
extern int	paramloads(), paramloadu(), paramload() ;
extern int	cfdouble() ;
extern int	machineinit(), machineadd() ;
extern int	runinit(), runadd(), rundel() ;


/* forward references */

static int	getfilename() ;


/* extern variables */

extern struct global	g ;

extern struct userinfo	u ;


/* local data structures */


/* exported subroutines */


int slave(nslave,ifp,pid)
int		nslave ;
bfile		*ifp ;
pid_t		pid ;
{
	VARSUB		vsh ;
	struct tm	ts, *timep ;
	time_t		daytime ;
	int		len, i, rs ;
	int		nparams = 0 ;
	int		f_bol, f_eol ;
	const char	*cp ;
	char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUG
	debugprintf("slave-%d: entered on host=%s dl=%d\n",
	    nslave,u.nodename,g.debuglevel) ;
#endif

/* perform a little synchronization ritual for old time sake */

	f_bol = TRUE ;
	while ((len = breadline(ifp,lbuf,LINEBUFLEN)) > 0) {

	    f_eol = FALSE ;
	    if (lbuf[len - 1] == '\n') {

	        f_eol = TRUE ;
	        len -= 1 ;
	    }

	    lbuf[len] = '\0' ;

	    if (f_bol && (strncmp(lbuf,"START",5) == 0)) break ;

	    f_bol = f_eol ;

	} /* end while */

#if	CF_DEBUG
	debugprintf("slave-%d: read len=%d\n",
	    nslave,len) ;
#endif

	if (len < 0) goto badread ;

/* get the ID that we will make log entries under */

	if (getfilename(ifp,&g.logid) < 0) goto badread ;

#if	CF_DEBUG
	debugprintf("slave-%d: logid=%s\n",
	    nslave,g.logid) ;
#endif

/* get the ADVICE program to execute */

	if (getfilename(ifp,&g.prog_advice) < 0) goto badread ;

/* read in the file names (five of them) */

	if (getfilename(ifp,&g.runfname) < 0) goto badread ;

	if (getfilename(ifp,&g.outfname) < 0) goto badread ;

	if (getfilename(ifp,&g.confname) < 0) goto badread ;

	if (getfilename(ifp,&g.paramfname) < 0) goto badread ;

	if (getfilename(ifp,&g.mainfname) < 0) goto badread ;


/* initialize the substitution array for 'subfile' */

	if ((rs = varsub_start(&vsh,0)) < 0)
		goto badalloc ;

/* get the number of parameters to read from the input */

	if ((rs = breadline(ifp,lbuf,LINEBUFLEN)) < 0) goto badread ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("slave-%d: line=%W\n",
	        nslave,lbuf,rs - 1) ;
#endif

	if ((rs = cfdeci(lbuf,rs - 1,&nparams)) < 0) 
		goto badnum ;

/* create the substitution array from the parent */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("slave: creating substitution array\n") ;
#endif

	i = 0 ;
	while ((i < nparams) && (len = breadline(ifp,lbuf,LINEBUFLEN)) > 0) {

	    if (lbuf[len - 1] == '\n') len -= 1 ;

	    lbuf[len] = '\0' ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("slave: lbuf=%W\n",
	            lbuf,len) ;
#endif

	    if ((cp = strchr(lbuf,'=')) != NULL) *cp++ = '\0' ;

	    rs = varsub_add(&vsh,lbuf,-1,cp,-1) ;

	    if (rs >= 0) {

	        if (strcmp(lbuf,"TIMESTEP") == 0)
	            rs = varsub_add(&vsh,"STEP",-1,cp,-1) ;

	        else if (strcmp(lbuf,"TIMELEN") == 0)
	            rs = varsub_add(&vsh,"LENGTH",-1,cp,-1) ;

	    }

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("slave: added to VSA %d\n",rs) ;
#endif

	    i += 1 ;

	} /* end for (creating substitution array) */

	bflush(ifp) ;

/* did we get the correct number of parameters ? */

	if (i != nparams) bprintf(g.efp,
	    "%s: we did not get the right number of params s=%d i=%d\n",
	    g.progname,nslave,i) ;

/* man alive, we got all the way to this point !! */

	(void) time(&daytime) ;

#ifdef	SYSV
	timep = (struct tm *) localtime_r((time_t *) &daytime,&ts) ;
#else
	timep = (struct tm *) localtime((time_t *) &daytime) ;
#endif

/* open the log file for the slave */

	if ((rs = logfile_open(&g.lh,g.logfname,0,0666,g.logid)) < 0)
	    bprintf(g.efp,"%s: could not open the log file (rs %d)\n",
	        g.progname,rs) ;

/* make the first entry (as this slave) */

	logfile_printf(&g.lh,
		"%02d%02d%02d %02d%02d:%02d S=%03d %-14s %s/%s\n",
	    timep->tm_year,
	    timep->tm_mon + 1,
	    timep->tm_mday,
	    timep->tm_hour,
	    timep->tm_min,
	    timep->tm_sec,
	    nslave,
	    g.progname,(g.f.sysv_ct) ? "SYSV" : "BSD",VERSION) ;

	logfile_printf(&g.lh,"S=%03d starting os=%s node=%s\n",
	    nslave,u.f.sysv ? "SYSV" : "BSD",u.nodename) ;


/* run it */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("slave: about to perform\n") ;
#endif

	rs = perform(&vsh,nslave,pid) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("slave: performed w/ rs=%d\n",rs) ;
#endif

/* free the variable substitution array */

	varsub_finish(&vsh) ;


/* clean up */
done:
	return rs ;

badnum:
#if	CF_DEBUG
	if (g.debuglevel > 0) debugprintf(
	    "slave-%d: bad number (rs %d)\n",nslave,rs) ;
#endif

	goto done ;

badread:

#if	CF_DEBUG
	if (g.debuglevel > 0) debugprintf(
	    "slave-%d: bad read (rs %d)\n",nslave,rs) ;
#endif

	goto done ;

badalloc:
#if	CF_DEBUG
	if (g.debuglevel > 0) debugprintf(
	    "slave-%d: bad alloc (rs %d)\n",nslave,rs) ;
#endif

	goto done ;

}
/* end subroutine (slave) */


static int getfilename(ifp,fnp)
bfile		*ifp ;
char		**fnp ;
{
	int	rs ;
	char	*cp ;


	if ((cp = (char *) malloc(MAXPATHLEN)) == NULL)
	    return BAD ;

	if ((rs = breadline(ifp,cp,MAXPATHLEN)) > 0) {

	    if (cp[rs - 1] == '\n') rs -= 1 ;

	    cp[rs] = '\0' ;
	}

	*fnp = cp ;
	return rs ;
}
/* end subroutine (getfilename) */



