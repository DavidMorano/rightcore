/* u_mmap */

/* map a file into memory */
/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	More Solaris® bugs!

        Stupid Solaris tries to prevent developers from mapping files while
        simultaneously using record locks on that file. There are even some
        reports that (stupid) Solaris also prevents users from mapping files and
        allowing that same file from being mounted remotely. However, the
        problem is that (stupid) Solaris is not consistent in its attempts to
        prevent uses from mapping these sorts of files. If someone maps a file
        that has record locks on it but maps it at exactly the size of that
        file, Solaris prevents it (or allows it, I forget). But if someone maps
        a file beyond the end of it, then (stupid) Solaris denies (or allows)
        it.

	Developer beware!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_AGAIN	6


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_mmap(addr,size,prot,flags,fd,off,vp)
caddr_t		addr ;
size_t		size ;
int		prot, flags ;
int		fd ;
offset_t	off ;
void		*vp ;
{
	caddr_t		ra ;
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;
	caddr_t		*rpp = (caddr_t *) vp ;

#if	CF_DEBUGS
	debugprintf("u_mmap: ent\n") ;
#endif

	repeat {
	    rs = SR_OK ;
	    ra = mmap(addr,size,prot,flags,fd,off) ;
	    if (ra == ((caddr_t) MAP_FAILED)) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	            if (to_again-- > 0) {
	                msleep(1000) ;
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

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? ra : NULL ;
	}

	return rs ;
}
/* end subroutine (u_mmap) */


int u_mapfile(addr,size,prot,flags,fd,off,vp)
caddr_t		addr ;
size_t		size ;
int		prot, flags ;
int		fd ;
offset_t	off ;
void		*vp ;
{
	return u_mmap(addr,size,prot,flags,fd,off,vp) ;
}
/* end subroutine (u_mapfile) */


