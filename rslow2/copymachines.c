/* copymachines */

/* copy the NETRC machine entries to a VECELEM */


#define	CF_DEBUG	1


/* revision history:

	- 94/12/01, David A­D­ Morano

	This was copied from the REXEC program.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Execute as :

	int copymachines()


**************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<time.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>

#include	<bfile.h>
#include	<vecelem.h>
#include	<logfile.h>
#include	<netfile.h>
#include	<rex.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	getnodedomain(char *,char *) ;
extern int	quoteshellarg() ;

extern int	hostequiv() ;


/* forward subroutines */


/* external variables */


/* local variables */

struct global	g ;



/* copy machine entries from a NETRC file to a vector element list */
int copymachines(netrcp,filename,machine,localdomain)
vecelem		*netrcp ;
char		filename[] ;
char		machine[] ;
char		localdomain[] ;
{
	NETFILE		tmp ;

	struct netrc	*mp ;

	int		rs, i ;
	int		f_euid = FALSE ;

	char		*hnp, hostname[4096 + 1] ;


	if (u_access(filename,R_OK) < 0) {

	    f_euid = TRUE ;
	    seteuid(g.euid) ;

	}

	if ((rs = netfileopen(&tmp,filename)) >= 0) {

	    if (f_euid) {

	        f_euid = FALSE ;
	        seteuid(g.uid) ;

	    }

	    for (i = 0 ; netfileget(&tmp,i,&mp) >= 0 ; i += 1) {

	        if (mp == NULL) continue ;

	        if (mp->machine == NULL) continue ;

/* convert all machine names to canonical form */

	        hnp = mp->machine ;
	        if (getchostname(mp->machine,hostname) >= 0)
	            hnp = hostname ;

/* copy over only those machines that match our target machine */

	        if (! hostequiv(hnp,machine,localdomain)) continue ;

	        if (hnp == hostname) {

	            free(mp->machine) ;

	            mp->machine = mallocstr(hostname) ;

	        }

	        vecelemadd(netrcp,mp,sizeof(struct netrc)) ;

	        mp->machine = NULL ;
	        mp->login = NULL ;
	        mp->password = NULL ;
	        mp->account = NULL ;

	    } /* end for */

	    netfileclose(&tmp) ;

	} /* end if */

	if (f_euid)
		seteuid(g.uid) ;

	return rs ;
}
/* end subroutine (copymachines) */



