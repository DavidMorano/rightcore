/* fileobject */

/* determine if named file is an object file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Given a file-name we determine if it is an object file (or core file).

	Synopsis:

	int fileobject(fname)
	const char	fname[] ;

	Arguments:

	fname		file-path to check

	Returns:

	<0		error
	==0		not an object file
	>0		is an object file


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int fileobject(cchar *fname)
{
	int		rs ;
	int		f = FALSE ;

	if (fname == NULL) return SR_FAULT ;
	if (fname[0] == '\0') return SR_INVALID ;

	if ((rs = u_open(fname,O_RDONLY,0666)) >= 0) {
	    struct ustat	sb ;
	    int			fd = rs ;

	    if (((rs = u_fstat(fd,&sb)) >= 0) && S_ISREG(sb.st_mode)) {
		char	buf[4 + 1] ;
	        if ((rs = u_read(fd,buf,4)) >= 4) {
	            f = (memcmp(buf,"\177ELF",4) == 0) ;
		}
	    } /* end if (stat) */

#if	CF_DEBUGS
	debugprintf("fileobject: stat() rs=%d f=%u\n",rs,f) ;
#endif

	    u_close(fd) ;
	} /* end if (open) */

#if	CF_DEBUGS
	debugprintf("fileobject: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (fileobject) */


