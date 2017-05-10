/* lockend */

/* lock the end of the a file */


/* revision history:

	= 2009-01-20, David A­D­ Morano
        This is a complete rewrite of the trash that performed this function
        previously.


*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine is used to lock the end of files. Other simple
        subroutines don't quite give a simple way to lock the end of files. Of
        course, 'fcntl(2)' can do anything along these lines.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<signal.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */

extern int	msleep(int) ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int lockend(fd,f_lock,f_read,to)
int		fd ;
int		f_lock ;
int		f_read ;
int		to ;
{
	struct flock	fl ;

	int	rs = SR_OK ;
	int	i ;
	int	f_exit = FALSE ;


	if (f_lock) {

	    fl.l_type = (f_read) ? F_RDLCK : F_WRLCK ;
	    fl.l_whence = SEEK_END ;
	    fl.l_start = 0L ;
	    fl.l_len = 0L ;
	    rs = SR_TIMEDOUT ;
	    for (i = 0 ; i < to ; i += 1) {

	        if (i > 0)
	            sleep(1) ;

	        f_exit = TRUE ;
	        rs = u_fcntl(fd,F_SETLK,&fl) ;

	        if (rs < 0) {
	            switch (rs) {
	            case SR_INTR:
	            case SR_ACCES:
	            case SR_AGAIN:
	                f_exit = FALSE ;
	                break ;
	            } /* end switch */
	        } /* end if */

	        if (f_exit) break ;
	    } /* end for (timing) */

	} else {

	    fl.l_type = F_UNLCK ;
	    fl.l_whence = SEEK_END ;
	    fl.l_start = 0L ;
	    fl.l_len = 0L ;
	    rs = u_fcntl(fd,F_SETLK,&fl) ;

	} /* end if */

	return rs ;
}
/* end subroutine (lockend) */



