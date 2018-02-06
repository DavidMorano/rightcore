/* sigign */

/* manage process signals */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This was created along with the DATE object.

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

#include	"sigign.h"


/* local defines */

#define	SIGIGN_HANDLE	struct sigign_handle

#define	NDF		"sigign.deb"


/* external subroutines */

#if	CF_DEBUGS || CF_DEBUGN
extern int	debugprintf(const char *,...) ;
extern int	nprintf(const char *,const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* exported variables */


/* local subroutines */


/* forward references */


/* local variables */

static const int	sigouts[] = {
	SIGTTOU,
	0
} ;


/* exported subroutines */


int sigign_start(SIGIGN *iap,const int *ignores)
{
	int		rs = SR_OK ;
	int		nhandles = 0 ;

	if (iap == NULL) return SR_FAULT ;
	if (ignores == NULL) ignores = sigouts ;

	memset(iap,0,sizeof(SIGIGN)) ;

	if (ignores != NULL) {
	    int		i ;
	    for (i = 0 ; ignores[i] != 0 ; i += 1) nhandles += 1 ;
	    nhandles = i ;
	    if (nhandles > 0) {
	        const int	size = (nhandles * sizeof(SIGIGN_HANDLE)) ;
	        void		*p ;
	        iap->nhandles = nhandles ;
	        if ((rs = uc_malloc(size,&p)) >= 0) {
	            SIGACTION	san, *sap ;
	            sigset_t	nsm ;
	            iap->handles = (SIGIGN_HANDLE *) p ;

/* ignore these signals */

	            uc_sigsetempty(&nsm) ;

	            if (ignores != NULL) {
	                SIGIGN_HANDLE	*hp = iap->handles ;
	                int		hsig ;

	                for (i = 0 ; (rs >= 0) && (ignores[i] != 0) ; i += 1) {
	                    hsig = ignores[i] ;
	                    hp[i].sig = hsig ;
	                    sap = &hp[i].action ;
	                    memset(&san,0,sizeof(SIGACTION)) ;
	                    san.sa_handler = SIG_IGN ;
	                    san.sa_mask = nsm ;
	                    san.sa_flags = 0 ;
	                    rs = u_sigaction(hsig,&san,sap) ;
	                } /* end for */

	                if (rs < 0) {
	                    int		j ;
	                    for (j = (i-1) ; j >= 0 ; j -= 1) {
	                        hsig = hp[j].sig ;
	                        sap = &hp[j].action ;
	                        u_sigaction(hsig,sap,NULL) ;
	                    }
			}
	            } /* end if (ignores) */

		    if (rs < 0) {
	      		uc_free(iap->handles) ;
			iap->handles = NULL ;
	 	     }
	        } /* end if (memory allocations) */
	    } /* end if (handles) */
	} /* end if (ignores) */

#if	CF_DEBUGN
	nprintf(NDF,"sigign_start: after-allocations rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    iap->magic = SIGIGN_MAGIC ;
	}

#if	CF_DEBUGN
	nprintf(NDF,"sigign_start: ret rs=%d nhandles=%u\n",
	    rs,iap->nhandles) ;
#endif

	return (rs >= 0) ? nhandles : rs ;
}
/* end subroutine (sigign_start) */


int sigign_finish(SIGIGN *iap)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (iap == NULL) return SR_FAULT ;
	if (iap->magic != SIGIGN_MAGIC) return SR_NOTOPEN ;

	if (iap->handles != NULL) {
	    SIGACTION	*sap ;
	    int		hsig ;
	    int		j ;
	    for (j = (iap->nhandles-1)  ; j >= 0 ; j -= 1) {
	        hsig = iap->handles[j].sig ;
	        sap = &iap->handles[j].action ;
	        rs1 = u_sigaction(hsig,sap,NULL) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end for */
	    rs1 = uc_free(iap->handles) ;
	    if (rs >= 0) rs = rs1 ;
	    iap->handles = NULL ;
	} /* end if */

	iap->magic = 0 ;
	return rs ;
}
/* end subroutine (sigign_finish) */


