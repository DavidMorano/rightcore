/* openint_copyout */

/* LOCAL facility open-service (copyout) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-11-04, David A­D­ Morano
        This code was started by taking the corresponding code from the
        TCP-family module. In retrospect, that was a mistake. Rather I should
        have started this code by using the corresponding UUX dialer module.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a facility-open-intercept module.

	Synopsis:

	int openint_copyout(pr,dn,bn,prn,of,om,argv,envv,to)
	const char	*pr ;
	const char	*dn ;
	const char	*bn ;
	const char	*prn ;
	int		of ;
	mode_t		om ;
	const char	**argv ;
	const char	**envv ;
	int		to ;

	Arguments:

	pr		program-root
	dn		dir-name
	bn		base-name
	prn		facility name
	of		open-flags
	om		open-mode
	argv		argument array
	envv		environment array
	to		time-out

	Returns:

	>=0		file-descriptor
	<0		error


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>

#include	"openint_copyout.h"
#include	"defs.h"


/* local defines */


/* external variables */


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local structures */


/* local variables */


/* forward references */


/* exported subroutines */


int openint_copyout(pr,dn,bn,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*dn ;
const char	*bn ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	int		rs = SR_OK ;
	int		fd = -1 ;
	const char	*inter = "copyout" ;
	char		fname[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	debugprintf("openint_copyout: pr=%s\n",pr) ;
	debugprintf("openint_copyout: dn=%s\n",dn) ;
	debugprintf("openint_copyout: prn=%s\n",prn) ;
#endif

	if ((rs = mkpath2(fname,dn,bn)) >= 0) {
	    rs = u_open(fname,of,om) ;
	    fd = rs ;
	}

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (openint_copyout) */


/* local subroutines */



