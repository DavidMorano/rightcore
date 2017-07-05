/* getstacksize */

/* get the machine HZ */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 2001-04-11, David A­D­ Morano
	This is a spin off of various programs that needed to get system
	(machine) specific paramaters.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	Retrieve a stack size value from the system.

	Synopsis:

	int getstacksize(int w)

	Arguments:

	w		which value to retrieve:
				0 -> minimum stack size
				1 -> default stack size

	Returns:

	<0		error
	==0		?
	>0		returned value


	Notes:

        Of course, both minimum and default stack sizes are highly OS dependent.
        Although there seems to be a way to find out what the minimum stack size
        (for a given OS) is, there does not seem to be a way to retrieve the
        default stack (see DEFSTACKSIZE define below) size for anyone (any OS).
        So, bascially, this code needs to be updated and recompiled for any
        given OS. Some better solution for this situation should be found.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#if	_LP64
#define	DEFSTACKSIZE	(2*1024)
#else
#define	DEFSTACKSIZE	(1*1024)
#endif

#define	GETSTACKSIZE	struct getstacksize


/* local structures */

struct getstacksize {
	int		ss[2] ;
} ;


/* forward references */

static int	getval(int) ;


/* local variables */

static GETSTACKSIZE	getstacksize_data ; /* zero-initialized */


/* exported subroutines */


int getstacksize(int w)
{
	GETSTACKSIZE	*op = &getstacksize_data ;
	int		rs ;
	if (op->ss[w] == 0) {
	    if ((rs = getval(w)) >= 0) {
	        op->ss[w] = rs ;
	    }
	} else {
	    rs = op->ss[w] ;
	}
	return rs ;
}
/* end subroutine (getstacksize) */


/* local subroutines */


static int getval(int w)
{
	int		rs = SR_INVALID ;
	switch (w) {
	case 0:
	    {
		const int	cmd = _SC_THREAD_STACK_MIN ;
	        rs = uc_sysconf(cmd,NULL) ;
	    }
	    break ;
	case 1:
	    rs = DEFSTACKSIZE ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */
	return rs ;
}
/* end subroutine (getval) */


