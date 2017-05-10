/* openint_hello */

/* LOCAL facility open-service (hello) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_OPENTMP	1		/* use 'opentmp()' */


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

	int openint_hello(pr,dn,bn,prn,of,om,argv,envv,to)
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
#include	<baops.h>
#include	<ids.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<vechand.h>
#include	<sbuf.h>
#include	<buffer.h>
#include	<paramfile.h>
#include	<nulstr.h>
#include	<logfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"openint_hello.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;


/* exported subroutines */


int openint_hello(pr,dn,bn,prn,of,om,argv,envv,to)
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
	int	rs = SR_OK ;
	int	pipes[2] ;
	int	fd = 0 ;
	int	sl = -1 ;

	const char	*sp = "hello world!\n" ;


	rs = u_pipe(pipes) ;
	fd = pipes[0] ;
	if (rs >= 0) {
	    int	wfd = pipes[1] ;

	    if (sl < 0) sl = strlen(sp) ;

	    rs = u_write(wfd,sp,sl) ;

	    u_close(wfd) ;
	    if (rs < 0) u_close(fd) ;
	} /* end if */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (openint_hello) */



