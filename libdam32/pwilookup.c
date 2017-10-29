/* pwilookup */

/* wrapper functon dealing with the PWI object */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	Module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little subroutine provides a bridge between those applications that
        want to lookup a single name (real-name) in the IPASSWD DB but do not
        want to use either the PWI or the IPASSWD objects directly (for whatever
        reason).

        Note that we use a PWI object method that does not allow for multiple DB
        responses. This is the desired behavior for some applications and is
        actually the most popular behavior desired so far.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"pwi.h"


/* local defines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external subroutines */


/* local variables */


/* exported subroutines */


int pwilookup(cchar *pr,cchar *dbname,char *rbuf,int rlen,cchar *name)
{
	PWI		index ;
	int		rs ;
	int		rs1 ;
	int		rl = 0 ;

#if	CF_DEBUGS
	debugprintf("pwilookup: pr=%s\n",pr) ;
	debugprintf("pwilookup: dbname=%s\n",dbname) ;
	debugprintf("pwilookup: name=%s\n",name) ;
#endif

	if (pr == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;

	if (name[0] == '\0') return SR_INVALID ;

	rbuf[0] = '\0' ;
	if ((rs = pwi_open(&index,pr,dbname)) >= 0) {

	    rs = pwi_lookup(&index,rbuf,rlen,name) ;
	    rl = rs ;

#if	CF_DEBUGS
	debugprintf("pwilookup: pwi_lookup() rs=%d\n",rs) ;
#endif

	    rs1 = pwi_close(&index) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (open) */

#if	CF_DEBUGS
	debugprintf("pwilookup: ret rs=%d rbuf=%s\n",rs,rbuf) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (pwilookup) */


