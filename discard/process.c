/* process */

/* process a hostname */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1


/* revision history:

	= 96/03/01, David A­D­ Morano

	The subroutine was adapted from other programs that
	do similar things.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine processes one hostname at a time.  It should be
	called repeatedly, once for each host.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<logfile.h>
#include	<varsub.h>
#include	<vecstr.h>
#include	<dater.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"pingstatdb.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	LONGTIME
#define	LONGTIME	(5 * 60)
#endif

#define	RF_NUMDIGITS	8		/* count field width */
#define	RF_LOGZLEN	18		/* time log field width */
#define	RF_UPSTAT	1		/* up status field width */
#define	RF_LEAD0	(RF_NUMDIGITS + 2*RF_LOGZLEN)
#define	RF_LEAD1	(RF_UPSTAT + 4)
#define	RF_LEAD		(RF_LEAD0 + RF_LEAD1)

#define	RF_BUFLEN	(RF_LEAD + MAXHOSTNAMELEN + 3)

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	inetping(const char *,int) ;

extern char	*timestr_logz(time_t, char *) ;


/* external variables */


/* local structures */


/* local forward references */


/* global variables */


/* local variables */


/* exported subroutines */


int process(pip,dp,ep,psp,name,minpingint,to)
struct proginfo	*pip ;
VARSUB		*dp, *ep ;
PINGSTATDB	*psp ;
char		name[] ;
int		minpingint ;
int		to ;
{
	PINGSTATDB_ENT	*pep ;

	time_t	daytime ;
	time_t	lastcheck ;

	int	rs = SR_OK, rs1, rs_match ;
	int	c_up = -1 ;
	int	f_update ;
	int	f_ourself ;

	char	timebuf[TIMEBUFLEN + 1] ;


	if (name == NULL)
	    return SR_FAULT ;

	if (name[0] == '\0')
	    return SR_INVALID ;

/* OK, continue */

	f_update = pip->f.update ;
	u_time(&daytime) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("process: name=\"%s\" to=%d f_update=%d daytime=%s\n",
	        name,to,
	        f_update,
	        timestr_log(daytime,timebuf)) ;
	    debugprintf("process: minpingint=%d\n",minpingint) ;
	}
#endif

	rs_match = pingstatdb_match(psp,name,&pep) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: rs_match=%d\n", rs_match) ;
#endif

	c_up = ((rs_match >= 0) && pep->f_up) ? 0 : -1 ;


	f_ourself = FALSE ;
	if (f_update) {

/* are we trying to PING ourselves ! */

	    rs = vecstr_find(&pip->localnames,name) ;

	    f_ourself = (rs >= 0) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("process: f_ourself=%d\n", f_ourself) ;
#endif

	}

	if (f_update && (rs_match >= 0)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("process: turn off f_update ? \n") ;
#endif

	    if ((! f_ourself) || (! pep->f_up)) {

	        dater_gettime(&pep->pdate,&lastcheck) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("process: lastcheck=%s\n",
	                timestr_log(lastcheck,timebuf)) ;
#endif

	        if ((daytime - lastcheck) < minpingint)
	            f_update = FALSE ;

	    } else {

		c_up = 0 ;
	        f_update = FALSE ;

	    }

	} /* end if (deciding if an update was needed) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: f_update=%d\n",f_update) ;
#endif


	if (! f_update) {

	    rs = rs_match ;
	    if ((rs_match >= 0) && (! pep->f_up))
		rs = SR_HOSTDOWN ;

	}


	rs1 = SR_NOANODE ;
	if (f_update) {

	    int	f_up ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: updating host=%s\n",name) ;
#endif

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: updating host=%s\n",
	            pip->progname,name) ;

	    rs = SR_OK ;
	    f_up = TRUE ;
	    if (! f_ourself) {

	        if (to < 0)
	            to = LONGTIME ;

	        rs = inetping(name,to) ;

	        f_up = (rs >= 0) ;
		c_up = (rs >= 0) ? 1 : -1 ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process: inetping rs=%d\n",rs) ;
#endif

	    }

	    if ((rs != SR_HOSTDOWN) && (rs < 0)) {

	        rs1 = SR_NOTSUP ;
	        logfile_printf(&pip->lh,"inetping host=%s rs=%d\n",
	            name,rs) ;

	    } else {

	        int	f_state0, f_state1 ;
	        int	f_present ;


/* prior state if any */

	        f_present = FALSE ;
	        if (rs_match >= 0) {

	            f_present = TRUE ;
	            f_state0 = pep->f_up ;
	        }

/* new state */

	        f_state1 = f_up ;

/* update the DB with the new information */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("process: doing the update \n") ;
#endif

	        rs1 = pingstatdb_update(psp,name,f_state1,daytime) ;

/* record any changes */

	        if (f_present && (! LEQUIV(f_state0,f_state1))) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("process: host state change\n") ;
#endif

	            u_time(&daytime) ;

	            logfile_printf(&pip->lh,"%s %s %s\n",
	                timestr_logz(daytime,timebuf),
	                ((f_state1) ? "U" : "D"),
	                name) ;

/* should we make an entry in the summary file ? */

	            if (pip->sumfp != NULL) {

	                bprintf(pip->sumfp,"%s %s %s\n",
	                    timestr_logz(daytime,timebuf),
	                    ((f_state1) ? "U" : "D"),
	                    name) ;

	            } /* end if (summary file entry) */

	        } /* end if (any changes) */

	    } /* end if (doing the DB update) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("process: pingstatdb_update rs=%d\n",rs1) ;
#endif

	} /* end if (update) */

/* do we want to print a report on this one ? */

	if ((! pip->f.update) || (pip->verboselevel > 0)) {

	    char	cdate[RF_LOGZLEN + 2], pdate[RF_LOGZLEN + 2] ;


	    if ((rs1 >= 0) && (rs_match < 0))
	        rs_match = pingstatdb_match(psp,name,&pep) ;

	    if (! pip->f.nooutput) {

	    if (rs_match >= 0) {

	            bprintf(pip->ofp,"%c %s\n",
	                ((pep->f_up) ? 'U' : 'D'),
	                pep->hostname) ;

	        if (pip->verboselevel > 0) {

	            dater_mklogz(&pep->cdate,cdate,RF_LOGZLEN + 1) ;

	            dater_mklogz(&pep->pdate,pdate,RF_LOGZLEN + 1) ;

		    if (pip->verboselevel > 1) {

			time_t	ptime ;


			dater_gettime(&pep->pdate,&ptime) ;

	                bprintf(pip->ofp,"%*d %s %s (%s)\n",
	                    RF_NUMDIGITS,pep->count,
	                    cdate,pdate,
	                    timestr_elapsed((daytime - ptime),timebuf)) ;

		    } else
	                bprintf(pip->ofp,"%*d %s %s\n",
	                    RF_NUMDIGITS,pep->count,
	                    cdate,pdate) ;

	        }

	    } else {

	        bprintf(pip->ofp,"%c %s\n",
	            '-', name) ;

	        if (pip->verboselevel > 0)
	            bprintf(pip->ofp, "*\n") ;

	    }

	    } /* end if (output enabled) */

	} /* end if (printing report) */


	if ((rs >= 0) && (c_up < 0))
		rs = SR_HOSTDOWN ;

#if	CF_DEBUG
	if ((DEBUGLEVEL(2) && (rs < 0)) || DEBUGLEVEL(3))
	    debugprintf("process: exiting rs=%d c_up=%d\n",
		rs,c_up) ;
#endif

	return (rs >= 0) ? c_up : rs ;
}
/* end subroutine (process) */



