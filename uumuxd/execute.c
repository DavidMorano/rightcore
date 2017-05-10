/* execute */

/* execute a server daemon program */
/* last modified %G% version %I% */


#define	CF_DEBUG	1


/* revision history:

	= David A.D. Morano, July 1986

	This program was originally written.

	= David A.D. Morano, July 1998

	I added the ability to specify the "address_from"
	for the case when we add an envelope header to the message.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This subroutine just 'exec(2)'s a daemon server program.


***************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/utsname.h>
#include	<sys/param.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<signal.h>
#include	<string.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"srvpe.h"
#include	"srventry.h"
#include	"builtin.h"
#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	getfiledirs() ;
extern int	processargs(char *,VECSTR *) ;
extern int	process() ;

extern char	*strbasename() ;
extern char	*timestr_log(), *timestr_edate() ;
extern char	*malloc_str(), *malloc_strn(), *malloc_sbuf(), *malloc_buf() ;


/* external variables */

extern struct global	g ;


/* forward references */


/* local variables */


/* exported subroutines */


int execute(gp,s,elp,program,arg0,alp)
struct global	*gp ;
int		s ;
VECSTR		*elp ;
char		program[], arg0[] ;
VECSTR		*alp ;
{
	int	rs ;

	char	*oldarg0 ;


#if	CF_DEBUG
	if (gp->debuglevel > 2) {

		int	i ;

		char	*sp ;


		debugprintf("execute: program=%s\n",program) ;
		debugprintf("execute: arg0=%s\n",arg0) ;

		for (i = 0 ; vecstrget(alp,i,&sp) >= 0 ; i += 1)
			debugprintf("execute: arg%d> %s\n",i,sp) ;

	}
#endif /* CF_DEBUG */

	if ((g.workdir != NULL) && (g.workdir[0] != '\0'))
		u_chdir(g.workdir) ;

	if ((s != 0) && (s != 1))
		u_close(s) ;

	oldarg0 = alp->va[0] ;
	alp->va[0] = arg0 ;
	{
	    const char	**eav = (const char **) alp->va ;
	    const char	**eev = (const char **) elp->va ;
	    rs = u_execve(program,eav,eev) ;
	}

#if	CF_DEBUG
	if (gp->debuglevel > 2)
		debugprintf("execute: exec rs=%s\n",rs) ;
#endif /* CF_DEBUG */

	alp->va[1] = oldarg0 ;
	return rs ;
}
/* end subroutine (execute) */



