/* teststackaddr */
/* lang=C89 */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#define	CF_DEBUGS	1
#include	<envstandards.h>
#include	<stdio.h>
#include	<vsystem.h>

#include	"stackaddr.h"

#define	VARDEBUGFNAME	"TESTSTACKADDR_DEBUGFILE"

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;

struct pair {
	const char	*hp ;
	const char	*up ;
} ;

static struct pair	pairs[] = {
	{ "h3", "u3" },
	{ "h3", "u3a" },
	{ "h2", "u2" },
	{ NULL, "u2a" },
	{ "h2", "u2b" },
	{ "h1", "u1" },
	{ NULL, "u0" },
	{ NULL, NULL }
} ;


int main(int argc,const char **argv,const char **envv)
{
	STACKADDR	s ;

	const int	rlen = MAXPATHLEN ;

	int	rs ;
	int	rs1 ;

	char	rbuf[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	{
	    const char	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL)
	        debugopen(cp) ;
	    debugprintf("main: starting\n") ;
	}
#endif /* CF_DEBUGS */

	if ((rs = stackaddr_start(&s,rbuf,rlen)) >= 0) {
	    int		i ;
	    const char	*hp ;
	    const char	*up ;
	    debugprintf("main: stackaddr_start() rs=%d\n",rs) ;

	    for (i = 0 ; pairs[i].up != NULL ; i += 1) {
		hp = pairs[i].hp ;
		up = pairs[i].up ;
		rs = stackaddr_add(&s,hp,-1,up,-1) ;
	        debugprintf("main: stackaddr_add() rs=%d\n",rs) ;
		if (rs < 0) break ;
	    } /* end for */

	    rs1 = stackaddr_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (grcache) */
	debugprintf("main: out rs=%d\n",rs) ;

	printf("sa=%s\n",rbuf) ;

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */

