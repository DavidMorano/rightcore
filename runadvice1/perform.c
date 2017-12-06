/* perform */

/* perform an actual ADVICE program execution */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1993-10-01, David A.D. Morano
	This program was originally written.

	= 1996-02-01, David A.D. Morano
	This program was pretty extensively modified to take
	much more flexible combinations of user supplied paramters.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We execute the ADVICE program as appropriate.


*******************************************************************************/

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<rpc/rpc.h>
#include	<unistd.h>
#include	<signal.h>
#include	<fcntl.h>
#include	<time.h>
#include	<string.h>
#include	<stdlib.h>

#include	<bfile.h>
#include	<baops.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<varsub.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"paramopt.h"
#include	"configfile.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	matstr() ;
extern int	configread(), configfree() ;
extern int	paramloads(), paramloadu(), paramload() ;
extern int	cfdouble() ;
extern int	machineinit(), machineadd() ;
extern int	runinit(), runadd(), rundel() ;

extern char	*strbasename() ;


/* forward references */


/* external variables */

extern struct global	g ;


/* local data structures */


/* exported subroutines */


int perform(vshp,nslave,pid)
VARSUB		*vshp ;
int		nslave ;
pid_t		pid ;
{
	bfile		*fpa[3] ;
	bfile		progout ;
	bfile		doutfile, *dofp = &doutfile ;
	bfile		tmpfile, *tfp = &tmpfile ;
	bfile		lockfile, *lfp = &lockfile ;

	int	rs ;
	int	len, i ;
	int	childstat ;
	int	f_bol, f_eol ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	tmprunfname[MAXPATHLEN + 1] ;
	char	tmpconfname[MAXPATHLEN + 1] ;
	char	tmpparamfname[MAXPATHLEN + 1] ;
	char	tmpmainfname[MAXPATHLEN + 1] ;
	char	tmpoutfname[MAXPATHLEN + 1] ;
	char		cmd[(MAXPATHLEN * 2) + 1] ;
	char		buf[(MAXPATHLEN * 2) + 1] ;
	char		*cp ;
	char		slavestring[10] ;
	char		*fname ;


#if	CF_DEBUG
	debugprintf("perform: entered dl=%d slave=%d pid=%d\n",
		g.debuglevel,nslave,pid) ;
#endif

	for (i = 0 ; i < 3 ; i += 1) fpa[i] = NULL ;

	if (g.f.machines) fpa[1] = &progout ;

	sprintf(slavestring,"%d",nslave) ;

/* make a generic temporary file template */

	mkpath2(tmpfname, g.tmpdir, "raXXXXXXXX.adv") ;

/* create a temporary "run" file */

#if	CF_DEBUG
	debugprintf("perform: S=%d about to create a TMP file\n",nslave) ;
#endif

	cp = "run2" ;
	if ((rs = mktmpfile(tmprunfname,0664,tmpfname)) < 0)
	    goto badtmpfile ;

#if	CF_DEBUG
	debugprintf("perform: S=%d we made a TMP file\n",nslave) ;
#endif

/* create a temporary "control" file */

	cp = "control" ;
	if ((rs = mktmpfile(tmpconfname,0644,tmpfname)) < 0)
	    goto badtmpfile ;

/* create a temporary "params" file */

	tmpparamfname[0] = '\0' ;
	if (access(g.paramfname,R_OK) == 0) {

	    cp = "params" ;
	    if ((rs = mktmpfile(tmpparamfname,0664,tmpfname)) < 0)
	        goto badtmpfile ;

	}

/* create a temporary "main" file */

	cp = "main" ;
	if ((rs = mktmpfile(tmpmainfname,0664,tmpfname)) < 0)
	    goto badtmpfile ;

/* create a temporary "output" file */

	cp = "output" ;
	sprintf(tmpfname,"%s/raXXXXXXXX.out",g.tmpdir) ;

	if ((rs = mktmpfile(tmpoutfname,0664,tmpfname)) < 0)
	    goto badtmpfile ;


/* add the substitutions for the various file names */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("perform: adding parameters for file names\n") ;
#endif

	varsub_add(vshp,"CONTROLCKT",-1,tmpconfname,-1) ;

	if (tmpparamfname[0] == '\0') {
	    varsub_add(vshp,"PARAMS",-1,"/dev/null",-1) ;
	} else {
	    varsub_add(vshp,"PARAMS",-1,tmpparamfname,-1) ;
	}

	rs = varsub_add(vshp,"MAINCKT",-1,tmpmainfname,-1) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("perform: added to VSA %d\n",rs) ;
#endif

	rs = varsub_add(vshp,"OUTFILE",-1,tmpoutfname,-1) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("perform: added to VSA %d\n",rs) ;
#endif

/* do the substitutions on all possible run related files */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("perform: applying substitutions to files\n") ;
#endif

/* run file */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("perform: about to 'runfile' %s\n",
	        g.runfname) ;
#endif

	fname = g.runfname ;
	if ((rs = subfile(vshp,g.runfname,tmprunfname)) < 0)
	    goto badsubfile ;

/* control file */

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "perform: about to 'confile' %s\n",
	    g.confname) ;
#endif

	fname = g.confname ;
	if ((rs = subfile(vshp,g.confname,tmpconfname)) < 0)
	    goto badsubfile ;

/* params file */

	if (tmpparamfname[0] != '\0') {

	    fname = g.runfname ;
	    if ((rs = subfile(vshp,g.paramfname,tmpparamfname)) < 0)
	        goto badsubfile ;

	}

/* main circuit file */

	fname = g.mainfname ;
	if ((rs = subfile(vshp,g.mainfname,tmpmainfname)) < 0)
	    goto badsubfile ;


#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "perform: freeing substitution array\n") ;
#endif

/* run ADVICE on this combination of parameters */

	sprintf(cmd,"%s < %s",g.prog_advice,tmprunfname) ;

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "perform: about to execute ADVICE program\n") ;
#endif

	if ((rs = bopencmd(fpa,cmd)) < 0) goto badspawn ;

	if (g.f.machines) {
	    const int	LINEBUFLEN ;
	    char	lbuf[LINEBUFLEN+1] ;

/* funnel the output from ADVICE back to the main parent */

	    f_bol = TRUE ;
	    while ((len = breadline(fpa[1],lbuf,llen)) > 0) {

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "perform: breadline=%W\n",lbuf,len) ;
#endif

	        f_eol = FALSE ;
	        if (lbuf[len - 1] == '\n') f_eol = TRUE ;

	        if (f_bol) bprintf(g.ofp,"%s: ",slavestring) ;

	        if ((rs = bwrite(g.ofp, lbuf,len)) < 0)
	            break ;

	        if (f_eol) bflush(g.ofp) ;

	        if (g.f_signal) goto signaled ;

	        f_bol = f_eol ;
	    } /* end while */

	    if (rs < 0)
	        bprintf(g.efp,
	            "%s: could not write output from program (rs %d)\n",
	            g.progname,rs) ;

	} /* end if (machines) */

/* wait for it (ADVICE) to come back */

	waitpid(rs,&childstat,WUNTRACED) ;

	if (g.f_signal) goto signaled ;


/* merge ADVICE program data output back into the actual data output file */

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "perform: merging output data\n") ;
#endif

	rs = OK ;
	if ((rs = bopen(tfp,tmpoutfname,"r",0666)) >= 0) {

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "perform: opened temporary output file\n") ;
#endif

/* open the data output file */

	    if ((rs = bopen(dofp,g.outfname,"wca",0664)) >= 0) {

/* lock it with the outer lock */

	        strcpy(buf,"LK") ;

	        strcpy(buf + 2,g.outfname) ;

#if	CF_DEBUG
	        if (g.debuglevel > 1) debugprintf(
	            "perform: about to get outer lock\n") ;
#endif

	        if ((rs = bopenlock(lfp,buf,120,3600)) >= 0) {

	            rs = bprintf(lfp,"%s: pid=%d\n",
	                g.progname,pid) ;

	            rs = bclose(lfp) ;

#if	CF_DEBUG
	            if (g.debuglevel > 3) debugprintf(
	                "perform: error from close (rs %d)\n",rs) ;
#endif

	        } else {

	            bprintf(g.efp,
	                "%s: could not get output file lock (outer) (rs %d)\n",
	                g.progname,rs) ;

	        }

#if	CF_DEBUG && 0
	        if (g.debuglevel > 1) {

	            debugprintf(
	                "perform: calling SHELL\n") ;

	            system("/usr/bin/ksh -i") ;

	            debugprintf(
	                "perform: came back from SHELL\n") ;

	        }
#endif

/* lock with the inner (UNIX record) lock */

#if	CF_DEBUG
	        if (g.debuglevel > 1) debugprintf(
	            "perform: about to get inner lock\n") ;
#endif

	        if ((rs = bcontrol(dofp,BC_LOCK,120)) < 0) {
	            bprintf(g.efp,
	                "%s: could not get output file lock (inner) (rs %d)\n",
	                g.progname,rs) ;
	        }

/* copy it all over */

#if	CF_DEBUG
	        if (g.debuglevel > 1) debugprintf(
	            "perform: copying over\n") ;
#endif

	        while ((rs = bcopyblock(tfp,dofp,1024)) > 0)
	            if (g.f_signal) break ;

#if	CF_DEBUG
	        if (g.debuglevel > 1) debugprintf(
	            "perform: about to unlock inner, rs=%d\n",rs) ;
#endif

/* unlock inner */

	        bcontrol(dofp,BC_UNLOCK,1) ;

#if	CF_DEBUG
	        if (g.debuglevel > 1) debugprintf(
	            "perform: unlocked inner\n") ;
#endif

/* unlock outer */

	        unlink(buf) ;

#if	CF_DEBUG
	        if (g.debuglevel > 1) debugprintf(
	            "perform: out of lock stuff rs=%d\n",rs) ;
#endif

	        bclose(dofp) ;
	    } /* end if (opening data output file */

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "perform: out of opening data file rs=%d\n",rs) ;
#endif

	    bclose(tfp) ;
	} /* end if (updating output file) */

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "perform: about to cleanup\n") ;
#endif

/* clean up */
done:
	if (tmprunfname[0] != '\0') unlink(tmprunfname) ;

	if (tmpconfname[0] != '\0') unlink(tmpconfname) ;

	if (tmpparamfname[0] != '\0') unlink(tmpparamfname) ;

	if (tmpmainfname[0] != '\0') unlink(tmpmainfname) ;

	if (tmpoutfname[0] != '\0') unlink(tmpoutfname) ;

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "perform: exiting rs=%d\n",rs) ;
#endif

	return rs ;

signaled:

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "perform: signaled\n",rs) ;
#endif

	rs = BAD ;
	goto done ;

badtmpfile:
#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "perform: bad TMP file (rs=%d)\n",rs) ;
#endif

	goto done ;

badsubfile:
#if	CF_DEBUG
	if (g.debuglevel > 1)
		debugprintf("perform: bad sub file (rs=%d)\n",rs) ;
#endif

	goto done ;

badspawn:
#if	CF_DEBUG
	if (g.debuglevel > 1)
		debugprintf("perform: bad spawn (rs=%d)\n",rs) ;
#endif

	goto done ;

}
/* end subroutine (perform) */


