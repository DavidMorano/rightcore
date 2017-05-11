/* u_getloadavg */

/* get the standard load averages maintained by the kernel */
/* translation layer interface for UNIX® equivalents */


/* revision history:

	= 2001-04-23, David A­D­ Morano
        Well Solaris-8 supplied this new capability. I have no idea about any
        status as a standard of any sort but this is a lot easier than using
        'kstat(3kstat)'! I continued to use (and still do) 'kstat(3kstat)' on
        Solaris-8 since it first came out. Originally I resisted investigating
        how to get the direct load averages out of the kernel besides using
        'kstat(3kstat)'. A little investigation (easy) showed that they had
        implemented a new system call to do it. I guess that they were also
        getting tired of using 'kstat(3kstat)' for kernel load averages!

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Yes, this grabs (rather directly and easily), the load averages
        maintained by the kernel. Solaris-8 (I think) was the first version to
        allow these kernel load averages to be grabbed without using
        'kstat(3kstat)' (but I am not sure). Anyway, this little subroutine uses
        the UNDOCUMENTED new system call in Solaris-8 to grab these load
        averages!

        Let me repeat myself for those in the audience who didn't quite hear me
        correctly the first time. This subroutine uses the secret and
        UNDOCUMENTED new system call to grab the load averages from the kernel.
        This could break at any time! If you do not believe me, go back to using
        the draft version of some of the stupid POSIX reentrant calls and see
        how long they will be continued to be supported!

	Synopsis:

	int u_getloadavg(la,n)
	uint	la[] ;
	int	n ;

	Arguments:

	la	array of unsigned integers of at least 'n' elements
	n	number of elements in the array 'la'

	Returns:

	>=0	OK
	<0	error

	CAUTION:

        This subroutine uses a secret and undocumented new system call
        introduced maybe with Solaris-8. This function could go away at any time
        so don't get to comfortable with using it! Don't give up your day job
        but rather instead keep your old 'kstat(3kstat)' stuff around for
        getting load averages when this new thing suddenly "goes away"!

	Extra information:

        If you can stand to get the load averages converted to the type double
        (an array of doubles rather than an array of ints), then check out
        'getloadavg(3c)'.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>

#if	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
#include	<sys/loadavg.h>
#elif	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)
#include	<stdlib.h>
#endif

#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_AGAIN	5


/* external subroutines */

extern int	msleep(int) ;


/* local structures */


/* forward references */

#if	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)
static int uc_getloadavg(double *,int) ;
#endif


/* local variables */


/* exported subroutines */


#if	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)

int u_getloadavg(uint *la,int n)
{
	int		rs = SR_OK ;

	if (la == NULL) return SR_FAULT ;

	if (n < 0) return SR_INVALID ;

#if	defined(OSNUM) && (OSNUM >= 8)
	if (n >= 0) {
	    if (n > LOADAVG_NSTATS) n = LOADAVG_NSTATS ;
	    if ((rs = __getloadavg((int *) la,n)) < 0) {
	        rs = (- errno) ;
	    }
	}
#else
	rs = (- ENOSYS) ;
#endif /* defined(OSNUM) && (OSNUM >= 8) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (u_getloadavg) */

#elif	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)

int u_getloadavg(uint *la,int n)
{
	double		d[3] ;
	int		rs = SR_INVALID ;

	if (la == NULL) return SR_FAULT ;

	if (n >= 0) {
	    if (n > 3) n = 3 ;
	    if ((rs = uc_getloadavg(d,n)) >= 0) {
	        int	i ;
	        for (i = 0 ; i < n ; i += 1) {
	            la[i] = (uint) (d[i] * FSCALE) ;
		}
	    } /* end if */
	} /* end if (greater-than) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (u_getloadavg) */


static int uc_getloadavg(double *la,int n)
{
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = getloadavg(la,n)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	            if (to_again-- > 0) {
			msleep(100) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (uc_getloadavg) */

#else /* all other implementations */

int u_getloadavg(uint la,int n)
{
	int		rs = SR_OK ;

	if (la == NULL) return SR_FAULT ;

	if (n >= 0)
	    rs = SR_NOSYS ;

	return rs ;
}

#endif /* which implementation */


