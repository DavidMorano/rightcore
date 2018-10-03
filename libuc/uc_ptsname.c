/* uc_ptsname */

/* interface component for UNIX® library-3c */
/* get the filename (path) of a slave-pseudo terminal device */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        This is written to get a portable (reentrant and as not theaded) version
        of PTRNAME as we can get.

	= 2018-10-03, David A.D. Morano
	I modernized this by replacing custom path creation crap with a call
	to the |snsd(3uc)| subroutine.

*/

/* Copyright © 1998,2018 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This code only has meaning on the newer System V UNIX® releases with the
        PTS dirver. This is now needed to get the filename of the slave side
        device of the new pseudo-terminal clone multiplexor driver. A new
        slave-side filename looks something like '/dev/pts/26'.

	Unlike other versions of this sort of function, this is thread-safe!

	The algorithm (from SVR3 docs) is:

        Check that the FD argument is a file descriptor of an opened master. Do
        this by sending an ISPTM ioctl message down stream. Ioctl() will fail
        if: (1) FD is not a valid file descriptor, (2) the file represented by
        FD does not understand ISPTM (not a master device). If we have a valid
        master, get its minor number via fstat(). Concatenate it to PTSPREFIX
        and return it as the name of the slave device.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mkdev.h>
#include	<sys/stream.h>
#include	<sys/stropts.h>
#include	<sys/stat.h>
#include	<sys/ptms.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>


/* local defines */

#define	PTSPREFIX	"/dev/pts/"	/* slave name */
#define	PTSPREFIXLEN	9		/* length of above string */
#define	PTSMAXLEN	32		/* slave name length */
#define	PTSMAXDEVS	1000000000	/* rather arbitrary */


/* external subroutines */

extern int	snsd(char *,int,cchar *,uint) ;


/* forward references */


/* exported subroutines */


int uc_ptsname(int fd,char *nbuf,int nlen)
{
	struct strioctl istr ;
	int		rs ;
	int		len = 0 ;

	if (nbuf == NULL) return SR_FAULT ;

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
		if (minordev < PTSMAXDEVS) {
/* put the number together with the prefix */
		    if ((rs = snsd(nbuf,nlen,PTSPREFIX,minordev)) >= 0) {
			len = rs ;
/* is the filename there in the file system? */
		        rs = u_access(nbuf,0) ;
		    }
		} else {
	    	    rs = SR_INVALID ;
		}
	    } /* end if (u_fstat) */
	} /* end if (u_ioctl) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (uc_ptsname) */

