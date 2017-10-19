/* getbufsize */

/* get various system buffer sizes */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */
#define	CF_DEBUGN	0		/* special */


/* revision history:

	= 2001-04-11, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We return various system buffer sizes. Some of these are extracted from
        the operating system. The rest we just made up values by ourselves.

	Synopsis:

	int getbufsize(int w)

	Arguments:

	w		which source to use:
				0 -> ARGS
				1 -> PW
				2 -> SP
				3 -> UA
				4 -> GR
				5 -> PJ
				6 -> PE
				7 -> SE
				8 -> NE
				9 -> HE

	Returns:

	<0		error
	==0		?
	>0		buffer size


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"getbufsize.h"


/* local defines */

#define	GETBUFSIZE_CONF	"/etc/default/bufsize"

#define	NDF		"getbufsize.deb"

/* password entry */
#ifndef	PWBUFLEN
#define	PWBUFLEN	1024		/* Solaris® _SC_GETPW_R_SIZE_MAX */
#endif

/* shadow-password entry */
#ifndef	SPBUFLEN
#define	SPBUFLEN	1024		/* same as PWBUFLEN for now */
#endif

/* user-attribute entry */
#ifndef	UABUFLEN
#define	UABUFLEN	(2*1024)	/* this is just a suggestion */
#endif

/* group entry */
#ifndef	GRBUFLEN
#define	GRBUFLEN	7296		/* Solaris® _SC_GETGR_R_SIZE_MAX */
#endif

/* project entry */
#ifndef	PJBUFLEN
#define	PJBUFLEN	(4 * 1024)	/* Solaris® recommends (4*1024) */
#endif

/* protocol entry */
#ifndef	PEBUFLEN
#define	PEBUFLEN	100
#endif

/* service entry */
#ifndef	SEBUFLEN
#define	SEBUFLEN	100
#endif

/* network entry */
#ifndef	NEBUFLEN
#define	NEBUFLEN	(4 * 1024)
#endif

/* host entry */
#ifndef	HEBUFLEN
#define	HEBUFLEN	(8 * 1024)
#endif

#define	GETBUFSIZE	struct getbufsize


/* external variables */


/* external subroutines */

extern int	matostr(cchar **,int,cchar *,int) ;
extern int	matocasestr(cchar **,int,cchar *,int) ;
extern int	cfdecmfi(cchar *,int,int *) ;
extern int	msleep(int) ;
extern int	vecstr_envfile(vecstr *,cchar *) ;
extern int	isNotPresent(int) ;
extern int	isNotValid(int) ;


/* local structures */

struct getbufsize {
	volatile int	f_init ;
	volatile int	f_initdone ;
	volatile int	f_begin ;
	volatile int	f_loaded ;
	int		bs[getbufsize_overlast] ;
} ;


/* forward references */

static int	getbufsize_begin(GETBUFSIZE *) ;
static int	getbufsize_load(GETBUFSIZE *) ;
static int	getbufsize_def(GETBUFSIZE *,int) ;
static int	getbufsize_sysbs(GETBUFSIZE *,int,int) ;


/* local variables */

static GETBUFSIZE	getbufsize_data ; /* zero-initialized */

/* these must match the enumerations */
static cchar	*vars[] = {
	"ARGS",
	"PASSWD",
	"SHADOW",
	"USERATTR",
	"GROUP",
	"PROJECT",
	"PROTOCOL",
	"SERVICE",
	"NETWORK",
	"HOST",
	NULL
} ;
/* these must match the enumerations */


/* exported subroutines */


int getbufsize_init()
{
	GETBUFSIZE	*uip = &getbufsize_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
#if	CF_DEBUGN
	nprintf(NDF,"getbufsize_init: ent\n") ;
#endif
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
#if	CF_DEBUGN
	    nprintf(NDF,"getbufsize_init: _begin()\n") ;
#endif
	    if ((rs = getbufsize_begin(uip)) >= 0) {
	        uip->f_initdone = TRUE ;
	        f = TRUE ;
	    }
	    if (rs < 0)
	        uip->f_init = FALSE ;
	} else {
	    while ((rs >= 0) && uip->f_init && (! uip->f_initdone)) {
	        rs = msleep(1) ;
		if (rs == SR_INTR) break ;
	    }
	    if ((rs >= 0) && (! uip->f_init)) rs = SR_LOCKLOST ;
	}
#if	CF_DEBUGN
	nprintf(NDF,"getbufsize_init: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (getbufsize_init) */


int getbufsize(int w)
{
	int		rs = SR_INVALID ;

#if	CF_DEBUGN
	nprintf(NDF,"getbufsize: ent w=%d\n",w) ;
#endif

	if (w < getbufsize_overlast) {
	    GETBUFSIZE	*gbp = &getbufsize_data ;
	    if ((rs = gbp->bs[w]) == 0) {
	        if ((rs = getbufsize_init()) >= 0) {
	            rs = getbufsize_def(gbp,w) ;
	        } /* end if (getbufsize_init) */
	    } /* end if (need initialization) */
	} /* end if (valid) */

#if	CF_DEBUGN
	nprintf(NDF,"getbufsize: ret rs=%d\n",rs) ;
#endif
#if	CF_DEBUGS
	debugprintf("getbufsize: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (getbufsize) */


/* local subroutines */


static int getbufsize_begin(GETBUFSIZE *uip)
{
	int		rs = SR_OK ;
	if (! uip->f_begin) {
	    uip->f_begin = TRUE ;
	    rs = getbufsize_load(uip) ;
	}
	return rs ;
}
/* end subroutine (getbufsize_begin) */


static int getbufsize_load(GETBUFSIZE *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (! uip->f_loaded) {
	    VECSTR	cv ;
	    uip->f_loaded = TRUE ;
	    if ((rs = vecstr_start(&cv,1,0)) >= 0) {
	        cchar	*fn = GETBUFSIZE_CONF ;
	        if ((rs = vecstr_envfile(&cv,fn)) >= 0) {
	            int		i ;
	            int		kl, vl ;
	            cchar	*tp ;
	            cchar	*kp, *vp ;
	            for (i = 0 ; vecstr_get(&cv,i,&kp) >= 0 ; i += 1) {
	                if (kp != NULL) {
	                    vp = NULL ;
	                    kl = -1 ;
	                    vl = 0 ;
	                    if ((tp = strchr(kp,'=')) != NULL) {
	                        kl = (tp-kp) ;
	                        vp = (tp+1) ;
	                        vl = -1 ;
	                    }
	                    if (vp != NULL) {
	                        int	w ;
	                        if ((w = matocasestr(vars,4,kp,kl)) >= 0) {
	                            int	v ;
	                            if ((rs = cfdecmfi(vp,vl,&v)) >= 0) {
	                                uip->bs[w] = v ;
	                            } else if (isNotValid(rs)) {
	                                rs = SR_OK ;
	                            }
	                        } /* end if (matocasestr) */
	                    } /* end if (non-null) */
	                } /* end if (non-null) */
	                if (rs < 0) break ;
	            } /* end for */
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        }
	        rs1 = vecstr_finish(&cv) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (vecstr) */
	} /* end if (need load) */
	return rs ;
}
/* end subroutine (getbufsize_load) */


static int getbufsize_def(GETBUFSIZE *uip,int w)
{
	int		rs ;
#if	CF_DEBUGN
	nprintf(NDF,"getbufsize_get: ent w=%d\n",w) ;
#endif
	if ((rs = uip->bs[w]) == 0) {
	    int		name = -1 ;
	    switch (w) {
	    case getbufsize_args:
	        name = _SC_ARG_MAX ;
	        break ;
	    case getbufsize_pw:
	        name = _SC_GETPW_R_SIZE_MAX ;
	        break ;
	    case getbufsize_sp:
	        rs = SPBUFLEN ;
	        break ;
	    case getbufsize_ua:
	        rs = UABUFLEN ;
	        break ;
	    case getbufsize_gr:
	        name = _SC_GETGR_R_SIZE_MAX ;
	        break ;
	    case getbufsize_pj:
	        rs = PJBUFLEN ;
	        break ;
	    case getbufsize_pe:
	        rs = PEBUFLEN ;
	        break ;
	    case getbufsize_se:
	        rs = SEBUFLEN ;
	        break ;
	    case getbufsize_ne:
	        rs = NEBUFLEN ;
	        break ;
	    case getbufsize_he:
	        rs = HEBUFLEN ;
	        break ;
	    default:
	        rs = SR_INVALID ;
	        break ;
	    } /* end switch */
	    if (rs > 0) {
	        uip->bs[w] = rs ;
	    } else if (rs == 0) {
	        if (name >= 0) {
	            rs = getbufsize_sysbs(uip,w,name) ;
	        } else {
	            rs = SR_BUGCHECK ;
	        }
	    }
	} /* end if (getting default valie) */
#if	CF_DEBUGN
	nprintf(NDF,"getbufsize_gef: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (getbufsize_def) */


static int getbufsize_sysbs(GETBUFSIZE *gbp,int w,int name)
{
	int		rs ;
	if (gbp->bs[w] == 0) {
	    if ((rs = uc_sysconf(name,NULL)) >= 0) {
	        gbp->bs[w] = rs ;
	    }
	} else {
	    rs = gbp->bs[w] ;
	}
	return rs ;
}
/* end subroutine (getbufsize_sysbs) */


