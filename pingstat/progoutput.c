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

        This subroutine forms the front-end part of a generic PCS type of
        program. This front-end is used in a variety of PCS programs.

        This subroutine was originally part of the Personal Communications
        Services (PCS) package but can also be used independently from it.
        Historically, this was developed as part of an effort to maintain high
        function (and reliable) email communications in the face of increasingly
        draconian security restrictions imposed on the computers in the DEFINITY
        development organization.


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
#include	"proglog.h"


/* local defines */


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	sfdirname(const char *,int,cchar **) ;
extern int	sfshrink(const char *,int,cchar **) ;
extern int	sfkey(const char *,int,cchar **) ;
extern int	nextfield(char *,int,cchar **) ;
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

extern int	progpingtabadd(PROGINFO *,const char *,int) ;
extern int	proghost(PROGINFO *,const char *,int,int) ;

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

static int	procargs(PROGINFO *,ARGINFO *,BITS *,vechand *) ;
static int	procdefpingtab(PROGINFO *) ;
static int	prochostsfins(PROGINFO *,VECHAND *) ;
static int	prochosts(PROGINFO *,VECHAND *) ;
static int	loadpingtabs(PROGINFO *,VECHAND *) ;
static int	loadhost(PROGINFO *,VECHAND *,cchar *,int,int,int) ;
static int	mungepingtab(PROGINFO *,const char *,char *) ;

static int	procout_begin(PROGINFO *,bfile *,const char *) ;
static int	procout_end(PROGINFO *) ;


/* local variables */


/* exported subroutines */


int progoutput(PROGINFO *pip,ARGINFO *aip,BITS *bop)
{
	bfile		ofile ;
	int		rs ;
	int		rs1 ;
	int		pan = 0 ;
	const char	*ofn = pip->ofname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("progoutput: ent defintmin=%d\n",
	        pip->defintminping) ;
	    debugprintf("progoutput: intmin=%d\n",pip->intminping) ;
	}
#endif

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = BFILE_STDOUT ;

	if ((rs = procout_begin(pip,&ofile,ofn)) >= 0) {
	    VECHAND	phosts ;
	    if ((rs = vechand_start(&phosts,20,0)) >= 0) {

		if ((rs = procargs(pip,aip,bop,&phosts)) >= 0) {
		    if ((rs = vecstr_count(&pip->pingtabs)) == 0) {
	        	if (pip->f.update) {
	            	    rs = procdefpingtab(pip) ;
	            	    pan += rs ;
			}
		    }
	            if (rs >= 0) {
	                if ((rs = loadpingtabs(pip,&phosts)) >= 0) {
	            	     rs = prochosts(pip,&phosts) ;
			}
		    }
	        } /* end if (procargs) */

/* free up the ping-hosts */

	        rs1 = prochostsfins(pip,&phosts) ;
	        if (rs >= 0) rs = rs1 ;

	        rs1 = vechand_finish(&phosts) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (phosts) */
	    rs1 = procout_end(pip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (procout) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progoutput: ret rs=%d pan=%u\n",rs,pan) ;
#endif

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (progoutput) */


/* local subroutines */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,vechand *php)
{
	const int	defintmin = pip->defintminping ;
	const int	to = pip->toping ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		pan = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	cchar		*afn = pip->afname ;
	cchar		*cp ;

	if (rs >= 0) {
	        int	ai ;
	        int	f ;
	        for (ai = 1 ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	            if (f) {
	                cp = aip->argv[ai] ;
			if (cp[0] != '\0') {
	            	    pan += 1 ;
	            	    rs = loadhost(pip,php,cp,-1,defintmin,to) ;
			}
		    }

	            if (rs < 0) break ;
	        } /* end for (looping through requested circuits) */
	} /* end if (ok) */

/* process any host names that are in the argument filename list file */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progoutput: afile arguments rs=%d pan=%u\n",
	                rs,pan) ;
#endif

	        if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	            bfile	afile, *afp = &afile ;

	            if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	            if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
			const int	dto = defintmin ;
	                const int	llen = LINEBUFLEN ;
			int		len ;
	                char		lbuf[LINEBUFLEN + 1] ;

	                while ((rs = breadline(afp,lbuf,llen)) > 0) {
	                    len = rs ;

	                    if (lbuf[len - 1] == '\n') len -= 1 ;
	                    lbuf[len] = '\0' ;

			    if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
	                        if (cp[0] == '#') {
	                    	    pan += 1 ;
	                    	    rs = loadhost(pip,php,cp,cl,dto,to) ;
				}
			    }

	                    if (rs < 0) break ;
	                } /* end while (reading lines) */

	                rs1 = bclose(afp) ;
			if (rs >= 0) rs = rs1 ;
	            } else {
			    fmt = "%s: inaccessible arg-list (%d)\n" ;
	                    bprintf(pip->efp,fmt,pn,rs) ;
			    fmt = "%s: afile=%s\n" ;
	                    bprintf(pip->efp,fmt,pn,afn) ;
	            } /* end if */

	        } /* end if (processing file argument file list) */

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (procargs) */


static int procout_begin(PROGINFO *pip,bfile *ofp,cchar *ofname)
{
	int		rs = SR_OK ;
	if (! pip->f.nooutput) {
	    if (pip->ofp == NULL) {
	        if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0) {
	            pip->ofp = ofp ;
	        }
	    }
	}
	return rs ;
}
/* end subroutine (procout_begin) */


static int procout_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->ofp != NULL) {
	    rs1 = bclose(pip->ofp) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->ofp = NULL ;
	}
	return rs ;
}
/* end subroutine (procout_end) */


static int procdefpingtab(PROGINFO *pip)
{
	int		rs ;
	const char	*dn = DEFPTFNAME ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progoutput/procdefpingtab: ent\n") ;
#endif

	rs = progpingtabadd(pip,dn,-1) ;

	return rs ;
}
/* end subroutine (procdefpingtab) */


/* process all of the ping-hosts */
static int prochosts(PROGINFO *pip,VECHAND *phlp)
{
	PINGHOST	*php ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		f = TRUE ;

	for (i = 0 ; vechand_get(phlp,i,&php) >= 0 ; i += 1) {
	    if (php != NULL) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("progoutput/prochosts: host=%s\n",php->name) ;
	            debugprintf("progoutput/prochosts: ->intminping=%d\n",
			php->intminping) ;
	            debugprintf("progoutput/prochosts: to=%d\n",php->to) ;
	        }
#endif

	        pip->c_hosts += 1 ;
	        rs1 = proghost(pip,php->name,php->intminping,php->to) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progoutput/prochosts: proghost() rs=%d\n",
			rs1) ;
#endif

	        if ((rs1 >= 0) || (rs1 == SR_HOSTDOWN))
	            pip->c_processed += 1 ;

	        if (rs1 >= 0)
	            pip->c_up += 1 ;

	        if (rs1 == SR_HOSTDOWN) {
	            f = FALSE ;
		    pip->f.hostdown = TRUE ;
		}

	        if ((rs1 < 0) && (rs1 != SR_HOSTDOWN))
	            rs = rs1 ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progoutput/prochosts: processed=%d up=%d\n",
	                pip->c_processed , pip->c_up) ;
#endif

	    }
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progoutput/prochosts: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (prochosts) */


/* free up all of the ping-hosts */
static int prochostsfins(PROGINFO *pip,VECHAND *phlp)
{
	PINGHOST	*php ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;

	if (pip == NULL) return SR_FAULT ;
	for (i = 0 ; vechand_get(phlp,i,&php) >= 0 ; i += 1) {
	    if (php != NULL) {
	        c += 1 ;
	        rs1 = pinghost_finish(php) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = uc_free(php) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progoutput/prochostsfins: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (prochostsfins) */


/* load the ping-hosts from a pingtab file */
static int loadpingtabs(PROGINFO *pip,VECHAND *phlp)
{
	VECSTR		*ptp = &pip->pingtabs ;
	PINGTAB		pt ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		min, to ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	const char	*ptname ;
	const char	*ptfname ;
	char		tmpfname[MAXPATHLEN + 1] ;

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
	                debugprintf("progoutput/loadpingtabs: "
			    "host=%s min=%d to=%d\n",pte.hostname,min,to) ;
#endif

	            if (pip->intminping >= 0) {
	                if ((min < 0) || (pip->intminping < min))
	                    min = pip->intminping ;
	            }
	            if (min < 0) min = pip->defintminping ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progoutput/loadpingtabs: "
				"final min=%d\n",min) ;
#endif

	            c += 1 ;
	            rs = loadhost(pip,phlp,pte.hostname,-1,min,to) ;

	            if (rs < 0) break ;
	        } /* end while */

	        rs1 = pingtab_close(&pt) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {

	        if (pip->debuglevel > 0) {
		    fmt = "%s: unaccessible pt=%s\n" ;
	            bprintf(pip->efp,fmt,pn,ptname) ;
		}

		    fmt = "inaccessible pt=%s" ;
	            proglog_printf(pip,fmt,ptname) ;

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
PROGINFO	*pip ;
VECHAND		*phlp ;
const char	hp[] ;
int		hl ;
int		min ;
int		to ;
{
	PINGHOST	*php ;
	const int	size = sizeof(PINGHOST) ;
	int		rs ;

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
static int mungepingtab(PROGINFO *pip,cchar *ptfname,char *tmpfname)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		cl ;
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

	    cp = ETCDNAME ;
	    cl = mkpath4(tmpfname,pip->pr,cp,PTDNAME,ptfname) ;

	    if (u_stat(tmpfname,&sb) >= 0)
	        rs = cl ;

	} else if (cl > 0) {
	    rs = cl ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progoutput/mungepingtab: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mungepingtab) */


