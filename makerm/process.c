/* process */

/* process a name */


#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The subroutine was written from scratch.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	O_EXTRA		(O_WRONLY | O_CREAT | O_TRUNC)



/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int process(pip,dirname,name)
struct proginfo	*pip ;
const char	dirname[] ;
char		name[] ;
{
	struct ustat	sb ;

	int	rs, sl ;

	char	dfname[MAXPATHLEN + 2] ;
	char	*bfname ;


	if ((dirname == NULL) || (name == NULL))
	    return SR_FAULT ;

	if ((name[0] == '\0') || (name[0] == '-'))
	    return SR_INVALID ;


	bfname = name ;
	if (name[0] != '/') {

	    bfname = dfname ;
	    mkpath2(dfname, dirname,name) ;

	}

	if ((rs = u_stat(bfname,&sb)) >= 0) {

	    rs = 1 ;
	    u_unlink(bfname) ;

	} else
	    rs = 0 ;

	return rs ;
}
/* end subroutine (process) */



