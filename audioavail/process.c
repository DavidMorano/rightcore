/* process */

/* process this device */


#define	CF_DEBUG 	0


/* revision history:

	= 96/03/01, David A­D­ Morano

	This subroutine was originally written.


*/


/***********************************************************************


	Synopsis:


	Arguments:

	- pip		program information pointer
	- devdname	directory at top of tree
	- name		device name to pop off !


	Returns:


***********************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#ifndef	LINELEN
#define	LINELEN		(MAXPATHLEN + 10)
#endif



/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;


/* external variables */






int process(pip,ofp,devdname,name)
struct proginfo	*pip ;
bfile		*ofp ;
char		devdname[] ;
char		name[] ;
{
	struct ustat	sb ;

	int	rs, len ;

	char	dfname[MAXPATHLEN + 1] ;
	char	*bdp ;


	if ((devdname == NULL) || (devdname[0] == '\0'))
		return SR_FAULT ;

	if (name == NULL)
	    return SR_FAULT ;

	bdp = name ;
	if ((name[0] != '/') &&
	    ((rs = perm(name,-1,-1,NULL,R_OK | W_OK)) < 0)) {

	    bdp = dfname ;
	    mkpath2(dfname,devdname,name) ;

	    rs = perm(bdp,-1,-1,NULL,(R_OK | W_OK)) ;

		if (rs < 0)
	        bprintf(pip->efp,"%s: could not access \"%s\" rs=%d\n",
	            pip->progname,bdp,rs) ;

	} /* end if */

	if (pip->verboselevel > 0)
	    bprintf(ofp,"device=%s\n",bdp) ;

	if (rs >= 0)
	rs = u_open(bdp,O_WRONLY | O_NONBLOCK,0666) ;

	if (rs >= 0) {

	    u_close(rs) ;

	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (process) */



