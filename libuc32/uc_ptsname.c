/* uc_ptsname */

/* interface component for UNIX® library-3c */
/* get the filename (path) of a slave-pseudo terminal device */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        This is written to get a portable (reentrant and as not theaded) version
        of PTRNAME as we can get.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This code only has meaning on the newer System V UNIX® releases with the
        PTS dirver. This is now needed to get the filename of the slave side
        device of the new pseudo-terminal clone multiplexor driver. A new
        slave-side filename looks something like '/dev/pts/26'.

	The algorithm (from SVR3 docs) is:

        Check that the FD argument is a file descriptor of an opened master. Do
        this by sending an ISPTM ioctl message down stream. Ioctl() will fail
        if: (1) FD is not a valid file descriptor, (2) the file represented by
        FD does not understand ISPTM (not a master device). If we have a valid
        master, get its minor number via fstat(). Concatenate it to PTSPREFIX
        and return it as the name of the slave device.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mkdev.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ptms.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#include	<vsystem.h>


/* local defines */

#define	PTSPREFIX	"/dev/pts/"	/* slave name */
#define	PTSPREFIXLEN	9		/* length of above string */
#define	PTSMAXLEN	32		/* slave name length */
#define	PTSMAXDEVS	1000000000	/* rather arbitrary */


/* forward references */

static int	cvtstr(char *,int) ;


/* exported subroutines */


int uc_ptsname(int fd,char *nbuf,int nlen)
{
	struct strioctl istr ;
	int		rs ;
	int		len = 0 ;

	if (nbuf == NULL) return SR_FAULT ;

	if (nlen < (PTSMAXLEN + 1)) return SR_OVERFLOW ;

	memset(&istr,0,sizeof(struct strioctl)) ;
	istr.ic_cmd = ISPTM ;
	istr.ic_len = 0 ;
	istr.ic_timout = 0 ;
	istr.ic_dp = NULL ;

	if ((rs = u_ioctl(fd,I_STR,&istr)) >= 0) {
	    struct ustat	sb ;
	    if ((rs = u_fstat(fd, &sb)) >= 0) {
		uint	minordev = minor(sb.st_rdev) ;
/* assume that something is bad if the number is too large */
		if (minordev <= PTSMAXDEVS) {
/* put the number together with the prefix */
		    strcpy(nbuf, PTSPREFIX) ;
		    len = PTSPREFIXLEN ;
		    len += cvtstr((nbuf+len),minordev) ;
/* is the filename there in the file system? */
		    rs = u_access(nbuf,0) ;
		} else {
	    	    rs = SR_INVALID ;
		}
	    } /* end if (u_fstat) */
	} /* end if (u_ioctl) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (uc_ptsname) */


/* local subroutines */


/* convert an integer to a decimal string */
static int cvtstr(char *ptr,int i)
{
	int		rs ;
	int		dig = 0 ;
	int		tempi = i ;

	do {
	    dig += 1 ;
	    tempi /= 10 ;
	} while (tempi) ;

	rs = dig ;
	ptr += dig ;
	*ptr = '\0' ;
	while (--dig >= 0) {
	    *(--ptr) = i % 10 + '0' ;
	    i /= 10 ;
	}

	return rs ;
}
/* end subroutine (cvtstr) */


