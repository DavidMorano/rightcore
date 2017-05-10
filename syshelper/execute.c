/* execute */

/* execute a server daemon program */
/* last modified %G% version %I% */


#define	CF_DEBUG	1


/* revision history:

	= 86/07/01, David A­D­ Morano
	This program was originally written.

	= 98/07/14, David A­D­ Morano
	I added the ability to specify the "address_from"
	for the case when we add an envelope header to the message.


*/



/**************************************************************************

	This subroutine just 'exec(2)'s a daemon server program.



***************************************************************************/




#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/utsname.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<vecstr.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */



/* external subroutines */


/* external variables */


/* forward references */


/* local data */





int execute(gp,s,program,arg0,alp,elp)
struct proginfo	*gp ;
int		s ;
char		program[], arg0[] ;
VECSTR		*alp ;
VECSTR		*elp ;
{
	int	rs ;

	char	*oldarg0 ;


#if	CF_DEBUG
	if (gp->debuglevel > 2) {
		int	i ;
		char	*sp ;
		debugprintf("execute: program=%s\n",program) ;
		debugprintf("execute: arg0=%s\n",arg0) ;
		for (i = 0 ; vecstr_get(alp,i,&sp) >= 0 ; i += 1)
			debugprintf("execute: arg%d> %s\n",i,sp) ;
	}
#endif /* CF_DEBUG */

	if ((gp->workdir != NULL) && (gp->workdir[0] != '\0'))
		u_chdir(gp->workdir) ;

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



