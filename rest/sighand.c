/* sighand */

/* manage process signals */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This small object provides a way to manage (block, ignore, and catch)
	process signals.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"sighand.h"


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
extern int	nprintf(const char *,const char *,...) ;
#endif


/* exported variables */


/* local subroutines */


/* forward references */


/* local variables */


/* exported subroutines */


int sighand_start(iap,blocks,ignores,catches,handle)
SIGHAND		*iap ;
const int	*blocks ;
const int	*ignores ;
const int	*catches ;
sighand_handler	handle ;
{
	sigset_t	nsm ;
	int		rs = SR_OK ;
	int		i ;
	int		nhandles = 0 ;
	int		size ;
	void		*p ;

	if (iap == NULL) return SR_FAULT ;

	memset(iap,0,sizeof(SIGHAND)) ;

	if (handle == NULL) handle = (void *) SIG_IGN ;

/* blocks */

	if ((rs >= 0) && (blocks != NULL)) {
	    uc_sigsetempty(&nsm) ;
	    for (i = 0 ; blocks[i] != 0 ; i += 1) {
	        uc_sigsetadd(&nsm,blocks[i]) ;
	    }
	    iap->nblocks = i ;
	    pthread_sigmask(SIG_BLOCK,&nsm,&iap->osm) ;
	} /* end if (blocks) */

/* calculate the allocations size */

	if (ignores != NULL) {
	    for (i = 0 ; ignores[i] != 0 ; i += 1) {
	        nhandles += 1 ;
	    }
	}
	if (catches != NULL) {
	    for (i = 0 ; catches[i] != 0 ; i += 1) {
	        nhandles += 1 ;
	    }
	}

	size = (nhandles * sizeof(SIGHAND_HANDLE)) ;
	if ((rs >= 0) && (size > 0) && ((rs = uc_malloc(size,&p)) >= 0)) {
	    SIGHAND_HANDLE *hp = p ;
	    SIGACTION	san, *sap ;
	    int		hsig ;
	    int		j = 0 ;
	    iap->handles = (SIGHAND_HANDLE *) p ;
	    iap->nhandles = nhandles ;

/* ignore these signals */

	    uc_sigsetempty(&nsm) ;

	    if (ignores != NULL) {

	        for (i = 0 ; ignores[i] != 0 ; i += 1) {
	            hsig = ignores[i] ;
	            hp[j].sig = hsig ;
	            sap = &hp[j].action ;
	            memset(&san,0,sizeof(SIGACTION)) ;
	            san.sa_handler = SIG_IGN ;
	            san.sa_mask = nsm ;
	            san.sa_flags = 0 ;
	            rs = u_sigaction(hsig,&san,sap) ;
	            if (rs < 0) break ;
	            j += 1 ;
	        } /* end for */

	    } /* end if (ignores) */

/* catch (interrupt on) these signals */

	    if ((rs >= 0) && (catches != NULL)) {

	        for (i = 0 ; catches[i] != 0 ; i += 1) {
	            hsig = catches[i] ;
	            hp[j].sig = hsig ;
	            sap = &hp[j].action ;
	            memset(&san,0,sizeof(SIGACTION)) ;
	            san.sa_sigaction = (sighand_handler) handle ;
	            san.sa_mask = nsm ;
	            san.sa_flags = (SA_SIGINFO) ;
	            rs = u_sigaction(hsig,&san,sap) ;
	            if (rs < 0) break ;
	            j += 1 ;
	        } /* end for */

	    } /* end if (catches) */

	    if (rs < 0) { /* error */
		for (i = (j-1) ; i >= 0 ; i -= 1) {
	  	    hsig = hp[i].sig ;
		    sap = &hp[i].action ;
		    u_sigaction(hsig,sap,NULL) ;
		}
	        uc_free(iap->handles) ;
	        iap->handles = NULL ;
	    }
	} /* end if (memory allocations) */

	if (rs >= 0)
	    iap->magic = SIGHAND_MAGIC ;

	return rs ;
}
/* end subroutine (sighand_start) */


int sighand_finish(SIGHAND *iap)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (iap == NULL) return SR_FAULT ;
	if (iap->magic != SIGHAND_MAGIC) return SR_NOTOPEN ;

	if (iap->handles != NULL) {
	    SIGACTION	*sap ;
	    int		hsig ;
	    int		i ;
	    for (i = (iap->nhandles-1)  ; i >= 0 ; i -= 1) {
	        hsig = iap->handles[i].sig ;
	        sap = &iap->handles[i].action ;
	        rs1 = u_sigaction(hsig,sap,NULL) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end for */
	    rs1 = uc_free(iap->handles) ;
	    if (rs >= 0) rs = rs1 ;
	    iap->handles = NULL ;
	} /* end if */

	if (iap->nblocks > 0)  {
	    pthread_sigmask(SIG_SETMASK,&iap->osm,NULL) ;
	}

	iap->magic = 0 ;
	return rs ;
}
/* end subroutine (sighand_finish) */


