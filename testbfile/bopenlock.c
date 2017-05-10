/* bopenlock */

/* program to catpure a lock based on whether a file exists or not */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	- 1998-09-01, David A­D­ Morano

	The program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine implements a locking strategy based on the
	existence of a file.  If the file name given exists already
	then the lock is not captured.  If the file does not exist,
	it is created and the lock is captured.  To release the lock,
	one would simply remove the file.


*******************************************************************************/


#define	BFILE_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#define	DEFWTIME	30
#define	TIME1DAY	(24 * 3600)
#define	TIME1HOUR	3600


/* external subroutines */


/* external variables */


/* forward references */


/* exported subroutines */


int bopenlock(lfp,filename,timeout,oldtime)
bfile		*lfp ;
const char	filename[] ;
int		timeout ;
int		oldtime ;
{
	BFILE_STAT	sb ;

	time_t	checktime = 0 ;

	int	rs = SR_OK ;
	int	f_checked = FALSE ;


	if (lfp == NULL)
	    return SR_FAULT ;

/* check arguments */

	if ((filename == NULL) || (filename[0] == '\0'))
	    return SR_INVALID ;

	if (timeout <= 0)
	    timeout = DEFWTIME ;

	if (oldtime < 0)
	    f_checked = TRUE ;

/* go through the loops */

	while (timeout > 0) {

	    rs = bopen(lfp,filename,"wce",0444) ;
	    if (rs >= 0) break ;

/* check if the lock file is too old */

	    if ((! f_checked) && (bopen(lfp,filename,"r",0666) >= 0)) {

	        f_checked = TRUE ;
	        if (bcontrol(lfp,BC_STAT,&sb) >= 0) {

	            if (checktime == 0) {
	                time_t	daytime = time(NULL) ;
	                checktime = daytime - oldtime ;
	            }

	            if (sb.st_mtime < checktime)
	                u_unlink(filename) ;

	        } /* end if */

	        bclose(lfp) ;

	    } /* end if (check for old lock) */

	    sleep(1) ;

	    timeout -= 1 ;

	} /* end while */

	if ((rs >= 0) && (timeout <= 0)) rs = SR_LOCKED ;

	return rs ;
}
/* end subroutine (bopenlock) */



