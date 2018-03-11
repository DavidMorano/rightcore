/* main */

/* front-end for HAVEPROGRAM */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_USAGE	0		/* |usage()| */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little program checks to see if programs are available with the
	current PATH environment variables.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<ids.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#define	NPATHS		40		/* starting guess of number paths */


/* external subroutines */

extern int	getprogpath(IDS *,VECSTR *,char *,const char *,int) ;
extern int	vecstr_addpath(vecstr *,const char *,int) ;
extern int	vecstr_addpathclean(vecstr *,const char *,int) ;


/* external variables */


/* local strutures */


/* forward references */

#if	CF_USAGE
static int	usage(PROGINFO *) ;
#endif


/* local variables */


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	VECSTR		dirs ;
	int		rs = SR_INVALID ;
	int		ex = EX_USAGE ;
	const char	*varpath = VARPATH ;
	const char	*path ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

/* must have supplied arguments in order to be valid */

	if (argc <= 1)
	    goto badarg ;

/* initialize and load DB for directory paths (NIL "PATH" is allowed) */

	ex = EX_OSERR ;
	if ((rs = vecstr_start(&dirs,NPATHS,0)) >= 0) {
	    if ((path = getenv(varpath)) != NULL) {
	        rs = vecstr_addpathclean(&dirs,path,-1) ;
	    } else {
	        rs = vecstr_addpath(&dirs,"",0) ;
	    }
	    if (rs >= 0) {
		IDS	id ;
	        if ((rs = ids_load(&id)) >= 0) {
		    int		i ;
		    const char	*pn ;
		    char	pbuf[MAXPATHLEN + 1] ;
	            ex = EX_NOPROG ;
	            for (i = 1 ; (i < argc) && (argv[i] != NULL) ; i += 1) {
	                pn = argv[i] ;
	                if ((pn[0] != '\0') && (strcmp(pn,"--") != 0)) {
	                    rs = getprogpath(&id,&dirs,pbuf,pn,-1) ;
	                    if (rs < 0) break ;
	                } /* end if */
	            } /* end for */
	            ids_release(&id) ;
	        } /* end if (ids) */
	    } /* end if (ok) */
	    vecstr_finish(&dirs) ;
	} /* end if (vecstr) */

	if (rs >= 0) ex = EX_OK ;

badarg:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


#if	CF_USAGE
/* print out (standard error) some short usage */
static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	if (pip->efp != NULL) {

	    fmt = "%s: USAGE> %s "
	        "[-e] [-f] [-l] [-i] [-m] [-x] [-xu] [-c] [-a] "
	        "<name(s)>\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	    wlen += rs ;

	    fmt = "%s:  [-q] [-p <varname(s)>] [-s <section(s)>] \n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	    wlen += rs ;

	    fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	    wlen += rs ;

	} /* end if (error-output enabled) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */
#endif /* CF_USAGE */


