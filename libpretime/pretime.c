/* pretime */

/* set or get some program (process) data */
/* last modified %G% version %I% */


#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 2004-11-22, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Manipulated system time management.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/time.h>		/* for |gettimeofday(3c)| */
#include	<sys/timeb.h>		/* for |ftime(3c)| */
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<tmz.h>
#include	<localmisc.h>


/* local defines */

#define	PRETIME		struct pretime_head

#define	VARBASETIME	"LIBPRETIME_BASETIME"

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	NDF		"libpretime.nd"


/* typedefs */

typedef int	(*gettimeofday_t)(struct timeval *,void *) ;


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	msleep(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct pretime_head {
	long		off ;
	time_t		(*func_time)(time_t *) ;
	int		(*func_gettimeofday)(struct timeval *,void *) ;
	int		(*func_ftime)(struct timeb *) ;
	volatile int	f_init ;
	volatile int	f_initdone ;
	int		serial ;
} ;


/* forward references */

static int pretime_loadsyms(PRETIME *) ;


/* local variables */

static PRETIME		pretime_data ; /* zero-initialized */

static cchar	*syms[] = {
	"time",
	"gettimeofday",
	"ftime",
	NULL
} ;

enum syms {
	sym_time,
	sym_gettimeofday,
	sym_ftime,
	sym_overlast
} ;


/* exported subroutines */


int pretime_init()
{
	PRETIME		*op = &pretime_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! op->f_init) {
	    op->f_init = TRUE ;
	    if (op->func_time == NULL) {
	        if ((rs = pretime_loadsyms(op)) >= 0) {
	            cchar	*cp ;
	            if ((cp = getenv(VARBASETIME)) != NULL) {
	                TMZ		z ;
	                if ((rs = tmz_toucht(&z,cp,-1)) >= 0) {
		            struct tm	ts ;
		            memset(&ts,0,sizeof(struct tm)) ;
		            if ((rs = tmz_gettm(&z,&ts)) >= 0) {
		                time_t	ti_base ;
		                if ((rs = uc_mktime(&ts,&ti_base)) >= 0) {
			            time_t ti_now = (*op->func_time)(NULL) ;
			            op->off = (ti_base-ti_now) ;
			            f = TRUE ;
		                }
		            }
	                }
	            } /* end if (base-time) */
		    if (rs >= 0) {
	                op->f_initdone = TRUE ;
		    } else {
	                op->f_init = FALSE ;
		    }
	        } /* end if (load-syms) */
#if	CF_DEBUGN
	    {
	        cchar	*ef = getexecname() ;
	        nprintf(NDF,"pretime_init: ef=%s\n",ef) ;
	    }
#endif
	    } /* end if (needed initialization) */
	} else {
	    while ((rs >= 0) && (! op->f_initdone)) {
		rs = msleep(1) ;
		if (rs == SR_INTR) rs = SR_OK ;
	    }
	}

#if	CF_DEBUGN
	nprintf(NDF,"pretime_init: ret rs=%d off=%ld\n",rs,op->off) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (pretime_init) */


void pretime_fini()
{
	PRETIME		*op = &pretime_data ;
	if (op->f_initdone) {
	    op->f_initdone = FALSE ;
	    op->f_init = FALSE ;
	}
}
/* end subroutine (pretime_fini) */


int pretime_serial()
{
	PRETIME		*op = &pretime_data ;
	int		rs ;
	if ((rs = pretime_init()) >= 0) {
	   rs = op->serial++ ;
#if	CF_DEBUGN
	    {
	        cchar	*ef = getexecname() ;
	        nprintf(NDF,"pretime_serial: sn=%u ef=%s\n",rs,ef) ;
	    }
#endif
	}
	return rs ;
}
/* end subroutine (pretime_serial) */


int pretime_getoff(long *offp)
{
	PRETIME		*op = &pretime_data ;
	int		rs ;
	if (offp == NULL) return SR_FAULT ;
	if ((rs = pretime_init()) >= 0) {
	    if (offp != NULL) *offp = op->off ;
	}
	return rs ;
}
/* end subroutine (pretime_getoff) */


int pretime_modtime(time_t *timep)
{
	PRETIME		*op = &pretime_data ;
	int		rs ;
	if (timep == NULL) return SR_FAULT ;
	if ((rs = pretime_init()) >= 0) {
	    time_t	t = (*op->func_time)(NULL) ;
#if	CF_DEBUGN
	    {
		char	tbuf[TIMEBUFLEN+1] ;
		timestr_logz(t,tbuf) ;
		nprintf(NDF,"libpretime/pretime_modtime: time=%s\n",tbuf) ;
	    }
#endif
	    t += op->off ;
	    *timep = t ;
	} /* end if (init) */
	return rs ;
}
/* end subroutine (pretime_modtime) */


int pretime_modtv(struct timeval *tvp,void *dummy)
{
	PRETIME		*op = &pretime_data ;
	int		rs ;
	if (tvp == NULL) return SR_FAULT ;
	tvp->tv_sec = 0 ;
	tvp->tv_usec = 0 ;
	if ((rs = pretime_init()) >= 0) {
	    if ((*op->func_gettimeofday)(tvp,dummy) >= 0) {
	        tvp->tv_sec += op->off ;
	    } else {
		rs = (-errno) ;
	    }
	}
	return rs ;
}
/* end subroutine (pretime_modtv) */


int pretime_modts(struct timespec *tsp)
{
	PRETIME		*op = &pretime_data ;
	int		rs ;
	if (tsp == NULL) return SR_FAULT ;
	if ((rs = pretime_init()) >= 0) {
	    struct timeval	tv ;
	    if ((*op->func_gettimeofday)(&tv,NULL) >= 0) {
		tsp->tv_sec = tv.tv_sec ;
		tsp->tv_nsec = (tv.tv_usec * 1000) ;
	        tsp->tv_sec += op->off ;
	    } else {
		rs = (-errno) ;
	    }
	}
	return rs ;
}
/* end subroutine (pretime_modts) */


int pretime_modtimeb(struct timeb *tbp)
{
	PRETIME		*op = &pretime_data ;
	int		rs ;
	if (tbp == NULL) return SR_FAULT ;
	memset(tbp,0,sizeof(struct timeb)) ;
	if ((rs = pretime_init()) >= 0) {
	    if ((*op->func_ftime)(tbp) >= 0) {
	        tbp->time += op->off ;
	    } else {
		rs = (-errno) ;
	    }
	}
	return rs ;
}
/* end subroutine (pretime_modtimeb) */


/* private subroutines */


static int pretime_loadsyms(PRETIME *op)
{
	int		rs = SR_OK ;
	int		i ;
	void		*sp ;
	for (i = 0 ; (rs >= 0) && (syms[i] != NULL) ; i += 1) {
	    if ((sp = dlsym(RTLD_NEXT,syms[i])) != NULL) {
	        switch (i) {
	        case sym_time:
	            op->func_time = (time_t (*)(time_t *)) sp ;
		    break ;
	        case sym_gettimeofday:
	            op->func_gettimeofday = (gettimeofday_t) sp ;
		    break ;
	        case sym_ftime:
	            op->func_ftime = (int (*)(struct timeb *)) sp ;
		    break ;
	        } /* end switch */
	    } else {
		rs = SR_LIBACC ;
	    }
	} /* end for */
	return rs ;
}
/* end subroutine (pretime_loadsyms) */


