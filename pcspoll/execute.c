/* execute */

/* execute a server daemon program */
/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* debug print-outs */
#define	CF_ISAEXEC	1		/* try 'isaexec(3c)' */


/* revision history:

	= 1986-07-01, David A­D­ Morano
	This program was originally written.

	= 1998-07-14, David A­D­ Morano
        I added the ability to specify the "address_from" for the case when we
        add an envelope header to the message.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This subroutine just 'exec(2)'s a daemon server program.


***************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int execute(pip,program,alp,elp)
struct proginfo	*pip ;
char		program[] ;
VECSTR		*alp ;
VECSTR		*elp ;
{
	int	rs ;


#if	CF_DEBUG
	if (pip->debuglevel > 2) {
	    int	i ;
	    char	*sp ;
	    debugprintf("execute: program=%s\n",program) ;
	    for (i = 0 ; vecstr_get(alp,i,&sp) >= 0 ; i += 1)
	        debugprintf("execute: arg%d> %s\n",i,sp) ;
	}
#endif /* CF_DEBUG */

	if ((pip->workdname != NULL) && (pip->workdname[0] != '\0'))
	    u_chdir(pip->workdname) ;

#if	CF_ISAEXEC && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	rs = uc_isaexecve(program,alp->va,elp->va) ;
#endif

	rs = uc_execve(program,alp->va,elp->va) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("execute: exec rs=%s\n",rs) ;
#endif /* CF_DEBUG */

	return rs ;
}
/* end subroutine (execute) */



