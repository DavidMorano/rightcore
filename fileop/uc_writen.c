/* uc_writen */

/* interface component for UNIX® library-3c */
/* write a fixed number of bytes */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUGWRITE	0		/* debug aboiding 'u_write(3u)' */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine writes a fixed number of bytes even for those devices
        that can return without writing all of the bytes that we wanted to.

	Synopsis:

	int uc_writen(fd,ubuf,ulen)
	int		fd ;
	const void	*ubuf ;
	int		ulen ;

	Arguments:

	fd		file descriptor
	ubuf		buffer holding data to write
	ulen		length of data to write

	Returns:

	>=0		number of bytes written
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/uio.h>
#include	<unistd.h>
#include	<string.h>
#include	<errno.h>

#include	<vsystem.h>


/* local defines */

#define	LIBUC_DEBUGFILE		"libuc.deb"


/* external subroutines */


/* external variables */


/* forward references */

#if	CF_DEBUG && CF_DEBUGWRITE
static int	debugwrite(int,const void *,int) ;
#endif


/* local variables */


/* exported subroutines */


int uc_writen(int fd,const void *abuf,int alen)
{
	int		rs = SR_OK ;
	int		len ;
	int		alenr = alen ;
	const char	*abp = (const char *) abuf ;

	if (fd < 0)
	    return SR_BADF ;

	if (abuf == NULL)
	    return SR_FAULT ;

	if (alen < 0) {
	    cchar	*ap = (cchar *) abuf ;
	    alen = strlen(ap) ;
	}

	if (alenr > 0) {

	    while ((rs >= 0) && (alenr > 0)) {

#if	CF_DEUGWRITE
	        rs = debugwrite(fd,abp,alenr) ;
		len = rs ;
#else
	        rs = u_write(fd,abp,alenr) ;
		len = rs ;
#endif

		if (rs >= 0) {
		    abp += len ;
		    alenr -= len ;
		}

	    } /* end while */

	} else {

#if	CF_DEBUG && CF_DEBUGWRITE
	    debugprintf("uc_writen: zero-length write!\n") ;
	        rs = debugwrite(fd,abp,alenr) ;
#else
	    rs = u_write(fd,abp,0) ;
#endif

	} /* end if */

#if	CF_DEBUGS
	debugprintf("uc_writen: ret rs=%d \n",rs) ;
#endif

	return (rs >= 0) ? alen : rs ;
}
/* end subroutine (uc_writen) */


/* local subroutines */


#if	CF_DEBUG && CF_DEBUGWRITE
#include	<errno.h>
static int debugwrite(fd,abuf,alen)
int		fd ;
const void	*abuf ;
int		alen ;
{
	int	rslen ;
	errno = 0 ;
	if ((rs = write(fd,abuf,alen)) == -1) rs = (- errno) ;
	return rs ;
}
/* end subroutine (debugwrite) */
#endif /* CF_DEBUGWRITE */


