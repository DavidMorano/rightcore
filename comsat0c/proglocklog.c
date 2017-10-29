/* progloglock */

/* process a locked-log-file note */


#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The subroutine was written from scratch.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutine process log-file messages, but we have a lock around
        them because we can be called from multiple threads.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfsub(const char *,int,const char *,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	bufprintf(char *,int,const char *,...) ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* global variables */


/* local variables */


/* exported subroutines */


int progloglock_begin(PROGINFO *pip)
{
	PROGINFO_LOG	*ldp = &pip->logdata ;
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    ldp->ti_logsize = pip->daytime ;
	    ldp->ti_logcheck = pip->daytime ;
	    ldp->ti_logflush = pip->daytime ;

	    ldp->intlogsize = DEFINTLOGSIZE ;
	    ldp->intlogcheck = DEFINTLOGCHECK ;
	    ldp->intlogflush = DEFINTLOGFLUSH ;

	    if (! pip->open.lm) {
	        rs = ptm_create(&ldp->lm,NULL) ;
	        pip->open.lm = (rs >= 0) ;
	    }
	}

	return rs ;
}
/* end subroutine (progloglock_begin) */


int progloglock_end(PROGINFO *pip)
{
	PROGINFO_LOG	*ldp = &pip->logdata ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->open.logprog) {
	    if (pip->open.lm) {
	        pip->open.lm = FALSE ;
	        rs1 = ptm_destroy(&ldp->lm) ;
		if (rs >= 0) rs = rs1 ;
	    }
	}

	return rs ;
}
/* end subroutine (progloglock_end) */


int progloglock_printf(PROGINFO *pip,const char *fmt,...)
{
	PROGINFO_LOG	*ldp = &pip->logdata ;
	int		rs = SR_OK ;

	if (pip->open.logprog && pip->open.lm) {
	    if ((rs = ptm_lock(&ldp->lm)) >= 0) {
		{
		    va_list	ap ;
	            va_begin(ap,fmt) ;
	            rs = logfile_vprintf(&pip->lh,fmt,ap) ;
	            va_end(ap) ;
		}
	        ptm_unlock(&ldp->lm) ;
	    } /* end if (log-lock) */
	} /* end block */

	return rs ;
}
/* end subroutine (progloglockprintf) */


int progloglock_maint(PROGINFO *pip)
{
	PROGINFO_LOG	*ldp = &pip->logdata ;
	int		rs = SR_OK ;

	if (pip->open.logprog && pip->open.lm) {
	    if ((rs = ptm_lock(&ldp->lm)) >= 0) {
		const time_t	dt = pip->daytime ;

	        if ((dt - ldp->ti_logsize) >= ldp->intlogsize) {
	            ldp->ti_logsize = pip->daytime ;
	            logfile_checksize(&pip->lh,pip->logsize) ;
		} else if ((dt - ldp->ti_logcheck) >= ldp->intlogcheck) {
	            ldp->ti_logcheck = pip->daytime ;
	            logfile_check(&pip->lh,pip->daytime) ;
	        } else if ((dt - ldp->ti_logflush) >= ldp->intlogflush) {
	            ldp->ti_logflush = pip->daytime ;
	            logfile_flush(&pip->lh) ;
	        }

	        ptm_unlock(&ldp->lm) ;
	    } /* end if (log-lock) */
	} /* end if (log open) */

	return rs ;
}
/* end subroutine (progloglock_maint) */


