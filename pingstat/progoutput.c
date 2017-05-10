/* progoutput */

/* program-output */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 2001-09-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine forms the front-end part of a generic PCS type
	of program.  This front-end is used in a variety of PCS programs.

	This subroutine was originally part of the Personal Communications
	Services (PCS) package but can also be used independently from it.
	Historically, this was developed as part of an effort to maintain
	high function (and reliable) email communications in the face
	of increasingly draconian security restrictions imposed on the
	computers in the DEFINITY development organization.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<vechand.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"pinghost.h"
#include	"pingstatdb.h"
#include	"pingtab.h"
#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfkey(const char *,int,const char **) ;
extern int	nextfield(char *,int,const char **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	getpwd(char *,int) ;
extern int	getfname(const char *,const char *,int,char *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	bopenroot(bfile *,char *,char *,char *,char *,int) ;

extern int	progpingtabadd(struct proginfo *,const char *,int) ;
extern int	proghost(struct proginfo *,const char *,int,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*timestr_log(time_t,char *) ;


/* externals variables */


/* local global variabes */


/* local structures */


/* forward references */

static int	procout_begin(struct proginfo *,bfile *,const char *) ;
static int	procout_end(struct proginfo *) ;

static int	procdefpingtab(struct proginfo *) ;
static int	loadpingtabs(struct proginfo *,VECHAND *) ;
static int	loadhost(struct proginfo *,VECHAND *,
			const char *,int,int,int) ;
static int	prochostsfins(struct proginfo *,VECHAND *) ;
static int	mungepingtab(struct proginfo *,const char *,char *) ;
static int	prochosts(struct proginfo *,VECHAND *) ;


/* local variables */


/* exported subroutines */


int progoutput(pip,aip,bop)
struct proginfo	*pip ;
struct arginfo	*aip ;
BITS		*bop ;
{
	bfile	ofile ;

	const int	defintmin = pip->defintminping ;
	const int	to = pip->toping ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	len ;
	int	sl, cl ;
	int	pan = 0 ;

	const char	*afname = pip->afname ;
	const char	*ofname = pip->ofname ;
	const char	*cp ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("progoutput: entered defintmin=%d\n",
		pip->defintminping) ;
	    debugprintf("progoutput: intmin=%d\n",pip->intminping) ;
	}
#endif

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = BFILE_STDOUT ;

	if ((rs = procout_begin(pip,&ofile,ofname)) >= 0) {
	    VECHAND	phosts ;

/* initialize the list of ping hosts */

	    if ((rs = vechand_start(&phosts,20,0)) >= 0) {
	    int	ai ;
	    int	f ;

/* process the positional arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progoutput: positional arguments\n") ;
#endif

	for (ai = 1 ; ai < aip->argc ; ai += 1) {

	    f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	    f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	    if (! f) continue ;

	    cp = aip->argv[ai] ;
	    pan += 1 ;
	    rs = loadhost(pip,&phosts,cp,-1,defintmin,to) ;

	    if (rs < 0) break ;
	} /* end for (looping through requested circuits) */

/* process any host names that are in the argument filename list file */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progoutput: argfile arguments rs=%d pan=%u\n",
		rs,pan) ;
#endif

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(afname,"-") == 0) afname = BFILE_STDIN ;

	    if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
		const int	llen = LINEBUFLEN ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            cl = nextfield(lbuf,len,&cp) ;

	            if ((cl <= 0) || (cp[0] == '#'))
	                continue ;

	            pan += 1 ;
	            rs = loadhost(pip,&phosts,cp,cl,defintmin,to) ;

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        bclose(afp) ;
	    } else {
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: inaccessible arg-list file (%d)\n",
	                pip->progname,rs) ;
	            bprintf(pip->efp,"%s: argfile=%s\n",afname) ;
	        } /* end if */
	    } /* end if */

	} /* end if (processing file argument file list) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progoutput: def rs=%d pan=%u\n",
		rs,pan) ;
#endif

/* if no hosts and no explicit PINGTABS, process the default PINGTAB */

	sl = vecstr_count(&pip->pingtabs) ;

	if ((rs >= 0) && (pan <= 0) && (sl <= 0) && pip->f.update) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progoutput: default arguments\n") ;
#endif

	    rs = procdefpingtab(pip) ;
	    pan += rs ;

	} /* end if (default PINGTAB file) */

	if (rs >= 0) {

/* now load all of the PINGTAB files */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progoutput: loadpingtabs()\n") ;
#endif

	    rs = loadpingtabs(pip,&phosts) ;
	    pan += rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progoutput: loadpingtabs() rs=%d\n",rs) ;
#endif

	} /* end for (processing PINGTABS) */

/* process all hosts that we were given */

	if (rs >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progoutput: process hosts\n") ;
#endif

	    rs1 = prochosts(pip,&phosts) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progoutput: prochosts() rs=%d\n",rs1) ;
#endif

#ifdef	COMMENT
	   if (pip->open.logprog && (rs1 >= 0))
		logfile_printf(&pip->lh,"hosts processed=%u\n", rs1) ;

	   if (pip->debuglevel > 0)
		bprintf(pip->efp,"%s: hosts processed=%u\n", 
		    pip->progname,pip->c_processed) ;
#endif /* COMMENT */

	} /* end if (processing) */

/* free up the ping-hosts */

	prochostsfins(pip,&phosts) ;

	vechand_finish(&phosts) ;
	} /* end if (phosts) */

	    procout_end(pip) ;
	} /* end if (procout) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progoutput: ret rs=%d pan=%u\n",rs,pan) ;
#endif

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (progoutput) */


/* local subroutines */


static int procout_begin(pip,ofp,ofname)
struct proginfo	*pip ;
bfile		*ofp ;
const char	*ofname ;
{
	int	rs = SR_OK ;
	pip->ofp = NULL ;
	if (! pip->f.nooutput) {
	    if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0) {
	        pip->ofp = ofp ;
	    }
	}
	return rs ;
}
/* end subroutine (procout_begin) */


static int procout_end(pip)
struct proginfo	*pip ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	if (pip->ofp != NULL) {
	    rs1 = bclose(pip->ofp) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->ofp = NULL ;
	}
	return rs ;
}
/* end subroutine (procout_end) */


static int procdefpingtab(pip)
struct proginfo	*pip ;
{
	int	rs ;

	const char	*dn = DEFPTFNAME ;


	rs = progpingtabadd(pip,dn,-1) ;

	return rs ;
}
/* end subroutine (procdefpingtab) */


/* process all of the ping-hosts */
static int prochosts(pip,phlp)
struct proginfo	*pip ;
VECHAND		*phlp ;
{
	PINGHOST	*php ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;
	int	f = TRUE ;


	for (i = 0 ; vechand_get(phlp,i,&php) >= 0 ; i += 1) {
	    if (php == NULL) continue ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	debugprintf("progoutput/prochosts: host=%s\n",php->name) ;
	debugprintf("progoutput/prochosts: ->intminping=%d\n",php->intminping) ;
	debugprintf("progoutput/prochosts: to=%d\n",php->to) ;
	}
#endif

	    pip->c_hosts += 1 ;
	    rs1 = proghost(pip,php->name,php->intminping,php->to) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
		debugprintf("progoutput/prochosts: proghost() rs=%d\n",rs1) ;
#endif

	    if ((rs1 >= 0) || (rs1 == SR_HOSTDOWN))
	        pip->c_processed += 1 ;

	    if (rs1 >= 0)
	        pip->c_up += 1 ;

	    if (rs1 == SR_HOSTDOWN)
	        f = FALSE ;

	    if ((rs1 < 0) && (rs1 != SR_HOSTDOWN))
	        rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("progoutput/prochosts: processed=%d up=%d\n",
	        pip->c_processed , pip->c_up) ;
#endif

	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("progoutput/prochosts: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (prochosts) */


/* free up all of the ping-hosts */
static int prochostsfins(pip,phlp)
struct proginfo	*pip ;
VECHAND		*phlp ;
{
	PINGHOST	*php ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;
	int	c = 0 ;


	for (i = 0 ; vechand_get(phlp,i,&php) >= 0 ; i += 1) {
	    if (php == NULL) continue ;

	    c += 1 ;
	    rs1 = pinghost_finish(php) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(php) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (prochostsfins) */


/* load the ping-hosts from a pingtab file */
static int loadpingtabs(pip,phlp)
struct proginfo	*pip ;
VECHAND		*phlp ;
{
	VECSTR	*ptp = &pip->pingtabs ;

	PINGTAB	pt ;

	int	rs = SR_OK ;
	int	i ;
	int	min, to ;
	int	c = 0 ;

	const char	*ptname ;
	const char	*ptfname ;

	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progoutput/loadpingtabs: defintmin=%d\n",
		pip->defintminping) ;
	    debugprintf("progoutput/loadpingtabs: toping=%d\n", pip->toping) ;
	}
#endif

	for (i = 0 ; vecstr_get(ptp,i,&ptname) >= 0 ; i += 1) {
	    if (ptname == NULL) continue ;

	    ptfname = ptname ;
	    if (mungepingtab(pip,ptname,tmpfname) > 0)
	        ptfname = tmpfname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progoutput/loadpingtabs: ptfname=%s\n",ptfname) ;
#endif

	    if ((rs = pingtab_open(&pt,ptfname)) >= 0) {
	        PINGTAB_ENT	pte ;

	        if (pip->debuglevel > 0)
	            bprintf(pip->efp,"%s: pt=%s\n",
	                pip->progname,ptname) ;

	        if (pip->open.logprog)
	            logfile_printf(&pip->lh,"pt=%s",ptname) ;

	        pip->c_pingtabs += 1 ;
	        while ((rs = pingtab_read(&pt,&pte)) > 0) {

	            min = pte.intminping ;
	            to = pte.timeout ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    	debugprintf("progoutput/loadpingtabs: host=%s min=%d to=%d\n",
			pte.hostname,min,to) ;
#endif

		    if (pip->intminping >= 0) {
		        if ((min < 0) || (pip->intminping < min))
			    min = pip->intminping ;
		    }
		    if (min < 0) min = pip->defintminping ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    	    debugprintf("progoutput/loadpingtabs: final min=%d\n",min) ;
#endif

	            c += 1 ;
	            rs = loadhost(pip,phlp,pte.hostname,-1,min,to) ;

	            if (rs < 0) break ;
	        } /* end while */

	        pingtab_close(&pt) ;
	    } else {

	        if (pip->debuglevel > 0)
	            bprintf(pip->efp,"%s: unaccessible pt=%s\n",
	                pip->progname,ptname) ;

	        if (pip->open.logprog)
	            logfile_printf(&pip->lh,"inaccessible pt=%s",ptname) ;

	    } /* end if (pingtab) */

	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progoutput/loadpingtabs: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpingtabs) */


/* load a single ping-host */
static int loadhost(pip,phlp,hp,hl,min,to)
struct proginfo	*pip ;
VECHAND		*phlp ;
const char	hp[] ;
int		hl ;
int		min ;
int		to ;
{
	PINGHOST	*php ;

	const int	size = sizeof(PINGHOST) ;

	int	rs ;


	if (hp == NULL) return SR_FAULT ;

	if ((to <= 0) && (pip->toping >= 0))
	    to = pip->toping ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progoutput/loadhost: host=%t min=%d to=%d\n",
			hp,hl,min,to) ;
#endif

	if ((rs = uc_malloc(size,&php)) >= 0) {
	    if ((rs = pinghost_start(php,hp,hl,min,to)) >= 0) {
	        rs = vechand_add(phlp,php) ;
	        if (rs < 0) pinghost_finish(php) ;
	    } /* end if (pinghost) */
	    if (rs < 0) uc_free(php) ;
	} /* end if (allocation) */

	return rs ;
}
/* end subroutine (loadhost) */


/* munge up the pingtab file names */
static int mungepingtab(pip,ptfname,tmpfname)
struct proginfo	*pip ;
const char	ptfname[] ;
char		tmpfname[] ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	cl ;

	const char	*cp ;


	cp = ptfname ;
	cl = getfname(pip->pr,cp,0,tmpfname) ;
	if (cl > 0)
	    cp = tmpfname ;

	if (((u_stat(cp,&sb) < 0) || S_ISDIR(sb.st_mode)) &&
	    (strchr(ptfname,'/') == NULL)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progoutput/mungepingtab: trying PTDNAME\n") ;
#endif

	    cp = PTDNAME ;
	    cl = mkpath3(tmpfname, pip->pr,cp,ptfname) ;

	    if (u_stat(tmpfname,&sb) >= 0)
	        rs = cl ;

	} else if (cl > 0)
	    rs = cl ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progoutput/mungepingtab: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mungepingtab) */



