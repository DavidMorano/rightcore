/* progng */

/* process a newsgroup */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGLOOK	1		/* debug |procartlook()| */
#define	CF_DEBUGEMIT	0		/* debug |emit()| */
#define	CF_DIRSHOWN	1		/* |dirshown_()| */
#define	CF_ARTLOAD	1		/* art-load */


/* revision history:

	= 1994-01-17, David A­D­ Morano

	I have made major modifications from a previous version of this
	subroutine (which was total junk!).  The previous functions
	that are now in this subroutine were scattered all over the
	place in the past.  The code was previously unmaintainable!


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine processes articles on a particular newsgroup
	passed down as an argument using the user's newsgroup pointer
	entry.	Also passed down is what articles on the newsgroup are to
	be selected for processing ; namely, old articles, all articles,
	or new articles.

 	Synopsis:

	int progng(pip,sdp,dsp,emit,cmode)
	struct proginfo	*pip ;
	DIRSHOWN	*sdp ;
	MKDIRLIST_ENT	*dsp ;
	int		(*emit)() ;
	int		cmode ;

 	Arguments:

	pip		pointer to global variables
	sdp		Shown-Directory pointer
 	dsp 		pointer to user's board status structure to 
			be processed
 	emit		bulletin processing function
 			typically print bulletin or report title
	cmode		currency mode 
			CM_NEW -> new articles only
			CM_ALL -> all articles
			CM_OLD -> old articles only
 
	Returns:

	<0		error
	>=0		EMIT-code


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<fsdir.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"dirshown.h"
#include	"mkdirlist.h"
#include	"artlist.h"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	pathadd(char *,int,const char *) ;

extern int	bbcpy(char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* forward references */

static int procartload(struct proginfo *,DIRSHOWN *,ARTLIST *,
		MKDIRLIST_ENT *) ;
static int procartdir(struct proginfo *,ARTLIST *,const char *) ;
static int procartlook(struct proginfo *,ARTLIST *,MKDIRLIST_ENT *,
		int (*)(struct proginfo *,...),int) ;


/* local (static) variables */

static const char	*ignorefiles[] = {
	"core",
	NULL
} ;


/* exported subroutines */


int progng(pip,sdp,dsp,emit,cmode)
struct proginfo	*pip ;
DIRSHOWN	*sdp ;
MKDIRLIST_ENT	*dsp ;
int		(*emit)(struct proginfo *,...) ;
int		cmode ;
{
	struct timeb	now = pip->now ;

	ARTLIST		al ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	retval = EMIT_DONE ;

	char	timebuf1[TIMEBUFLEN + 1] ;
	char	timebuf2[TIMEBUFLEN + 1] ;
	char	*timebuf = timebuf1 ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progng: entered w/ umt=%s cmode=%d\n",
	        timestr_log(dsp->mtime,timebuf1),cmode) ;
#endif

	if (sdp == NULL) return SR_FAULT ;
	if (dsp == NULL) return SR_FAULT ;

/* initialization functions */

	if ((rs = artlist_start(&al,&now,pip->zname)) >= 0) {
#if	CF_ARTLOAD
	    if ((rs = procartload(pip,sdp,&al,dsp)) >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
		    int	i ;
	            const char	*np ;
	            time_t	a ;
	            debugprintf("progng: artlist so far¬\n") ;
	            for (i = 0 ; artlist_get(&al,i,NULL,&np,&a) >= 0 ; i += 1)
	                debugprintf("progng: i=%d name=%s\n",i,np) ;
	        }
#endif /* CF_DEBUG */

/* for those verbose users, give them a little extra */

#ifdef	COMMENT
	        if (pip->verboselevel > 1)
	            bprintf(pip->ofp,"newsgroup=%s\n",ngname) ;
#endif

/* sort the article list */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progng: sorting\n") ;
#endif /* CF_DEBUG */

	        artlist_sort(&al,pip->sortmode,pip->f.reverse) ;

/* look at articles as appropriate given the currency mode */

#if	CF_DEBUGLOOK
	        rs = procartlook(pip,&al,dsp,emit,cmode) ;
	        retval = rs ;
#endif

	    } /* end if (proc-load-arts) */
#endif /* CF_ARTLOAD */
	    artlist_finish(&al) ;
	} /* end if (artlist) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progng: ret rs=%d retval=%d\n",rs,retval) ;
#endif

	return (rs >= 0) ? retval : rs ;
}
/* end subroutine (progng) */


/* local subroutines */


static int procartload(pip,sdp,alp,dsp)
struct proginfo	*pip ;
DIRSHOWN	*sdp ;
ARTLIST		*alp ;
MKDIRLIST_ENT	*dsp ;
{
	MKDIRLIST_ENT	*dsp2 ;
	int	rs = SR_OK ;
	int	rs1 ;

	while ((rs >= 0) && (dsp != NULL)) {

#if	CF_DIRSHOWN
	    rs1 = dirshown_already(sdp,dsp,&dsp2) ;
	    if (rs1 == SR_NOTFOUND) {
	        if ((rs = procartdir(pip,alp,dsp->name)) >= 0)
	            rs = dirshown_set(sdp,dsp) ;
	    } /* end if (checking if directory was shown before) */
#else /* CF_DIRSHOWN */
	        rs = procartdir(pip,alp,dsp->name) ;
#endif /* CF_DIRSHOWN */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progng: bottom loop\n") ;
#endif /* CF_DEBUG */

	    dsp = dsp->link ;
	    if (rs < 0) break ;
	} /* end while (looping through directories of this newsgroup) */

	return rs ;
}
/* end subroutine (procartload) */


static int procartdir(pip,alp,ngd)
struct proginfo	*pip ;
ARTLIST		*alp ;
const char	ngd[] ;
{
	fsdir		dir ;
	fsdir_ent	ds ;

	int		rs ;
	int		c = 0 ;

	const char	*nd = pip->newsdname ;

	char	apath[MAXPATHLEN + 2] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procartdir: entered ngd=%s\n",ngd) ;
#endif

	if (ngd == NULL)
	    return SR_FAULT ;

	if ((rs = mkpath2(apath,nd,ngd)) >= 0) {
	    int	alen = rs ;

	    if ((rs = fsdir_open(&dir,apath)) >= 0) {

	        while ((rs = fsdir_read(&dir,&ds)) > 0) {
	            if (ds.name[0] == '.') continue ;
	            if (matstr(ignorefiles,ds.name,-1) < 0) {
		        if ((rs = pathadd(apath,alen,ds.name)) >= 0) {
	                    c += 1 ;
	        	    rs = artlist_add(alp,ngd,apath) ;
		        }
		    }
	        } /* end while (reading directory entries) */

	        fsdir_close(&dir) ;
	    } /* end if (opened directory) */

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procartdir: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procartdir) */


static int procartlook(pip,alp,dsp,emit,cmode)
struct proginfo	*pip ;
ARTLIST		*alp ;
MKDIRLIST_ENT	*dsp ;
int		(*emit)(struct proginfo *,...) ;
int		cmode ;
{
	struct timeb	now = pip->now ;

	ARTLIST_ENT	*aep ;

	time_t	mtime_seen = DATE1970 ;
	time_t	amt ;

	int	rs = SR_OK ;
	int	ai ;
	int	retval = EMIT_OK ;
	int	c = 0 ;
	int	f_previous = FALSE ;
	int	f_exit = FALSE ;
	int	f ;

	const char	*fmt ;
	const char	*fname ;
	const char	*ngd ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progng/procartlook: looping through articles\n") ;
#endif /* CF_DEBUG */

	for (ai = 0 ; artlist_get(alp,ai,&ngd,&fname,&amt) >= 0 ; ai += 1) {
	    if (fname == NULL) continue ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
		char	timebuf[TIMEBUFLEN+1] ;
	        debugprintf("progng/procartlook: ai=%d mtime=%s\n",ai,
	            timestr_log(dsp->mtime,timebuf)) ;
	        debugprintf("progng/procartlook: ngd=%s\n",
	            ngd) ;
	        debugprintf("progng/procartlook: fname=%s\n",
	            fname) ;
	        debugprintf("progng/procartlook: amt=%s\n",
	            timestr_log(amt,timebuf)) ;
	    }
#endif /* CF_DEBUG */

/* look at articles with the correct currency */

	    f = (cmode == CM_ALL) ;
	    f = f || f_previous ;
	    f = f || ((cmode == CM_NEW) && (amt > dsp->utime)) ;
	    f = f || ((cmode == CM_OLD) && (amt <= dsp->utime)) ;

	    if (f && ((rs = artlist_getentry(alp,ai,&aep)) >= 0)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progng/procartlook: interactive?\n") ;
#endif

	        if (pip->f.interactive)
	            bflush(pip->ofp) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progng/procartlook: calling emit\n") ;
#endif

	        {
	            const int	blen = MAXNAMELEN ;
	            int		bl ;
	            const char	*bp ;
	            char	bname[MAXNAMELEN+1] ;
	            if ((bl = sfbasename(fname,-1,&bp)) > 0) {
	                if ((rs = sncpy1w(bname,blen,bp,bl)) > 0) {

#if	CF_DEBUGEMIT
#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("progng/procartlook: emit ai=%d\n",ai) ;
	            debugprintf("progng/procartlook: ngd=%s\n",ngd) ;
	            debugprintf("progng/procartlook: bname=%s\n",bname) ;
	        }
#endif
#else
	                    rs = (*emit)(pip,dsp,ai,aep,ngd,bname) ;
	                    retval = rs ;
#endif


			}
	            }
	        } /* end block */

/* back from EMIT */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
		    char	timebuf[TIMEBUFLEN+1] ;
	            debugprintf("progng/procartlook: emit() rv=%d\n",retval) ;
	            debugprintf("progng/procartlook: mtime_seen=%s\n",
	                timestr_log(mtime_seen,timebuf)) ;
	            debugprintf("progng/procartlook: amt=%s\n",
	                timestr_log(amt,timebuf)) ;
	        }
#endif /* CF_DEBUG */

/* handle things (like a user specified "quit") */

		if (rs >= 0) {
	        switch (retval) {

	        case EMIT_OK:
	        case EMIT_NEXT:
	            f_previous = FALSE ;
	            mtime_seen = MAX(mtime_seen,amt) ;
	            c += 1 ;
	            break ;

	        case EMIT_SKIP:
	        case EMIT_QUIT:
	            f_exit = TRUE ;
	            mtime_seen = MAX(mtime_seen,amt) ;
	            break ;

	        case EMIT_PREVIOUS:
	            if (ai <= 0) {
	                fmt = "no previous articles are available\n" ;
	                ai -= 1 ;
	                bprintf(pip->ofp,fmt) ;
	            } else {
	                f_previous = TRUE ;
	                ai -= 2 ;
	                c -= 2 ;
	            }
	            break ;

	        case EMIT_DONE:
	        case EMIT_SAVE:
	            mtime_seen = MAX(mtime_seen,amt) ;
	            break ;

	        } /* end switch */
		} /* end if */

	    } /* end if (of looking at this article) */

	    if (f_exit || (retval == EMIT_QUIT)) break ;
	    if (rs < 0) break ;
	} /* end for (looping through articles) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progng/procartlook: for-out rs=%d\n", rs) ;
#endif /* CF_DEBUG */

/* call the same 'emit' routine with NULL final argument (for some reason) */

#ifdef	COMMENT
	if ((rs >= 0) && (retval >= 0))
	    (void) (*emit)(pip,ngname,0,-1,NULL) ;
#endif

	if (rs >= 0) {
	    dsp->mtime = MAX(dsp->mtime,mtime_seen) ;
	    pip->count += c ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    char	timebuf1[TIMEBUFLEN+1] ;
	    char	timebuf2[TIMEBUFLEN+1] ;
	    debugprintf("progng/procartlook: ret retval=%d\n",
	        retval) ;
	    debugprintf("progng/procartlook: mt=%s old_umt=%s\n",
	        timestr_log(mtime_seen,timebuf1),
	        timestr_log(dsp->mtime,timebuf2)) ;
	}
#endif

	return (rs >= 0) ? retval : rs ;
}
/* end subroutine (procartlook) */


