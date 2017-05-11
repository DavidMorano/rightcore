/* debugsetfd */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<unistd.h>



/* this is it here!!  we are defining this thing!! */

int	err_fd = 2 ;




int debugsetfd(fd)
int	fd ;
{
	struct ustat	sb ;

	int		rs = -1 ;


	if ((fd < 256) && (fstat(fd,&sb) >= 0))
		rs = err_fd = fd ;

	else if ((access((char *) fd,W_OK) >= 0) &&
		((rs = open((char *) fd,O_WRONLY | O_APPEND,0666)) >= 0))
		err_fd = rs ;

	return rs ;
}
/* end subroutine (debugsetfd) */



