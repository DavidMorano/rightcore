/* termstore */

/* store terminal attributes */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_TCATTR	1		/* use |tcXXXattr(3termion)| */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This code was originally written for use by PCS 'vmail'.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/time.h>
#include	<unistd.h>
#include	<termios.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"termstore.h"


/* external varaibles */


/* local variables */


/* exported subroutines */


int termstore_save(tsp,tfd)
struct termstore	*tsp ;
int			tfd ;
{
	struct ustat	sb ;

	int		rs ;


/* save the terminal mode first */

	tsp->f_stored = FALSE ;
	if ((rs = u_fstat(tfd,&sb)) < 0) 
		return rs ;

	tsp->mode = sb.st_mode ;

#if	CF_TCATTR
	rs = uc_tcgetattr(tfd,&tsp->settings) ;
#else
	rs = u_ioctl(tfd,TCGETS,&tsp->settings) ;
#endif

	if (rs < 0) 
		return rs ;

	tsp->f_stored = TRUE ;
	return rs ;
}
/* end subroutine (termstore_save) */


int termstore_restore(tsp,tfd)
struct termstore	*tsp ;
int			tfd ;
{
	struct ustat	sb ;

	int		rs ;


	if (! tsp->f_stored) 
		return SR_INVALID ;

/* restore the terminal settings */

#if	CF_TCATTR
	rs = uc_tcsetattr(tfd,TCSADRAIN,&tsp->settings) ;
#else
	rs = u_ioctl(tfd,TCSETSW,&tsp->settings) ;
#endif


/* restore the terminal mode */

	if ((u_fstat(tfd,&sb) < 0) || (sb.st_mode != tsp->mode))
		rs = u_fchmod(tfd,tsp->mode) ;

	return rs ;
}
/* end subroutine (termstore_restore) */



