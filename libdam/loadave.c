/* loadave */

/* loadave gathering and manipulation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_GETLOADAVG	1		/* use 'u_getloadavg(2)' */


/* revision history:

	= 1999-12-01, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object module provides a convenient way to get the load average
        numbers our of the kernel. This object only works for those OSes that
        support the KSTAT system kernel interface API. OSes that supprt the
        KSTAT interface are usually newer SVR4 systems.

        This object is *not* supposed to be multi-thread-safe because according
        to the last that I knew, the KSTAT system API was not 
	multi-thread-safe!


*******************************************************************************/


#define	LOADAVE_MASTER		1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/systeminfo.h>
#include	<unistd.h>
#include	<limits.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<kstat.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<localmisc.h>

#include	"loadave.h"


/* local defines */

#define	LOADAVE_MAGIC	1686593468
#define	LOADAVE_BUFLEN	100

#define	TO_KMAXIDLE	(5 * 60)	/* maximum idle time allowed */
#define	TO_KUPDATE	(15 * 60)	/* time before a chain update */
#define	TO_KMAXOPEN	(60 * 60)	/* maximum open time */
#define	TO_KSYSMISC	5		/* time between system_misc gets */
#define	TO_LOADAVE	1		/* time between LOADAVE gets */

#define	LOADAVE_INTUPDATE	(1 * 3600)
#define	LOADAVE_INTMAXOPEN	(4 * 3600)


#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	loadave_kopen(LOADAVE *,time_t) ;
static int	loadave_kclose(LOADAVE *) ;
static int	loadave_getid(LOADAVE *,time_t) ;
static int	loadave_getdata(LOADAVE *,LOADAVE_VALUES *) ;

static void	popuint() ;
static void	poptime() ;


/* local variables */

static const char	*miscnames[] = {
	"ncpus",
	"nproc",
	"avenrun_1min",
	"avenrun_5min",
	"avenrun_15min",
	NULL
} ;

enum miscnames {
	miscname_ncpus,
	miscname_nproc,
	miscname_la1min,
	miscname_la5min,
	miscname_la15min,
	miscname_overlast
} ;


/* exported subroutines */


int loadave_start(eop)
LOADAVE		*eop ;
{
	kstat_named_t *ksn ;
	time_t		daytime = time(NULL) ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("loadave_start: ent\n") ;
#endif

	if (eop == NULL) return SR_FAULT ;

	memset(eop,0,sizeof(LOADAVE)) ;

	rs = loadave_kopen(eop,daytime) ;
	if (rs < 0) goto bad0 ;

/* see if we can actually get any data ! */

	eop->v.tim_read = daytime ;
	rs = loadave_getdata(eop,&eop->v) ;
	if (rs < 0) goto bad1 ;

#if	CF_DEBUGS
	debugprintf("loadave_start: loadave_getdata() rs=%d\n",rs) ;
#endif

/* get the machine boot-up time */

	ksn = (kstat_named_t *)
	    kstat_data_lookup(eop->ksp,"boot_time") ;

#if	CF_DEBUGS
	debugprintf("loadave_start: lookup boot_time %s\n",
	    ((ksn == NULL) ? "no" : "yes")) ;
#endif

	poptime(ksn,&eop->v.tim_boot) ;

#if	CF_DEBUGS
	debugprintf("loadave_start: popped time \n") ;
#endif


/* load the time this data was taken */

	eop->tim_access = daytime ;

	eop->magic = LOADAVE_MAGIC ;

ret0:
	return rs ;

/* bad things */
bad1:
	if (eop->kcp != NULL) {
	    kstat_close(eop->kcp) ;
	    eop->kcp = NULL ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (loadave_start) */


/* free up the entire vector string data structure object */
int loadave_finish(eop)
LOADAVE		*eop ;
{

#if	CF_DEBUGS
	debugprintf("loadave_finish: ent\n") ;
#endif

	if (eop == NULL) return SR_FAULT ;

	if (eop->magic != LOADAVE_MAGIC) return SR_NOTOPEN ;

	if (eop->kcp != NULL) {
	    kstat_close(eop->kcp) ;
	    eop->kcp = NULL ;
	}

	return SR_OK ;
}
/* end subroutine (loadave_finish) */


/* read some load averages */
int loadave_readvalues(eop,vp)
LOADAVE		*eop ;
LOADAVE_VALUES	*vp ;
{
	time_t		daytime = time(NULL) ;
	int		rs = SR_OK ;

	if (eop == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	if (eop->magic != LOADAVE_MAGIC) return SR_NOTOPEN ;

/* have we reached the timeout for the chain update? */

	if (eop->f.open &&
	    ((daytime - eop->tim_update) >= TO_KUPDATE)) {

	    eop->tim_update = daytime ;
	    eop->ksp = NULL ;
	    (void) kstat_chain_update(eop->kcp) ;

	} /* end if (chain update was needed) */

	if ((daytime - eop->v.tim_read) >= TO_KSYSMISC) {

	    if (! eop->f.open)
	        rs = loadave_kopen(eop,daytime) ;

	    if (rs >= 0) {
	        eop->v.tim_read = daytime ;
	        rs = loadave_getdata(eop,&eop->v) ;
	    }

	} /* end if */

#if	CF_GETLOADAVG
	if ((eop->v.tim_read != daytime) &&
		((daytime - eop->tim_access) >= TO_LOADAVE)) {
		uint	la[3] ;

		rs = u_getloadavg(la,3) ;

		eop->v.la1min = la[0] ;
		eop->v.la5min = la[1] ;
		eop->v.la15min = la[2] ;

	}
#endif /* CF_GETLOADAVG */

/* satisfy this current request */

	eop->tim_access = daytime ;
	memcpy(vp,&eop->v,sizeof(LOADAVE_VALUES)) ;

	return rs ;
}
/* end subroutine (loadave_readvalues) */


/* read the machine ID information */
int loadave_readmid(eop,vp)
LOADAVE		*eop ;
LOADAVE_MID	*vp ;
{
	time_t		daytime = time(NULL) ;
	int		rs = SR_OK ;

	if (eop == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	if (eop->magic != LOADAVE_MAGIC) return SR_NOTOPEN ;

	if ((daytime - eop->mid.tim_read) >= TO_KUPDATE)
	    rs = loadave_getid(eop,daytime) ;

	memcpy(vp,&eop->mid,sizeof(LOADAVE_MID)) ;

	return rs ;
}
/* end subroutine (loadave_readmid) */


/* check up on the object */
int loadave_check(eop,daytime)
LOADAVE		*eop ;
time_t		daytime ;
{

	int		rs = SR_OK ;

	if (eop == NULL) return SR_FAULT ;

	if (eop->magic != LOADAVE_MAGIC) return SR_NOTOPEN ;

	if (daytime == 0)
	    daytime = time(NULL) ;

/* check on the weirdo 'K' stuff */

	if (eop->f.open &&
	    ((daytime - eop->tim_access) >= TO_KMAXIDLE))
	    loadave_kclose(eop) ;

	if (eop->f.open &&
	    ((daytime - eop->tim_open) >= TO_KMAXOPEN))
	    loadave_kclose(eop) ;

	return rs ;
}
/* end subroutine (loadave_check) */


/* private subroutines */


static int loadave_kopen(eop,daytime)
LOADAVE		*eop ;
time_t		daytime ;
{

	eop->kcp = kstat_open() ;

	if (eop->kcp == NULL)
	    return SR_NOTSUP ;

	eop->tim_open = daytime ;
	eop->tim_update = daytime ;
	eop->f.open = TRUE ;

	return SR_OK ;
}
/* end subroutine (loadave_kopen) */


static int loadave_kclose(eop)
LOADAVE		*eop ;
{

	if (eop->kcp != NULL) {
	    kstat_close(eop->kcp) ;
	    eop->kcp = NULL ;
	}

	eop->f.open = FALSE ;
	return SR_OK ;
}
/* end subroutine (loadave_kclose) */


static int loadave_getid(eop,daytime)
LOADAVE		*eop ;
time_t		daytime ;
{
	LOADAVE_MID	*mp ;
	int		rs ;
	char		buf[LOADAVE_BUFLEN + 1] ;

	mp = &eop->mid ;

/* get the easy stuff that does not change */

	rs = u_sysinfo(SI_HW_PROVIDER,buf,LOADAVE_BUFLEN) ;
	if (rs < 0)
	    goto bad0 ;

	strwcpy(mp->provider,buf,LOADAVE_IDLEN) ;

	rs = u_sysinfo(SI_HW_SERIAL,buf,LOADAVE_BUFLEN) ;
	if (rs < 0)
	    goto bad1 ;

	strwcpy(mp->serial,buf,LOADAVE_IDLEN) ;

	mp->tim_read = daytime ;
	mp->hostid = (uint) gethostid() ;

ret0:
	return 0 ;

/* bad stuff */
bad1:
bad0:
	goto ret0 ;
}
/* end subroutine (loadave_getid) */


/* read the data out of the kernel */
static int loadave_getdata(eop,vp)
LOADAVE		*eop ;
LOADAVE_VALUES	*vp ;
{
	kstat_t		*ksp ;
	kstat_named_t	*ksn ;
	kid_t		kid ;
	int		rs = SR_OK ;
	int		i, j ;
	uchar		haveval[miscname_overlast] ;

	if ((ksp = eop->ksp) == NULL) {

#if	CF_DEBUGS
	    debugprintf("loadave_getdata: lookup 'system_misc' !\n") ;
#endif

	    ksp = kstat_lookup(eop->kcp, "unix", 0, "system_misc") ;

	    if (ksp == NULL) {
	        rs = SR_NOTSUP ;
	        goto bad0 ;
	    }

	    eop->ksp = ksp ;

	} /* end if (chached KSTAT pointer) */

#if	CF_DEBUGS
	debugprintf("loadave_getdata: got a 'system_misc'\n") ;
#endif

	kid = kstat_read(eop->kcp, ksp, NULL) ;

	if (kid < 0) {
	    rs = SR_IO ;
	    goto bad0 ;
	}

	memset(haveval,0,miscname_overlast) ;

#if	CF_DEBUGS
	debugprintf("loadave_getdata: read some data, kid=%d\n",
	    kid) ;
#endif

	ksn = (kstat_named_t *) ksp->ks_data ;
	for (i = 0 ; i < ksp->ks_ndata ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("loadave_getdata: looping i=%d\n",i) ;
#endif

/* see if the name is one that we want */

	    for (j = 0 ; miscnames[j] != NULL ; j += 1) {

	        if ((! haveval[j]) &&
	            (strncmp(ksn->name,miscnames[j],KSTAT_STRLEN) == 0))
	            break ;

	    } /* end for */

	    if (miscnames[j] != NULL) {

#if	CF_DEBUGS
	        debugprintf("loadave_getdata: found one of ours j=%d\n",j) ;
#endif

	        haveval[j] = 1 ;

	        switch (j) {

	        case miscname_ncpus:
	            popuint(ksn,&vp->ncpu) ;
	            break ;

	        case miscname_nproc:
	            popuint(ksn,&vp->nproc) ;
	            break ;

	        case miscname_la1min:
	            popuint(ksn,&vp->la1min) ;
	            break ;

	        case miscname_la5min:
	            popuint(ksn,&vp->la5min) ;
	            break ;

	        case miscname_la15min:
	            popuint(ksn,&vp->la15min) ;
	            break ;

	        } /* end switch */

	    } /* end if */

	    ksn += 1 ;

	} /* end for */

#if	CF_DEBUGS
	debugprintf("loadave_getdata: exiting rs=%d\n",rs) ;
#endif

bad0:
	return rs ;
}
/* end subroutine (loadave_getdata) */


static void popuint(ksn,vp)
kstat_named_t	*ksn ;
uint		*vp ;
{

	switch (ksn->data_type) {

	case KSTAT_DATA_CHAR:
	    *vp = (uint) ksn->value.c[0] ;
	    break ;

	case KSTAT_DATA_INT32:
	    *vp = (uint) ksn->value.i32 ;
	    break ;

	case KSTAT_DATA_UINT32:
	    *vp = (uint) ksn->value.ui32 ;
	    break ;

	case KSTAT_DATA_INT64:
	    *vp = (uint) ksn->value.i64 ;
	    break ;

	case KSTAT_DATA_UINT64:
	    *vp = (uint) ksn->value.ui64 ;
	    break ;

	} /* end switch */

}
/* end subroutine (popuint) */


static void poptime(ksn,vp)
kstat_named_t	*ksn ;
time_t		*vp ;
{

	switch (ksn->data_type) {

	case KSTAT_DATA_CHAR:
	    *vp = (time_t) ksn->value.c[0] ;
	    break ;

	case KSTAT_DATA_INT32:
	    *vp = (time_t) ksn->value.i32 ;
	    break ;

	case KSTAT_DATA_UINT32:
	    *vp = (time_t) ksn->value.ui32 ;
	    break ;

	case KSTAT_DATA_INT64:
	    *vp = (time_t) ksn->value.i64 ;
	    break ;

	case KSTAT_DATA_UINT64:
	    *vp = (time_t) ksn->value.ui64 ;
	    break ;

	} /* end switch */

}
/* end subroutine (poptime) */


