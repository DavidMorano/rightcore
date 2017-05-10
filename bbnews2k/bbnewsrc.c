/* bbnewsrc */

/* object to handle the user's BBNEWSRC file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1994-03-01, David A­D­ Morano

        We collect the code that accesses the user currency file pretty much
        into one place. The functions handled by this module were previously
        scattered around in the past!

	= 1998-11-13, David A­D­ Morano

        This is enhanced from the older version of the same (that I wrote back
        in the early 90s).


*/

/* Copyright © 1994,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object module accesses the user currency file. The user currency
        file is usually named '.bbnewsrc' and is normally located in the user's
        home directory.

        Having all of the reading and writing of the user currency file in one
        object module makes it a lot easier to handle things like the Y2K thing.
        The BBnews code was Y2K compliant already but this new object here makes
        it easier to change the user currency file in the future.


*******************************************************************************/


#define	BBNEWSRC_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>

#include	"bbnewsrc.h"


/* local object defines */

#define	BBNEWSRC_MAGIC	0x12341413

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY	100
#endif

#ifndef	DATE1970
#define	DATE1970	(24 * 3600)
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	mkpath1w(char *,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	siskipwhite(const char *,int) ;
extern int	cfdecul(const char *,int,ulong *) ;
extern int	dater_getbbtime(DATER *,const char *,int,time_t *) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	bbnewsrc_ent(BBNEWSRC *,BBNEWSRC_ENT *,const char *,int) ;


/* local variables */


/* exported subroutines */


int bbnewsrc_open(BBNEWSRC *ungp,cchar ufname[],int f_readtime)
{
	int		rs ;
	int		f_opened = FALSE ;
	const char	*cp ;

#if	CF_DEBUGS
	debugprintf("bbnewsrc_open: ent\n") ;
	debugprintf("bbnewsrc_open: ufn=%s\n",ufname) ;
#endif

	if (ungp == NULL) return SR_FAULT ;
	if (ufname == NULL) return SR_FAULT ;

	if (ufname[0] == '\0') return SR_INVALID ;

	memset(ungp,0,sizeof(BBNEWSRC)) ;
	ungp->f.readtime = f_readtime ;
	ungp->f.wroteheader = FALSE ;

	if ((rs = uc_mallocstrw(ufname,-1,&cp)) >= 0) {
	    ungp->ufname = cp ;
	    if ((rs = bopen(&ungp->nf,ufname,"rw",0664)) >= 0) {
	        ungp->f.fileopen = TRUE ;
		f_opened = TRUE ;
#if	CF_DEBUGS
		debugprintf("bbnewsrc_open: opened\n") ;
#endif
	    } else if (rs == SR_NOENT) {
		rs = SR_OK ;
	    }
	    if (rs >= 0) {
		DATER	*dp = &ungp->tmpdate ;
	        if ((rs = dater_start(dp,NULL,NULL,0)) >= 0) {
	            ungp->magic = BBNEWSRC_MAGIC ;
	        }
	        if (rs < 0) {
	            if (ungp->f.fileopen) {
	                ungp->f.fileopen = FALSE ;
		        bclose(&ungp->nf) ;
	            }
	        }
	    }
	    if (ungp->ufname != NULL) {
		uc_free(ungp->ufname) ;
		ungp->ufname = NULL ;
	    }
	} /* end if (file-name allocation) */

#if	CF_DEBUGS
	debugprintf("bbnewsrc_open: ret rs=%d f_opened=%u\n",rs,f_opened) ;
#endif

	return (rs >= 0) ? f_opened : rs ;
}
/* end subroutine (bbnewsrc_open) */


/* close this object */
int bbnewsrc_close(BBNEWSRC *ungp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ungp == NULL) return SR_FAULT ;

	if (ungp->magic != BBNEWSRC_MAGIC) return SR_NOTOPEN ;

	if (ungp->f.fileopen) {

/* if we were writing, write the trailer stuff */

	if (ungp->f.wroteheader) {
	    offset_t	off ;
	    bprintf(&ungp->nf,"\n\n") ;
	    btell(&ungp->nf,&off) ;
	    bcontrol(&ungp->nf,BC_TRUNCATE,off) ;
	}

	rs1 = bclose(&ungp->nf) ;
	if (rs >= 0) rs = rs1 ;

	} /* end if (file-was-open) */

	rs1 = dater_finish(&ungp->tmpdate) ;
	if (rs >= 0) rs = rs1 ;

	if (ungp->ufname != NULL) {
	    rs1 = uc_free(ungp->ufname) ;
	    if (rs >= 0) rs = rs1 ;
	    ungp->ufname = NULL ;
	}

	return rs ;
}
/* end subroutine (bbnewsrc_close) */


/* read an entry from the file */
int bbnewsrc_read(BBNEWSRC *ungp,BBNEWSRC_ENT *ep)
{
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		len ;
	int		c = 0 ;
	char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("bbnewsrc_read: ent\n") ;
#endif

	if (ungp == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;

	if (ungp->magic != BBNEWSRC_MAGIC) return SR_NOTOPEN ;

	memset(ep,0,sizeof(BBNEWSRC_ENT)) ;
	ep->f_subscribed = FALSE ;

/* return OK (EOF) if we didn't have a file to open originally */

	if (! ungp->f.fileopen)
		return SR_OK ;

/* read the line since the file was opened */

	while ((rs = breadline(&ungp->nf,lbuf,llen)) > 0) {
	    len = rs ;

	    if (lbuf[len - 1] == '\n') len -= 1 ;
	    lbuf[len] = '\0' ;

	    {
		const char	*lp ;
		int		ll ;
		if ((ll = sfshrink(lbuf,len,&lp)) > 0) {
		    if (lp[0] != '#') {
	                rs = bbnewsrc_ent(ungp,ep,lp,ll) ;
			if (rs < INT_MAX) c += 1 ;
		    }
		}
	    }

	    if (c > 0) break ;

	    ungp->line += 1 ;
	    if (rs < 0) break ;
	} /* end while (looping through the user's list file) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bbnewsrc_read) */


/* write an entry to the file */
int bbnewsrc_write(BBNEWSRC *ungp,char ng[],int sf,time_t mtime)
{
	DATER		d ;
	time_t		daytime = 0 ;
	int		rs = SR_OK ;
	char		timebuf[TIMEBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("bbnewsrc_write: ent\n") ;
#endif

	if (ungp == NULL) return SR_FAULT ;

	if (ungp->magic != BBNEWSRC_MAGIC) return SR_NOTOPEN ;

/* open the file if we couldn't originally */

	if (! ungp->f.fileopen) {

		rs = bopen(&ungp->nf,ungp->ufname,"rwc",0664) ;

		if (rs < 0)
			return rs ;

		ungp->f.fileopen = TRUE ;

	} /* end if (opening the file for writing) */

/* write the file header if we didn't already */

	if (! ungp->f.wroteheader) {

#if	CF_DEBUGS
	    debugprintf("bbnewsrc_write: writing header\n") ;
#endif

	    daytime = time(NULL) ;

/* output the newsgroup list file header comments */

	    bprintf(&ungp->nf,"# PCS user newsgroup list file\n") ;

	    bprintf(&ungp->nf,"#\n") ;

	    bprintf(&ungp->nf,"# updated %s (%lu)\n",
	        timestr_logz(daytime,timebuf),
	        daytime) ;

	    bprintf(&ungp->nf,"#\n\n") ;

	    ungp->f.wroteheader = TRUE ;

	} /* end if (writing header stuff) */

/* write the NG entry information that we have */

	if ((mtime != 0) && (mtime > DATE1970)) {

#if	CF_DEBUGS
	    debugprintf("bbnewsrc_write: non-zero mtime=%s\n",
	        timestr_log(mtime,timebuf)) ;
#endif

	    if (ungp->f.readtime) {

		if ((rs = dater_start(&d,NULL,NULL,0)) >= 0) {

#if	CF_DEBUGS
	        debugprintf("bbnewsrc_write: readable time\n") ;
	        debugprintf("bbnewsrc_write: local mtime=%s\n",
			timestr_log(mtime,timebuf)) ;
#endif

		dater_settimezn(&d,mtime,NULL,-1) ;

#if	CF_DEBUGS
	        debugprintf("bbnewsrc_write: internal=%s\n",
			timestr_log(d.b.time,timebuf)) ;
#endif

		dater_mkstrdig(&d,timebuf,TIMEBUFLEN) ;

#if	CF_DEBUGS
	        debugprintf("bbnewsrc_write: writing mtime=%s\n",
			timebuf) ;
#endif

	        rs = bprintf(&ungp->nf,"%s%c %s\n",
	            ng, (sf ? ':' : '!'), timebuf) ;

		dater_finish(&d) ;
		} /* end if (dater) */

	    } else {

#if	CF_DEBUGS
	        debugprintf("bbnewsrc_write: integer time\n") ;
#endif

	        bprintf(&ungp->nf,"%s%c %lu\n",
	            ng,
	            (sf ? ':' : '!'),
	            ((ulong) mtime)) ;

	    }

	} else {

#if	CF_DEBUGS
	    debugprintf("bbnewsrc_write: zero mtime\n") ;
#endif

	    rs = bprintf(&ungp->nf,"%s%c %d\n",
	        ng,
	        (sf ? ':' : '!'),0) ;

	}

	return rs ;
}
/* end subroutine (bbnewsrc_write) */


/* rewind the file */
int bbnewsrc_rewind(BBNEWSRC *ungp)
{
	int		rs ;

	if (ungp == NULL) return SR_FAULT ;

	if (ungp->magic != BBNEWSRC_MAGIC) return SR_NOTOPEN ;

	rs = bseek(&ungp->nf,0L,SEEK_SET) ;

	return rs ;
}
/* end subroutine (bbnewsrc_rewind) */


/* local subroutines */


static int bbnewsrc_ent(BBNEWSRC *ungp,BBNEWSRC_ENT *ep,cchar lbuf[],int llen)
{
	int		rs = SR_OK ;
	int		si ;
	int		cl ;
	int		ngl = -1 ;
	int		dsl = -1 ;
	const char	*tp, *cp ;
	const char	*ngp = NULL ;
	const char	*dsp = NULL ;

#if	CF_DEBUGS
	debugprintf("bbnewsrc_ent: ent\n") ;
#endif

	memset(ep,0,sizeof(BBNEWSRC_ENT)) ;

	if ((si = siskipwhite(lbuf,llen)) > 0) {
	   lbuf += si ;
	   llen -= si ;
	}

#if	CF_DEBUGS
	debugprintf("bbnewsrc_ent: l=>%t<\n",lbuf,llen) ;
#endif

	ngp = lbuf ;
	if ((tp = strnpbrk(lbuf,llen,":!")) != NULL) {
	    ngl = (tp-lbuf) ;
	    dsp = (tp+1) ;
	    dsl = ((lbuf+llen)-(tp+1)) ;
	    ep->f_subscribed = (*tp == ':') ;
	}

#if	CF_DEBUGS
	debugprintf("bbnewsrc_ent: ng=%t\n",ngp,ngl) ;
#endif

	if ((cl = sfshrink(ngp,ngl,&cp)) > 0) {
	    if ((rs = mkpath1w(ep->ngname,cp,cl)) >= 0) {
#if	CF_DEBUGS
		debugprintf("bbnewsrc_ent: ds=%t\n",dsp,dsl) ;
#endif
		if ((dsp != NULL) && ((cl = sfshrink(dsp,dsl,&cp)) > 0)) {
		    DATER	*dp = &ungp->tmpdate ;
		    time_t	tv ;
#if	CF_DEBUGS
		    debugprintf("bbnewsrc_ent: ts=%t\n",cp,cl) ;
#endif
	            if ((rs = dater_getbbtime(dp,cp,cl,&tv)) >= 0) {
			ep->mtime = tv ;
		    }
		}
	    } /* end if (memory-allocation) */
	} /* end if (had a newsgroup name) */

/* this is an old convention also (zero-time -> not-subscribed) */

	if (ep->mtime == 0)
	    ep->f_subscribed = FALSE ;

#if	CF_DEBUGS
	{
	    char	timebuf[TIMEBUFLEN+1] ;
	    timestr_log(ep->mtime,timebuf) ;
	    debugprintf("bbnewsrc_read: user NG=%s umt=%s\n",
	        ep->ngname,timebuf) ;
	}
#endif

	return rs ;
}
/* end subroutine (bbnewsrc_ent) */


