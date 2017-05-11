/* uc_getpj */

/* interface component for UNIX® library-3c */
/* project DB access */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SOLARISBUG	1		/* Solaris® errno bad on access */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine was written so that we could use a single interface to
        access the PEOJECT database on all UNIX® platforms. This code module
        provides a platform independent implementation of UNIX® PROJECT database
        access subroutines.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<project.h>

#include	<vsystem.h>
#include	<projectent.h>
#include	<localmisc.h>


/* local defines */

#define	TO_AGAIN	10


/* external subroutines */

extern int	msleep(int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


#if	defined(SYSHAS_PROJECT) && (SYSHAS_PROJECT > 0)

int uc_setpjent()
{
	setprojent() ;
	return SR_OK ;
}

int uc_endpjent()
{
	endprojent() ;
	return SR_OK ;
}

int uc_getpjent(struct project *pjp,char rbuf[],int rlen)
{
	struct project	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_getpjent: ent\n") ;
#endif

	if (pjp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	repeat {
	    {
	        errno = 0 ;
	        rp = getprojent(pjp,rbuf,(size_t) rlen) ;
	        if (rp != NULL) {
	            rs = projectent_size(pjp) ;
	        } else {
	            rs = (- errno) ;
	        }
	    }

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

#if	CF_DEBUGS
	debugprintf("uc_getpjent: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_getpjent) */

int uc_getpjbyid(projid_t projid,struct project *pjp, char rbuf[],int rlen)
{
	struct project	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_getpjbyid: ent projid=%d\n",projid) ;
#endif

	if (pjp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	repeat {
	    {
	        errno = 0 ;
	        rp = getprojbyid(projid,pjp,rbuf,(size_t) rlen) ;
	        if (rp != NULL) {
	            rs = projectent_size(pjp) ;
	        } else {
	            rs = (errno != 0) ? (- errno) : SR_NOTFOUND ;
		}
	    }

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
	            break;
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if */
	} until ((rs >= 0) || f_exit) ;

#if	CF_DEBUGS
	debugprintf("uc_getpjbyid: ret rs=%d\n",rs) ;
	debugprintf("uc_getpjbyid: project=%s\n",pjp->pj_name) ;
#endif

	return rs ;
}
/* end subroutine (uc_getpjbyid) */

int uc_getpjbyname(cchar name[],struct project *pjp,char rbuf[],int rlen)
{
	struct project	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_getpjbyname: ent name=%s\n",name) ;
#endif

	if (name == NULL) return SR_FAULT ;
	if (pjp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	repeat {
	    {
	        errno = 0 ;
	        rp = getprojbyname(name,pjp,rbuf,(size_t) rlen) ;
	        if (rp != NULL) {
	            rs = projectent_size(pjp) ;
	        } else {
	            rs = (errno != 0) ? (- errno) : SR_NOTFOUND ;
		}
	    }

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
	    } /* end if */
	} until ((rs >= 0) || f_exit) ;

#if	CF_DEBUGS
	debugprintf("uc_getpjbyname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_getpjbyname) */


#else /* defined(SYSHAS_PROJECT) && (SYSHAS_PROJECT > 0) */


int uc_setpjent()
{
	return SR_NOSYS ;
}

int uc_endpjent()
{
	return SR_NOSYS ;
}

int uc_getpjent(struct project *pjp,char rbuf[],int rlen)
{
	if (pjp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	return SR_NOSYS ;
}

int uc_getpjbyid(projid_t projid,struct project *pjp,char rbuf[],int rlen)
{
	if (pjp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	return SR_NOSYS ;
}

int uc_getpjbyname(cchar name[],struct project *pjp,char rbuf[],int rlen)
{
	if (name == NULL) return SR_FAULT ;
	if (pjp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	return SR_NOSYS ;
}

#endif /* defined(SYSHAS_PROJECT) && (SYSHAS_PROJECT > 0) */


