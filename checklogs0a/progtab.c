/* progtab */

/* process a log table file */


#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGDETAIL	0		/* extra debugging */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The subroutine was adapted from other programs that do similar
	things.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	In this subroutine we process 'logtab' files.  The input is taken
	as a 'logtab file.  We process each 'logtab' file by calling
	'proglogent()' for each entry.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<varsub.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	cfdecmfui(const char *,int,uint *) ;

extern int	proglogent(struct proginfo *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

#if	CF_DEBUGS
extern int	mkhexstr(char *,int,void *,int) ;
#endif


/* local variables */


/* exported subroutines */


int progtab(pip,lnp,lnl)
struct proginfo	*pip ;
const char	lnp[] ;
int		lnl ;
{
	bfile		ltfile, *ltp = &ltfile ;
	uint		logsize ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		len = 0 ;
	const char	*sizespec ;
	const char	*sp, *cp ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		logfname[MAXPATHLEN+1] ;

	if (lnp == NULL) return SR_FAULT ;

	if (lnp[0] == '\0') return SR_INVALID ;

	if (lnl < 0) lnl = strlen(lnp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progtab: logtab=%t\n",lnp,lnl) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: logtab=%t\n",
	        pip->progname,lnp,lnl) ;
	}

	if (pip->open.logprog)
	    logfile_printf(&pip->lh,"logtab=%t\n",lnp,lnl) ;

	if (lnp[lnl] != '\0') {
	    rs = snwcpy(tmpfname,MAXPATHLEN,lnp,lnl) ;
	    lnp = tmpfname ;
	}

	if (rs < 0)
	    goto ret0 ;

/* start in */

	pip->c_logtabs += 1 ;
	memset(&pip->stab,0,sizeof(struct proginfo_stat)) ;

/* open the LOGTAB file */

	if ((lnp[0] == '\0') || (lnp[0] == '-')) lnp = BFILE_STDIN ;

	if ((rs = bopen(ltp,lnp,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    char	lbuf[LINEBUFLEN + 1] ;

	while ((rs = breadline(ltp,lbuf,llen)) > 0) {
	    len = rs ;

	    if (lbuf[len - 1] != '\n') {
	        int	ch ;
	        while ((ch = bgetc(ltp)) >= 0) {
	            if (ch == '\n') break ;
	        }
	        continue ;
	    }

	    lbuf[--len] = '\0' ;
	    cp = lbuf ;

#if	CF_DEBUG && CF_DEBUGDETAIL
	    if (DEBUGLEVEL(4)) {
	        debugprintf("progtab: line len=%d\n",len) ;
	        debugprintf("progtab: line=>%t<\n",lbuf,len) ;
	    }
#endif

	    while (*cp && isspace(*cp)) cp += 1 ;

	    if ((*cp == '\0') || (*cp == '#')) continue ;

/* get the logfile name */

	    sp = cp ;
	    while (*cp && (! isspace(*cp)))
	        cp += 1 ;

	    mkpath1w(logfname,sp,(cp-sp)) ;

	    while (*cp && isspace(*cp)) cp += 1 ;

/* get the size specification */

	    sizespec = cp ;
	    while (*cp && (! isspace(*cp))) cp += 1 ;

	    logsize = pip->deflogsize ;
	    if (sizespec[0] != '\0') {
		int	uv ;
	        if (cfdecmfui(sizespec,(cp-sizespec),&uv) >= 0)
	            logsize = uv ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("progtab: logfname=%s\n",logfname) ;
	        debugprintf("progtab: logize=%u\n",logsize) ;
	    }
#endif

	    rs = proglogent(pip,logfname,logsize) ;

	    if (rs < 0) break ;
	} /* end while (reading lines) */

	bclose(ltp) ;
	} /* end if (file-open) */

	if (rs >= 0) {
	    pip->sall.total += pip->stab.total ;
	    pip->sall.checked += pip->stab.checked ;
	    pip->sall.oversized += pip->stab.oversized ;
	    pip->c_processed += 1 ;
	} /* end if */

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: total=%u checked=%u oversized=%u\n",
	        pip->progname,
	        pip->stab.total,pip->stab.checked,pip->stab.oversized) ;
	}

	if (pip->open.logprog) {
	    logfile_printf(&pip->lh,"files=%u checked=%u oversized=%u\n",
	        pip->stab.total,pip->stab.checked,pip->stab.oversized) ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progtab: ret rs=%d checked=%u\n",
		rs,pip->stab.checked) ;
#endif

	return (rs >= 0) ? pip->stab.checked : rs ;
}
/* end subroutine (progtab) */


