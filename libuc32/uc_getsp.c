/* uc_getsp */

/* interface component for UNIX® library-3c */
/* shadow password DB access */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SOLARISBUG	1		/* Solaris® errno bad on access */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine was written so that we could use a single interface to
	access the 'passwd' database on all UNIX® platforms.  This code module
	provides a platform independent implementation of UNIX® 'passwd'
	database access subroutines.

	Notes:

	+ Solaris® bug

	On Solaris®, there is some sort of bug when trying to access the
	SHADOW-PASSWD database facility when (perhaps) NIS+ is running on the
	system.  Instead of returning SR_ACCESS (access violation) when the
	SHADOW-PASSWD database is inaccessible (which is almost all of the time
	for almost all regular users), it returns SR_BADF (bad
	file-descriptor).  A compile-time switch is available for converting
	SR_BADF into SR_ACCESS in this case.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<shadow.h>

#include	<vsystem.h>
#include	<storeitem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_AGAIN	10

#define	RF_SPCOPY	0		/* flag to set need of 'spcopy()' */
#if	defined(SYSHAS_GETSPENTR) && (SYSHAS_GETSPENTR > 0)
#else
#undef	RF_SPCOPY
#define	RF_SPCOPY	1
#endif
#if	defined(SYSHAS_GETSPXXXR) && (SYSHAS_GETSPXXXR > 0)
#else
#undef	RF_SPCOPY
#define	RF_SPCOPY	1
#endif


/* external subroutines */

extern int	msleep(int) ;


/* external variables */


/* local structures */


/* forward references */

#if	defined(SYSHAS_SHADOW) && (SYSHAS_SHADOW > 0)
static int	spsize(struct spwd *,const char *) ;
#endif

#if	RF_SPCOPY
static int	spcopy(struct spwd *,char *,int,struct spwd *) ;
static int	si_copystr(STOREITEM *,char **,const char *) ;
#endif


/* local variables */


/* exported subroutines */


#if	defined(SYSHAS_SHADOW) && (SYSHAS_SHADOW > 0)

int uc_setspent()
{
	setspent() ;
	return SR_OK ;
}

int uc_endspent()
{
	endspent() ;
	return SR_OK ;
}

int uc_getspent(struct spwd *spp,char rbuf[],int rlen)
{
	struct spwd	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	if (spp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	repeat {

#if	defined(SYSHAS_GETSPENTR) && (SYSHAS_GETSPENTR > 0)
	{
	    errno = 0 ;
	    rp = getspent_r(spp,rbuf,rlen) ;
	    if (rp != NULL) {
	        rs = spsize(spp,rbuf) ;
	    } else {
	        rs = (- errno) ;
	    }
	}
#else /* SYSHAS_GETSPENTR */
	{
	    rp = getspent() ;
	    if (rp != NULL) {
	        rs = spcopy(spp,rbuf,rlen,rp) ;
	    } else {
	        rs = (- errno) ;
	    }
	}
#endif /* SYSHAS_GETSPENTR */

#if	CF_SOLARISBUG
		if (rs == SR_BADF) rs = SR_NOENT ;
#endif /* CF_SOLARISBUG */

	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
		    if (to_again-- > 0) {
	                msleep(1000) ;
	 	    } else {
		        f_exit = TRUE ;
		    }
		    break ;
	        case SR_INTR:
	            break ;
	        default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (uc_getspent) */

int uc_getspnam(cchar name[],struct spwd *spp,char rbuf[],int rlen)
{
	struct spwd	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_getspnam: ent name=%s\n",name) ;
	debugprintf("uc_getspnam: splen=%d\n",rlen) ;
	debugprintf("uc_getspnam: spp{%p}\n",spp) ;
	debugprintf("uc_getspnam: spbuf{%p}\n",rbuf) ;
#endif

	if (name == NULL) return SR_FAULT ;
	if (spp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	repeat {

#if	CF_DEBUGS
	debugprintf("uc_getspnam: trying\n") ;
#endif

#if	defined(SYSHAS_GETSPXXXR) && (SYSHAS_GETSPXXXR > 0)
	{
	    errno = 0 ;
#if	CF_DEBUGS
	debugprintf("uc_getspnam: trying-have-r\n") ;
#endif
	    rp = getspnam_r(name,spp,rbuf,rlen) ;
#if	CF_DEBUGS
	debugprintf("uc_getspnam: getspnam_r() r=%p\n",rp) ;
	debugprintf("uc_getspnam: errno=%d\n",errno) ;
#endif
	    if (rp != NULL) {
	        rs = spsize(spp,rbuf) ;
	    } else {
	        rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	    }
	}
#else /* SYSHAS_GETSPXXXR */
	{
#if	CF_DEBUGS
	debugprintf("uc_getspnam: trying-have-not-r\n") ;
#endif
	    rp = getspnam(name) ;
	    if (rp != NULL) {
	        rs = spcopy(spp,rbuf,rlen,rp) ;
	    } else {
	        rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	    }
	}
#endif /* SYSHAS_GETSPXXXR */

#if	CF_SOLARISBUG
		if (rs == SR_BADF) rs = SR_NOENT ;
#endif /* CF_SOLARISBUG */

#if	CF_DEBUGS
	debugprintf("uc_getspnam: mid rs=%d\n",rs) ;
#endif
	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
		    if (to_again-- > 0) {
	                msleep(1000) ;
	 	    } else {
		        f_exit = TRUE ;
		    }
		    break ;
	        case SR_INTR:
	            break ;
	        default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

#if	CF_DEBUGS
	debugprintf("uc_getspnam: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_getspnam) */


#else /* defined(SYSHAS_SHADOW) && (SYSHAS_SHADOW > 0) */


int uc_setspent()
{
	return SR_NOSYS ;
}

int uc_endspent()
{
	return SR_NOSYS ;
}

/* ARGSUSED */
int uc_getspent(struct spwd *spp,rbuf,rlen)
{
	if (spp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	return SR_NOSYS ;
}
/* end subroutine (uc_getspent) */

/* ARGSUSED */
int uc_getspnam(cchar name[],struct spwd *spp,rbuf,rlen)
{
	if (name == NULL) return SR_FAULT ;
	if (spp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	return SR_NOSYS ;
}
/* end subroutine (uc_getspnam) */


#endif /* defined(SYSHAS_SHADOW) && (SYSHAS_SHADOW > 0) */


/* local subroutines */


#if	defined(SYSHAS_SHADOW) && (SYSHAS_SHADOW > 0)

static int spsize(struct spwd *sp,cchar *buf)
{
	int		rs = 0 ;
	const char	*ol = buf ;
	const char	*p ;

	p = sp->sp_namp ;
	if ((p != NULL) && (p > ol))
	    ol = p ;

	p = sp->sp_pwdp ;
	if ((p != NULL) && (p > ol))
	    ol = p ;

	if (ol > buf) {
	    rs = ol + strlen(ol) + 1 - buf ;
	}

	return rs ;
}
/* end subroutine (spsize) */

#if	RF_SPCOPY

static int spcopy(rp,rbuf,rlen,sp)
struct spwd	*rp ;
char		rbuf[] ;
int		rlen ;
struct spwd	*sp ;
{
	STOREITEM	ib ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("uc_getsp/spcopy: ent rlen=%d\n",rlen) ;
#endif

	memcpy(rp,sp,sizeof(struct spwd)) ;

	if ((rs = storeitem_start(&ib,rbuf,rlen)) >= 0) {

	    si_copystr(&ib,&rp->sp_name,sp->sp_name) ;

	    si_copystr(&ib,&rp->sp_pwdp,sp->sp_pwdp) ;

	    rs1 = storeitem_finish(&ib) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (storeitem) */

#if	CF_DEBUGS
	debugprintf("uc_getsp/spcopy: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (spcopy) */

static int si_copystr(ibp,pp,p1)
STOREITEM	*ibp ;
char		**pp ;
const char	*p1 ;
{
	int		rs = SR_OK ;
	const char	**cpp = (const char **) pp ;

	*cpp = NULL ;
	if (p1 != NULL) {
	    rs = storeitem_strw(ibp,p1,-1,cpp) ;
	}

	return rs ;
}
/* end subroutine (si_copystr) */

#endif /* RF_SPCOPY */

#endif /* defined(SYSHAS_SHADOW) && (SYSHAS_SHADOW > 0) */


