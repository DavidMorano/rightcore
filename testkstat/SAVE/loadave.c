/* loadave */

/* loadave gathering and manipulation */


#define	F_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history :

	= 95/12/01, Dave Morano

	This module was originally written.


*/


/**************************************************************************

	This object module provides a convenient way to get the load
	average numbers our of the kernel.  This object only works for
	those OSes that support the KSTAT system kernel interface
	API.  OSes that supprt the KSTAT interface are usually newer
	SVR4 systems.

	This object is *not* supposed to be multi-thread-safe because
	according to the last that I knew, the KSTAT system API
	was not multi-thread-safe !



**************************************************************************/


#define	LOADAVE_MASTER		1


#include	<sys/types.h>
#include	<sys/systeminfo.h>
#include	<unistd.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<kstat.h>

#include	<vsystem.h>
#include	<bitops.h>

#include	"misc.h"
#include	"loadave.h"



/* local defines */

#define	LOADAVE_MAGIC	1686593468
#define	LOADAVE_BUFLEN	100



/* external subroutines */


/* forward references */

static int	loadave_getdata(LOADAVE *,LOADAVE_VALUES *) ;

static void	popuint() ;
static void	poptime() ;


/* local variables */

static char	*const miscnames[] = {
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







int loadave_init(eop)
LOADAVE	*eop ;
{
	kstat_named_t *ksn ;

	int	rs ;

	char	buf[LOADAVE_BUFLEN + 1] ;


#if	F_DEBUGS
	eprintf("loadave_init: entered\n") ;
#endif

	if (eop == NULL)
	    return SR_FAULT ;

	eop->magic = 0 ;


	eop->kcp = kstat_open() ;

	rs = SR_NOTSUP ;
	if (eop->kcp == NULL)
	    goto bad0 ;

/* see if we can actually get any data ! */

	rs = loadave_getdata(eop,&eop->v) ;

	if (rs < 0)
	    goto bad1 ;

#if	F_DEBUGS
	eprintf("loadave_init: loadave_getdata() rs=%d\n",rs) ;
#endif

/* get the machine boot-up time */

	ksn = (kstat_named_t *)
	    kstat_data_lookup(eop->ksp,"boot_time") ;

#if	F_DEBUGS
	eprintf("loadave_init: lookup boot_time %s\n",
		((ksn == NULL) ? "no" : "yes")) ;
#endif

	poptime(ksn,&eop->v.tim_boot) ;

#if	F_DEBUGS
	eprintf("loadave_init: popped time \n") ;
#endif


/* get the easy stuff that does not change */

	rs = u_sysinfo(SI_HW_PROVIDER,buf,LOADAVE_BUFLEN) ;

	if (rs < 0)
	    goto bad1 ;

	strwcpy(&eop->v.provider,buf,LOADAVE_IDLEN) ;

	rs = u_sysinfo(SI_HW_SERIAL,buf,LOADAVE_BUFLEN) ;

	if (rs < 0)
	    goto bad1 ;

	strwcpy(&eop->v.serial,buf,LOADAVE_IDLEN) ;

	rs = uc_gethostid(&eop->v.hostid) ;

	if (rs < 0)
	    goto bad1 ;

/* load the time this data was taken */

	u_time(&eop->v.tim_read) ;

	eop->tim_open = eop->v.tim_read ;
	eop->tim_update = eop->v.tim_read ;


	eop->magic = LOADAVE_MAGIC ;
	return rs ;

/* bad things */
bad1:
	if (eop->kcp != NULL)
	    kstat_close(eop->kcp) ;

bad0:
	return rs ;
}
/* end subroutine (loadave_init) */


/* free up the entire vector string data structure object */
int loadave_free(eop)
LOADAVE	*eop ;
{
	int	i ;


#if	F_DEBUGS
	eprintf("loadave_free: entered\n") ;
#endif

	if (eop == NULL)
	    return SR_FAULT ;

	if (eop->magic != LOADAVE_MAGIC)
	    return SR_NOTOPEN ;

	if (eop->kcp != NULL)
	    kstat_close(eop->kcp) ;

	return SR_OK ;
}
/* end subroutine (loadave_free) */


/* read some loadave */
int loadave_readvalues(eop,vp)
LOADAVE		*eop ;
LOADAVE_VALUES	*vp ;
{
	time_t	daytime ;

	int	rs ;


	if (eop == NULL)
	    return SR_FAULT ;

	if (vp == NULL)
	    return SR_FAULT ;

	if (eop->magic != LOADAVE_MAGIC)
	    return SR_NOTOPEN ;


/* have we reached the timeout for the chain update ? */

	u_time(&daytime) ;

	if ((daytime - eop->tim_update) > LOADAVE_INTUPDATE) {

		eop->tim_update = daytime ;
		eop->ksp = NULL ;
	        (void) kstat_chain_update(eop->kcp) ;

	} /* end if (chain update was needed) */


/* satisfy this current request */

	memcpy(vp,&eop->v,sizeof(LOADAVE_VALUES)) ;

	rs = loadave_getdata(eop,vp) ;

	if (rs >= 0)
		u_time(&vp->tim_read) ;

	return rs ;
}
/* end subroutine (loadave_readvalues) */


/* check up on the object */
int loadave_check(eop,daytime)
LOADAVE		*eop ;
time_t		daytime ;
{
	int	rs = SR_OK ;


	if (eop == NULL)
	    return SR_FAULT ;

	if (eop->magic != LOADAVE_MAGIC)
	    return SR_NOTOPEN ;

	if (daytime == 0)
		u_time(&daytime) ;



	return rs ;
}
/* end subroutine (loadave_check) */



/* CONSTRUCTORS + DESTRUCTORS */



LOADAVE obj_loadave()
{
	LOADAVE	temp ;


	(void) loadave_init(&temp) ;

	return temp ;
}


LOADAVE *new_loadave()
{
	LOADAVE	*sop = NULL ;


	if (uc_malloc(sizeof(LOADAVE),((void *) &sop)) >= 0)
	    (void) loadave_init(sop) ;

	return sop ;
}


void free_loadave(sop)
LOADAVE	*sop ;
{


	(void) loadave_free(sop) ;

	free(sop) ;

}



/* PRIVATE SUBROUTINES */



/* read the data out of the kernel */
static int loadave_getdata(eop,vp)
LOADAVE		*eop ;
LOADAVE_VALUES	*vp ;
{
	kstat_t		*ksp ;

	kstat_named_t	*ksn ;

	kid_t		kid ;

	int	rs = SR_OK, i, j ;

	uchar	haveval[miscname_overlast] ;


	if ((ksp = eop->ksp) == NULL) {

#if	F_DEBUGS
	eprintf("loadave_getdata: lookup 'system_misc' !\n") ;
#endif

	    ksp = kstat_lookup(eop->kcp, "unix", 0, "system_misc") ;

	    if (ksp == NULL) {

	        rs = SR_NOTSUP ;
	        goto bad0 ;
	    }

		eop->ksp = ksp ;

	} /* end if (chached KSTAT pointer) */

#if	F_DEBUGS
	eprintf("loadave_getdata: got a 'system_misc'\n") ;
#endif

	kid = kstat_read(eop->kcp, ksp, NULL) ;

	if (kid < 0) {

	    rs = SR_IO ;
	    goto bad0 ;
	}


	memset(haveval,0,miscname_overlast) ;

#if	F_DEBUGS
	eprintf("loadave_getdata: read some data, kid=%d\n",
	    kid) ;
#endif

	ksn = (kstat_named_t *) ksp->ks_data ;
	for (i = 0 ; i < ksp->ks_ndata ; i += 1) {

#if	F_DEBUGS
	eprintf("loadave_getdata: looping i=%d\n",i) ;
#endif

/* see if the name is one that we want */

	    for (j = 0 ; miscnames[j] != NULL ; j += 1) {

	        if ((! haveval[j]) &&
	            (strncmp(ksn->name,miscnames[j],KSTAT_STRLEN) == 0))
	            break ;

	    } /* end for */

	    if (miscnames[j] != NULL) {

#if	F_DEBUGS
	eprintf("loadave_getdata: found one of ours j=%d\n",j) ;
#endif

	        haveval[j] = 1 ;

	        switch (j) {

	        case miscname_ncpus:
	            popuint(ksn,&vp->ncpus) ;

	            break ;

	        case miscname_nproc:
	            popuint(ksn,&vp->nprocs) ;

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

#if	F_DEBUGS
	eprintf("loadave_getdata: exiting rs=%d\n",rs) ;
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



