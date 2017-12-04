/* svckey */

/* service key data-structure-conversion access */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2008-07-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This object really forms some glue to pass service entry parameters
	(like from a file processed with SVCFILE) to other objects that expect
	to be handed pointers to service-entry related string values.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"svckey.h"


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static cchar	*svckeys[] = {
	"file",
	"so",
	"program",
	"passfile",
	"args",
	"username",
	"groupname",
	"interval",
	"access",
	"opts",
	"failcont",
	"include",
	NULL
} ;


/* exported subroutines */


int svckey_load(SVCKEY *skp,SVCFILE_ENT *sep)
{
	int		i ;
	int		c = 0 ;

	if (skp == NULL) return SR_FAULT ;
	if (sep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("svckey_load: sep=%p\n",sep) ;
	debugprintf("svckey_load: svc(%p)\n",sep->svc) ;
	debugprintf("svckey_load: svc=%s\n",sep->svc) ;
#endif

	memset(skp,0,sizeof(SVCKEY)) ;

	skp->svc = sep->svc ;
	for (i = 0 ; sep->keyvals[i][0] != NULL ; i += 1) {
	    int		ki ;
	    cchar	*kp = sep->keyvals[i][0] ;
	    cchar	*vp = sep->keyvals[i][1] ;
	    if ((ki = matostr(svckeys,1,kp,-1)) >= 0) {
#if	CF_DEBUGS
	        debugprintf("svckey_load: ki=%d kp=%s vp=>%s<\n",ki,kp,vp) ;
#endif
	        switch (ki) {
	        case svckey_file:
	            skp->file = vp ;
	            break ;
	        case svckey_pass:
	            skp->pass = vp ;
	            break ;
	        case svckey_so:
	            skp->so = vp ;
	            break ;
	        case svckey_p:
	            skp->p = vp ;
	            break ;
	        case svckey_a:
	            skp->a = vp ;
	            break ;
	        case svckey_u:
	            skp->u = vp ;
	            break ;
	        case svckey_g:
	            skp->g = vp ;
	            break ;
	        case svckey_interval:
	            skp->interval = vp ;
	            break ;
	        case svckey_acc:
	            skp->acc = vp ;
	            break ;
	        case svckey_opts:
	            skp->opts = vp ;
	            break ;
	        case svckey_failcont:
	            skp->failcont = vp ;
	            break ;
	        case svckey_include:
	            skp->include = vp ;
	            break ;
	        } /* end switch */
	        if (ki >= 0) c += 1 ;
	    } /* end if (matostr) */
	} /* end for */

	return c ;
}
/* end subroutine (svckey_load) */


