/* openshmtmp */

/* open a temproary file in shared memory (really private) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates and opens a POSIX® shared-memory temporary
	object.  The POSIX® SHM-type name of the object created is returned to
	the caller.

	Synopsis:

	int openshmtmp(rbuf,rlen,operm)
	char		rbuf[] ;
	int		rlen ;
	mode_t		operm ;

	Arguments:

	rbuf		suppled buffer to receive the resulting SHM name
	rlen		length of suppled buffer; should be at least SHMNAMELEN
	operm		SHM object creation opermissions

	Returns:

	>=0		file descriptor to program STDIN and STDOUT
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ugetpid.h>
#include	<cthex.h>
#include	<sigblock.h>
#include	<localmisc.h>


/* local defines */

#ifndef	SHMNAMELEN
#define	SHMNAMELEN	MAXNAMELEN
#endif

#define	EBUFLEN		(2*sizeof(ULONG))

#define	NTRIES		1000

#define	NDF		"openshmtmp.deb"


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	vstrkeycmp(const void **,const void **) ;
extern int	strkeycmp(const char *,const char *) ;

#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif


/* external variables */


/* forward reference */

static int	randinit(ULONG *) ;
static int	mkshmname(char *,int,ULONG) ;


/* local variables */


/* exported subroutines */


int openshmtmp(char *rbuf,int rlen,mode_t om)
{
	int		rs = SR_OK ;
	int		fd_shm = -1 ;
	int		f_bufalloc = FALSE ;

	if ((rbuf != NULL) && (rlen >= 0) && (rlen < SHMNAMELEN))
	    return SR_OVERFLOW ;

#if	CF_DEBUGN
	nprintf(NDF,"openshmtmp: ent\n") ;
	if (rbuf != NULL) {
	    nprintf(NDF,"openshmtmp: rbuf=%s\n",rbuf) ;
	}
#endif

	if (rbuf == NULL) {
	    rlen = (SHMNAMELEN + 1) ;
	    if ((rs = uc_malloc(rlen,&rbuf)) >= 0) {
	    	f_bufalloc = TRUE ;
	    }
	}

	if (rs >= 0) {
	    ULONG	rv ;
	    if ((rs = randinit(&rv)) >= 0) {
	        SIGBLOCK	b ;
	        if ((rs = sigblock_start(&b,NULL)) >= 0) {
	            const int	oflags = (O_CREAT | O_EXCL | O_RDWR) ;
	            const int	ntries = NTRIES ;
		    int		i ;

	            for (i = 0 ; i < ntries ; i += 1) {
	                if ((rs = mkshmname(rbuf,rlen,rv)) >= 0) {
	                    rs = uc_openshm(rbuf,oflags,om) ;
	                    fd_shm = rs ;
	                }
		        rv += 1 ;
		        if (rs != SR_EXISTS) break ;
	            } /* end for */

#if	CF_DEBUGN
		    nprintf(NDF,"openshmtmp: loop-out rs=%d\n",rs) ;
#endif

	            if ((rs >= 0) && f_bufalloc) {
		        uc_unlinkshm(rbuf) ;
	 	    }

	            sigblock_finish(&b) ;
	        } /* end if (sigblock) */
	    } /* end if (randinit) */
	} /* end if (ok) */

	if (f_bufalloc && (rbuf != NULL)) {
	    uc_free(rbuf) ;
	} /* end if (buffer was not supplied by the caller) */

#if	CF_DEBUGN
	nprintf(NDF,"openshmtmp: ret rs=%d fd=%d\n",rs,fd_shm) ;
#endif

	return (rs >= 0) ? fd_shm : rs ;
}
/* end subroutine (openshmtmp) */


/* local subroutines */


static int randinit(ULONG *rvp)
{
	const pid_t	pid = ugetpid() ;
	const time_t	dt = time(NULL) ;
	ULONG		rv = 0 ;
	ULONG		sv ;
	sv = (ULONG) dt ;
	rv += (sv << 8) ;
	sv = (ULONG) pid ;
	rv += sv ;
	*rvp = rv ;
	return SR_OK ;
}
/* end subroutine (randinit) */


static int mkshmname(char *rbuf,int rlen,ULONG rv)
{
	const int	elen = EBUFLEN ;
	int		rs ;
	char		ebuf[EBUFLEN+1] ;

	if ((rs = cthexull(ebuf,elen,rv)) >= 0) {
	    int	i = (rs >= 16) ? ((rs-16)+6) : 0 ;
	    rs = sncpy2(rbuf,rlen,"/tmp",(ebuf+i)) ;
	}

	return rs ;
}
/* end subroutine (mkshmname) */


