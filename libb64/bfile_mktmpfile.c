/* bfile_mktmpfile */

/* BASIC INPUT OUTPUT package */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#define	BFILE_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<time.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#define	O_MODE		(O_CREAT | O_EXCL | O_RDWR)
#define	MAXLOOP		100

#define	PIDBUFLEN	64


/* external subroutines */

extern int	bufprintf(char *,int,const char *,...) ;

#if	CF_DEBUGS
extern int	debugprintf() ;
#endif


/* external variables */


/* forward reference */


/* exported subroutines */


int bfile_mktmpfile(inname,mode,outname)
char	inname[] ;
int	mode ;
char	outname[] ;
{
	pid_t	pid = getpid() ;

	time_t	daytime = time(NULL) ;

	long	rv ;

	int	rs = SR_OK ;
	int	fd = -1 ;
	int	loop = 0 ;
	int	rlen ;
	int	f_exit = FALSE ;

	char	namebuf[MAXPATHLEN + 1], *nbp ;
	char	pidbuf[PIDBUFLEN + 1], *pbp ;
	char	*ibp, *obp ;


	obp = namebuf ;
	if (outname != NULL) 
		obp = outname ;

	*obp = '\0' ;

	while ((loop < MAXLOOP) && (! f_exit)) {

#if	CF_DEBUGS
	    debugprintf("bfile_mktmpfile: top of while\n") ;
#endif

	    f_exit = TRUE ;
	    rs = BAD ;
	    rv = (pid + loop) ^ (daytime & 0xFFFF) ;
	    rlen = bufprintf(pidbuf,PIDBUFLEN,"%08X%08X%08X%08X",
	        rv,rv,rv,rv) ;

	    ibp = inname ;
	    nbp = obp ;
	    pbp = pidbuf + rlen - 1 ;
	    while (*ibp && (pbp >= pidbuf)) {

	        if (*ibp == 'X') {
			*nbp++ = *pbp-- ;
	        } else 
			*nbp++ = *ibp ;

	        ibp += 1 ;

	    } /* end while */

	    *nbp = '\0' ;

/* make the thing */

	    if (S_ISDIR(mode)) {

		rs = u_mkdir(obp,mode) ;

		if ((rs == SR_EXIST) || (rs == SR_INTR)) {
	                f_exit = FALSE ;
	        } else
	            rs = OK ;

	    } else if (S_ISREG(mode)) {

		rs = u_open(obp,O_MODE,mode) ;
		fd = rs ;
		if ((rs == SR_EXIST) || (rs == SR_INTR)) {
	                f_exit = FALSE ;

	        } else {

	            rs = OK ;
		    if (fd >= 0) {
	                u_close(fd) ;
			fd = -1 ;
		    }

	        } /* end if */

	    } else if (S_ISFIFO(mode)) {

#if	CF_DEBUGS
	        debugprintf("bfile_mktmpfile: got a FIFO\n") ;
#endif

	        if ((rs = mknod(obp,0010600 | (mode & 0777),0)) < 0) {

	            if ((rs == SR_EXIST) || (rs == SR_INTR))
	                f_exit = FALSE ;

#if	CF_DEBUGS
	            debugprintf("bfile_mktmpfile: bad rs=%d\n",rs) ;
#endif

	        } else
	            rs = OK ;

	    } else if (S_ISSOCK(mode)) {

	        return SR_NOTSUP ;

	    } else 
		return SR_INVALID ;

	    loop += 1 ;

	} /* end while */

	return ((loop >= MAXLOOP) ? SR_RANGE : rs) ;
}
/* end subroutine (bfile_mktmpfile) */



