/* progmoder */

/* process a newsgroup (an entire newsgroup) */


#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_SHOW		1		/* actually show */


/* revision history:

	= 1994-01-17, David A­D­ Morano

	I have made major modifications from a previous version of this
	subroutine (which was total junk!).  The previous functions that are
	now in this subroutine were scattered all over the place in the past.
	The code was previously unmaintainable!


*/

/* Copyright © 1994,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is the main routine that processes newsgroups.  It
	switches on the current program mode and performs the appropriate
	actions for the given mode.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<buffer.h>
#include	<expcook.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"mkdirlist.h"
#include	"dirshown.h"
#include	"artlist.h"


/* local defines */

#undef	LINEFOLDLEN
#define	LINEFOLDLEN	78


/* external subroutines */

extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	isNotPresent(int) ;

extern int	progng(PROGINFO *,DIRSHOWN *,MKDIRLIST_ENT *,
			int (*)(PROGINFO *,...),int) ;
extern int	bbcpy(char *,cchar *) ;
extern int	emit_test(PROGINFO *,...) ;
extern int	emit_article(PROGINFO *,...) ;
extern int	emit_count(PROGINFO *,...) ;
extern int	emit_header(PROGINFO *,...) ;
extern int	emit_mailbox(PROGINFO *,...) ;

extern char	*strnchr(cchar *,int,int) ;


/* external variables */


/* forward references */

static int	procqueryout(PROGINFO *,bfile *,int) ;
static int	procqueryoutprep(PROGINFO *,EXPCOOK *,int) ;
static int	description(PROGINFO *,bfile *,MKDIRLIST_ENT *) ;


/* local variables */

#if	CF_DEBUGS || CF_DEBUG
static cchar	*progmodes[] = {
	"read",
	"header",
	"names",
	"count",
	"subscription",
	"mailbox",
	NULL
} ;
#endif /* CF_DEBUG */


/* exported subroutines */


int progmoder(PROGINFO *pip,MKDIRLIST *ngp,cchar *ofn)
{
	MKDIRLIST_ENT	*dsp ;
	bfile		ofile, *ofp = &ofile ;
	const int	pm = pip->progmode ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		cmode ;
	int		c = 0 ;
	char		namebuf[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("progmoder: entered w/ progmode=%s(%d)\n",
	        progmodes[pm],pm) ;
	    debugprintf("progmoder: popscreen=%u\n",
	        pip->f.popscreen) ;
	}
#endif /* CF_DEBUG */

	cmode = CM_NEW ;
	if (pip->f.old) cmode = CM_OLD ;
	if (pip->f.all) cmode = CM_ALL ;

	if ((ofn == NULL) || (ofn[0] == '=') || (ofn[0] == '\0'))
	    ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",0644)) >= 0) {
	    DIRSHOWN	ds ;
	    if ((rs = dirshown_start(&ds)) >= 0) {
	        int	i ;
	        for (i = 0 ; mkdirlist_get(ngp,i,&dsp) >= 0 ; i += 1) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("progmoder: dirlist rs=%d i=%d\n",rs,i) ;
#endif
#if	CF_SHOW
	            if (dsp->f.show || pip->f.every) {
	                pip->count = 0 ;
	                switch (pm) {

	                case PM_TEST:
	                    pip->ofp = ofp ;
	                    rs = progng(pip,&ds,dsp,emit_test,cmode) ;
	                    break ;

	                case PM_READ:
	                    pip->ofp = ofp ;
	                    rs = progng(pip,&ds,dsp,emit_article,cmode) ;
	                    break ;

	                case PM_HEADER:
	                    pip->ofp = ofp ;
	                    pip->f.header = TRUE ;
	                    rs = progng(pip,&ds,dsp,emit_header,cmode) ;
	                    break ;

/* count new bulls */
	                case PM_COUNT:
	                    pip->ofp = ofp ;
	                    rs = progng(pip,&ds,dsp,emit_count,cmode) ;
#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("progmoder: progng() rs=%d\n",rs) ;
#endif
	                    if (pip->f.query) {
	                        if (pip->count > 0)
	                            pip->f.newmessages = TRUE ;
	                    } else if (pip->f.all || pip->f.old || 
	                        pip->f.description || (pip->count > 0)) {
	                        bbcpy(namebuf,dsp->name) ;
	                        bprintf(ofp,"newsgroup> %s %d\n",
	                            namebuf,pip->count) ;
	                        if (pip->f.description)
	                            rs = description(pip,ofp,dsp) ;
	                    } /* end if */
	                    break ;

/* report subscribed newsgroup names that have unread articles */
	                case PM_NAMES:
	                    bbcpy(namebuf,dsp->name) ;
	                    if (pip->f.description) {
	                        bprintf(ofp,"newsgroup> %s\n",namebuf) ;
	                        rs = description(pip,ofp,dsp) ;
	                    } else
	                        rs = bprintf(ofp,"%s\n",namebuf) ;
	                    break ;

/* handle newsgroup subscription here */
	                case PM_SUBSCRIPTION:
	                    dsp->f.subscribe = pip->f.subscribe ;
	                    break ;

	                case PM_MAILBOX:
	                    pip->ofp = ofp ;
	                    rs = progng(pip,&ds,dsp,emit_mailbox,cmode) ;
	                    break ;

	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch */
	                c += pip->count ;
	            } /* end if (show) */
#endif /* CF_SHOW */
	            if (rs == EMIT_QUIT) break ;
	            if (rs < 0) break ;
	        } /* end for (dir-lists) */
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("progmoder: dirlist-out rs=%d i=%d\n",
			rs,i) ;
#endif
	        rs1 = dirshown_finish(&ds) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (dirshown) */
	    if (rs >= 0) {
	        if ((pip->progmode == PM_COUNT) && pip->f.query) {
	            if (pip->f.newmessages)
	                rs = procqueryout(pip,ofp,c) ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("progmoder: progqueryout() rs=%d\n",rs) ;
#endif
	        } /* end if */
	    } /* end if */
	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (output-file-open) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmoder: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (progmoder) */


/* local subroutines */


static int procqueryout(PROGINFO *pip,bfile *ofp,int tc)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->querytext != NULL) {
	    cchar	*qt = pip->querytext ;
	    cchar	*tp ;
	    char	*qbuf = NULL ;
	    if ((tp = strchr(qt,'%')) != NULL) {
	        BUFFER	b ;
	        int	size = (strlen(qt) + 5) ;
	        if ((rs = buffer_start(&b,size)) >= 0) {
	            EXPCOOK	ec ;
	            if ((rs = expcook_start(&ec)) >= 0) {
	                if ((rs = procqueryoutprep(pip,&ec,tc)) >= 0) {
	                    if ((rs = expcook_expbuf(&ec,0,&b,qt,-1)) >= 0) {
	                        cchar	*sp ;
	                        if ((rs = buffer_get(&b,&sp)) >= 0)
	                            rs = bprintline(ofp,sp,rs) ;
	                    } /* end if (buf-exp) */
	                } /* end if (prep) */
	                rs1 = expcook_finish(&ec) ;
			if (rs >= 0) rs = rs1 ;
	            } /* end if (expcook) */
	            rs1 = buffer_finish(&b) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (buffer) */
	    } else
	        rs = bprintline(ofp,qt,-1) ;
	} /* end if */

	return rs ;
}
/* end subroutine (procqueryout) */


static int procqueryoutprep(pip,ecp,tc)
PROGINFO	*pip ;
EXPCOOK		*ecp ;
int		tc ;
{
	const int	dlen = DIGBUFLEN ;
	int		rs ;
	char		dbuf[DIGBUFLEN+1] ;

	if ((rs = ctdeci(dbuf,dlen,tc)) >= 0) {
	    rs = expcook_add(ecp,"n",dbuf,rs) ;
	}

	return rs ;
}
/* end subroutine (procqueryoutprep) */


static int description(pip,ofp,dsp)
PROGINFO	*pip ;
bfile		*ofp ;
MKDIRLIST_ENT	*dsp ;
{
	bfile		dfile ;
	const int	llen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		len ;
	int		wlen = 0 ;
	cchar		*nd = pip->newsdname ;
	cchar		*df = DESCFNAME ;
	char		fname[MAXPATHLEN + 1] ;
	char		*lbuf = fname ;

	while (dsp != NULL) {

	    if ((rs = mkpath3(fname,nd,dsp->name,df)) >= 0) {
	        if ((rs = bopen(&dfile,fname,"r",0666)) >= 0) {

	            while ((rs = breadline(&dfile,lbuf,llen)) > 0) {
	                len = rs ;
	                if (lbuf[len-1] == '\n') len -= 1 ;
	                if (len > 0) {
	                    rs = bprintf(ofp,"  %t\n",fname,len) ;
	                    wlen += rs ;
	                }
	                if (rs < 0) break ;
	            } /* end while */

	            bclose(&dfile) ;
	        } else if (isNotPresent(rs))
	            rs = SR_OK ;
	    } /* end if (mkpath) */

	    dsp = dsp->link ;
	    if (rs < 0) break ;
	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (description) */


