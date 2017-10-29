/* isfsremote */

/* is the file on a local or remote filesystem? */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine checks if the specified file-descriptor (FD) points to a
        file on a remote file-system.

	Synopsis:

	int isfsremote(int fd)

	Arguments:

	fd		file-descriptor to check

	Returns:

	<0		error
	==0		not on remote file-system
	>0		Yes, on a remote file-system


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */

extern int	getfstype(char *,int,int) ;
extern int	matstr(const char **,const char *,int) ;


/* external variables */


/* local structures */


/* forward refernces */

int islocalfs(const char *,int) ;


/* local variables */

static const char	*localfs[] = {
	"ufs",
	"tmpfs",
	"lofs",
	"zfs",
	"vxfs",
	"pcfs",
	"hsfs",
	NULL
} ;


/* exported subroutines */


int isfsremote(int fd)
{
	int		rs ;
	int		f = FALSE ;
	char		fstype[USERNAMELEN + 1] ;

	if ((rs = getfstype(fstype,USERNAMELEN,fd)) >= 0) {
	    f = (matstr(localfs,fstype,rs) < 0) ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (isfsremote) */


int islocalfs(const char *np,int nl)
{
	return (matstr(localfs,np,nl) >= 0) ;
}
/* end subroutine (islocalfs) */


