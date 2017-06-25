/* mapshmtmp */

/* map some POSIX® SHM-type shared memory */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_UNLINK	1		/* unlink SHM object on success */


/* revision history:

	= 2006-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2006 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine creates and maps a POSIX® shared-memory temporary
        object. The POSIX® SHM-type name of the object created is returned to
        the caller.

	Synopsis:

	int mapshmtmp(rbuf,rlen,operm,shmlen,rpp)
	char		rbuf[] ;
	int		rlen ;
	mode_t		operm ;
	int		shmlen ;
	char		**rpp ;

	Arguments:

	rbuf		suppled buffer to receive the resulting SHM name
	rlen		length of suppled buffer; should be at least SHMNAMELEN
	operm		SHM object creation opermissions
	shmlen		desired length of resulting SHM object
	rpp		pointer to hold address of resulting SHM object

	Returns:

	>=0		file descriptor to program STDIN and STDOUT
	<0		error


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	SHMNAMELEN
#define	SHMNAMELEN	MAXNAMELEN
#endif


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	openshmtmp(char *,int,int) ;


/* external variables */


/* forward reference */

static int	shmalloc(int,int) ;


/* local variables */


/* exported subroutines */


int mapshmtmp(char *rbuf,int rlen,mode_t operm,int shmlen,char **rpp)
{
	int		rs = SR_OK ;
	int		f_bufalloc = FALSE ;

	if (rpp == NULL) return SR_FAULT ;

	if (shmlen <= 0) return SR_INVALID ;

	if ((rbuf != NULL) && (rlen >= 0) && (rlen < SHMNAMELEN))
	    return SR_OVERFLOW ;

	if (rbuf == NULL) {
	    rlen = SHMNAMELEN ;
	    if ((rs = uc_malloc(rlen,&rbuf)) >= 0) {
	        f_bufalloc = TRUE ;
	    }
	}

	if ((rs >= 0) && ((rs = openshmtmp(rbuf,rlen,operm)) >= 0)) {
	    const int	fd = rs ;

	    if (f_bufalloc) {
	         uc_unlinkshm(rbuf) ;
	    }

	    if ((rs = shmalloc(fd,shmlen)) >= 0) {
		size_t	msize = shmlen ;
	        int	mprot = (PROT_READ | PROT_WRITE) ;
	        int	mopts = MAP_SHARED ;
	        rs = u_mmap(NULL,msize,mprot,mopts,fd,0L,rpp) ;
	    } /* end if */

	    u_close(fd) ;
	} /* end if (openshmtmp) */

	if (f_bufalloc && (rbuf != NULL)) {
	    uc_free(rbuf) ;
	}

	return rs ;
}
/* end subroutine (mapshmtmp) */


/* local subroutines */


static int shmalloc(int fd,int shmlen)
{
	offset_t	off = 0 ;
	const int	wlen = sizeof(int) ;
	const int	ps = getpagesize() ;
	int		rs = SR_OK ;
	char		wbuf[sizeof(int) + 1] ;

	memset(wbuf,0,wlen) ;

	while ((rs >= 0) && (off < shmlen)) {
	    rs = u_pwrite(fd,wbuf,wlen,off) ;
	    off += ps ;
	} /* end while */

	return rs ;
}
/* end subroutine (shmalloc) */


