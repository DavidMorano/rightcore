/* kinfo */

/* kernel information access (for Sun Solaris) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		1		/* "safe" mode? */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	Module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object provides access to some kernel-related information.

	Extra notes for the observant:

	You might note that the KSTAT(3kstat) stuff is NOT reentrant nor
	thread safe.  Yes, its true, they are not!  Blame Sun Microsystems
	for that!


*******************************************************************************/

#define	KINFO_MASTER	1

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<kstat.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#if	(defined(SYSHAS_LOADAVG) && (SYSHAS_LOADAVG > 0))
#include	<sys/loadavg.h>
#endif

#include	<vsystem.h>
#include	<localmisc.h>

#include	"kinfo.h"


/* local defines */

#define	KINFO_DEFENTS	10
#define	KINFO_MAGIC		0x98743251

#define	TO_KSYSMISC	4		/* time between system_misc gets */
#define	TO_KLOADAVE	1		/* time between LOADAVE gets */
#define	TO_KUPDATE	(5 * 60)	/* time before a chain update */
#define	TO_KMAXIDLE	(10 * 60)	/* maximum idle time allowed */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */


/* forward referecens */

int		kinfo_sysmisc(KINFO *,time_t,KINFO_DATA *) ;

static int	kinfo_kopen(KINFO *,time_t) ;
static int	kinfo_kclose(KINFO *) ;
static int	kinfo_kupdate(KINFO *,time_t) ;
static int	kinfo_ksysmisc(KINFO *,time_t) ;
static int	kinfo_kloadave(KINFO *,time_t) ;


/* local variables */

static const char	*avenruns[] = {
	"avenrun_1min",
	"avenrun_5min",
	"avenrun_15min",
	NULL
} ;


/* exported subroutines */


int kinfo_open(sp,ti_daytime)
KINFO		*sp ;
time_t		ti_daytime ;
{
	int	rs = SR_OK ;


#if	CF_SAFE
	if (sp == NULL)
	    return SR_FAULT ;
#endif

	memset(sp,0,sizeof(KINFO)) ;

	sp->tosysmisc = TO_KSYSMISC ;
	sp->toloadave = TO_KLOADAVE ;

	sp->magic = KINFO_MAGIC ;
	return rs ;
}
/* end subroutine (kinfo_open) */


int kinfo_boottime(sp,ti_daytime,kdp)
KINFO		*sp ;
time_t		ti_daytime ;
KINFO_DATA	*kdp ;
{
	kstat_t	*ksp = NULL ;

	int	rs = SR_OK ;

#if	CF_DEBUGS
	char	timebuf[TIMEBUFLEN + 1] ;
#endif


#if	CF_SAFE
	if (sp == NULL)
	    return SR_FAULT ;

	if (sp->magic != KINFO_MAGIC)
	    return SR_NOTOPEN ;

	if (kdp == NULL)
	    return SR_FAULT ;
#endif

#if	CF_DEBUGS
	debugprintf("kinfo_boottime: ent daytime=%s\n",
	    timestr_log(ti_daytime,timebuf)) ;
#endif

	if (! sp->f.kopen)
	    rs = kinfo_kopen(sp,ti_daytime) ;

/* too tired, just do this! */

	if (rs >= 0)
	    rs = kinfo_sysmisc(sp,ti_daytime,kdp) ;

	return rs ;
}
/* end subroutine (kinfo_boottime) */


int kinfo_loadave(sp,ti_daytime,kdp)
KINFO		*sp ;
time_t		ti_daytime ;
KINFO_DATA	*kdp ;
{
	kstat_t	*ksp ;

	kid_t	kid ;

	int	rs = SR_OK ;
	int	i ;

#if	CF_DEBUGS
	char	timebuf[TIMEBUFLEN + 1] ;
#endif

#if	CF_SAFE
	if (sp == NULL)
	    return SR_FAULT ;

	if (sp->magic != KINFO_MAGIC)
	    return SR_NOTOPEN ;

	if (kdp == NULL)
	    return SR_FAULT ;
#endif

#if	CF_DEBUGS
	debugprintf("kinfo_loadave: ent daytime=%s\n",
	    timestr_log(ti_daytime,timebuf)) ;
#endif

	sp->ti_access = ti_daytime ;
	if ((ti_daytime - sp->ti_loadave) >= sp->toloadave) {

	    rs = kinfo_kloadave(sp,ti_daytime) ;

#if	CF_DEBUGS
	    debugprintf("kinfo_loadave: continuing rs=%d\n",rs) ;
#endif

	    if ((rs == SR_NOSYS) || (rs == SR_NOENT)) {

	        rs = SR_OK ;
	        if (! sp->f.kopen) {
	            rs = kinfo_kopen(sp,ti_daytime) ;
		} else
	            rs = kinfo_kupdate(sp,ti_daytime) ;

	        if (rs >= 0)
	            rs = kinfo_ksysmisc(sp,ti_daytime) ;

	    } /* end if (alternative loadave information) */

	} /* end if (loadave information) */

	if ((rs >= 0) && (kdp != NULL))
	    *kdp = sp->d ;

ret0:
	return rs ;
}
/* end subroutine (kinfo_loadave) */


int kinfo_sysmisc(sp,ti_daytime,kdp)
KINFO		*sp ;
time_t		ti_daytime ;
KINFO_DATA	*kdp ;
{
	kstat_t	*ksp ;

	kid_t	kid ;

	int	rs = SR_OK ;
	int	i ;
	int	f ;

#if	CF_DEBUGS
	char	timebuf[TIMEBUFLEN + 1] ;
#endif


#if	CF_SAFE
	if (sp == NULL)
	    return SR_FAULT ;

	if (sp->magic != KINFO_MAGIC)
	    return SR_NOTOPEN ;

	if (kdp == NULL)
	    return SR_FAULT ;
#endif

#if	CF_DEBUGS
	debugprintf("kinfo_sysmisc: ent daytime=%s\n",
	    timestr_log(ti_daytime,timebuf)) ;
#endif

	rs = SR_TIMEDOUT ;
	sp->ti_access = ti_daytime ;
	f = ((ti_daytime - sp->ti_loadave) >= sp->toloadave) &&
	    ((ti_daytime - sp->ti_sysmisc) < sp->tosysmisc) ;

#if	CF_DEBUGS
	debugprintf("kinfo_sysmisc: LOADAVE timeout=%u\n",f) ;
#endif

	if (f)
	    rs = kinfo_kloadave(sp,ti_daytime) ;

#if	CF_DEBUGS
	debugprintf("kinfo_sysmisc: continuing rs=%d\n",rs) ;
#endif

	if ((rs == SR_NOSYS) || (rs == SR_NOENT) ||
	    ((ti_daytime - sp->ti_sysmisc) >= sp->tosysmisc)) {

#if	CF_DEBUGS
	    debugprintf("kinfo_sysmisc: continuing\n") ;
#endif

	    rs = SR_OK ;
	    if (! sp->f.kopen) {
	        rs = kinfo_kopen(sp,ti_daytime) ;
	    } else
	        rs = kinfo_kupdate(sp,ti_daytime) ;

	    if (rs >= 0)
	        rs = kinfo_ksysmisc(sp,ti_daytime) ;

	} /* end if (extended information) */

	if ((rs >= 0) && (kdp != NULL))
	    *kdp = sp->d ;

ret0:
	return rs ;
}
/* end subroutine (kinfo_sysmisc) */


int kinfo_check(sp,ti_daytime)
KINFO		*sp ;
time_t		ti_daytime ;
{
	int	rs = SR_OK ;

#if	CF_DEBUGS
	char	timebuf[TIMEBUFLEN + 1] ;
#endif


#if	CF_SAFE
	if (sp == NULL)
	    return SR_FAULT ;

	if (sp->magic != KINFO_MAGIC)
	    return SR_NOTOPEN ;
#endif

#if	CF_DEBUGS
	debugprintf("kinfo_check: ent daytime=%s\n",
	    timestr_log(ti_daytime,timebuf)) ;
#endif

/* check on the weirdo 'K' stuff */

	if (sp->f.kopen) {

	    if ((ti_daytime - sp->ti_access) >= TO_KMAXIDLE) {

#if	CF_DEBUGS
	        debugprintf("kinfo_check: closing\n") ;
#endif

	        kinfo_kclose(sp) ;

	    } /* end if */

	    if (sp->f.kopen)
	        rs = kinfo_kupdate(sp,ti_daytime) ;

	} /* end if (KOPEN) */

/* we're out of here */

	return rs ;
}
/* end subroutine (kinfo_check) */


int kinfo_close(sp)
KINFO		*sp ;
{
	int	rs ;


#if	CF_SAFE
	if (sp == NULL)
	    return SR_FAULT ;

	if (sp->magic != KINFO_MAGIC)
	    return SR_NOTOPEN ;
#endif

#if	CF_DEBUGS
	debugprintf("kinfo_close: ent\n") ;
#endif

	rs = kinfo_kclose(sp) ;

	sp->magic = 0 ;
	return rs ;
}
/* end subroutine (kinfo_close) */


/* private subroutines */


static int kinfo_kopen(sp,ti_daytime)
KINFO		*sp ;
time_t		ti_daytime ;
{
	int	rs = SR_OK ;


	if (! sp->f.kopen) {

	    sp->kcp = kstat_open() ;

	    if (sp->kcp != NULL) {

	        sp->ti_access = ti_daytime ;
	        sp->ti_update = ti_daytime ;
	        sp->f.kopen = TRUE ;
	        sp->nactive += 1 ;

	    } else
	        rs = SR_NOANODE ;

	} /* end if (was not open) */

	return rs ;
}
/* end subroutine (kinfo_kopen) */


static int kinfo_kclose(sp)
KINFO		*sp ;
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (sp->f.kopen) {
	    sp->f.kopen = FALSE ;
	    if (sp->kcp != NULL) {
	        if ((rs1 = kstat_close(sp->kcp)) < 0) rs1 = SR_NXIO ;
		if (rs >= 0) rs = rs1 ;
	        sp->kcp = NULL ;
	    }
	    if (sp->nactive > 0) sp->nactive -= 1 ;
	} /* end if (was open) */

	return rs ;
}
/* end subroutine (kinfo_kclose) */


static int kinfo_kupdate(sp,ti_daytime)
KINFO		*sp ;
time_t		ti_daytime ;
{
	int	rs = SR_OK ;

	if (sp->f.kopen) {
	    if ((ti_daytime - sp->ti_update) >= TO_KUPDATE) {

	        sp->ti_update = ti_daytime ;
	        (void) kstat_chain_update(sp->kcp) ;

	    } /* end if */
	} /* end if (kopen) */

	return rs ;
}
/* end subroutine (kinfo_kupdate) */


static int kinfo_ksysmisc(sp,ti_daytime)
KINFO		*sp ;
time_t		ti_daytime ;
{
	kstat_t	*ksp ;

	kid_t	kid ;

	int	rs = SR_OK ;
	int	i ;
	int	f ;


	ksp = kstat_lookup(sp->kcp,"unix",0,"system_misc") ;

#if	CF_DEBUGS
	debugprintf("kinfo_ksysmisc: kstat_lookup() rs=%d\n",
	    ((ksp != NULL) ? 0 : SR_NOTSUP)) ;
#endif

	if ((rs >= 0) && (ksp != NULL)) {

	    if ((kid = kstat_read(sp->kcp, ksp, NULL)) >= 0) {
	        kstat_named_t *ksn ;

	        sp->ti_sysmisc = ti_daytime ;
	        sp->ti_loadave = ti_daytime ;
	        ksn = (kstat_named_t *)
	            kstat_data_lookup(ksp,"ncpus") ;

	        if (ksn != NULL)
	            sp->d.ncpu = ksn->value.ui32 ;

	        ksn = (kstat_named_t *)
	            kstat_data_lookup(ksp,"nproc") ;

	        if (ksn != NULL)
	            sp->d.nproc = ksn->value.ui32 ;

	        for (i = 0 ; i < 3 ; i += 1) {

	            ksn = (kstat_named_t *)
	                kstat_data_lookup(ksp,(char *) avenruns[i]) ;

	            if (ksn != NULL)
	                sp->d.la[i] = ksn->value.ui32 ;

	        } /* end for */

	        if (sp->d.boottime == 0) {

	            ksn = (kstat_named_t *)
	                kstat_data_lookup(ksp,"boot_time") ;

	            if (ksn != NULL)
	                sp->d.boottime = ksn->value.ui32 ;

	        } /* end if */

#if	CF_DEBUGS
	        {
	            for (i = 0 ; i < 3 ; i += 1) {
	                double	fla = sp->d.la[i] ;
	                fla /= FSCALE ;
	                debugprintf("kinfo_loadave: expensive la%u=%7.3f\n",
	                    i,fla) ;
	            }
	        }
#endif /* CF_DEBUGS */

	        rs = SR_OK ;

	    } /* end if (reading data) */

	} else
	    rs = SR_NOTFOUND ;

	return rs ;
}
/* end subroutine (kinfo_ksysmisc) */


static int kinfo_kloadave(sp,ti_daytime)
KINFO		*sp ;
time_t		ti_daytime ;
{
	int	rs = SR_OK ;
	int	i ;


#if	(defined(SYSHAS_LOADAVG) && (SYSHAS_LOADAVG > 0))
	{
	    uint	las[3] ;


	    if ((rs = u_getloadavg(las,3)) >= 0) {

	        sp->ti_loadave = ti_daytime ;
	        for (i = 0 ; i < 3 ; i += 1)
	            sp->d.la[i] = las[i] ;

#if	CF_DEBUGS
	        {
	            for (i = 0 ; i < 3 ; i += 1) {
	                double	fla = sp->d.la[i] ;
	                fla /= FSCALE ;
	                debugprintf("kinfo_loadave: cheapo la%u=%7.3f\n",
	                    i,fla) ;
	            }
	        }
#endif /* CF_DEBUGS */

	    } /* end if (had |loadavg(3c)|) */

	}
#else /* SYSHAS_LOADAVG */

	rs = SR_NOSYS ;

#endif /* SYSHAS_LOADAVG */

	return rs ;
}
/* end subroutine (kinfo_kloadave) */



