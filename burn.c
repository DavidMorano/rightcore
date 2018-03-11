/* burn */

/* burn a file */


#define	CF_DEBUGS 	0		/* non-printable debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/***********************************************************************

	This subroutine is used to "burn" (shred) the file
	given.

	Synopsis:

	int burn(rvp,bcount,name)
	randomvar	*rvp ;
	int		bcount ;
	const char	name[] ;

	Arguments:

	name		directory entry
	rvp		RANDOMVAR object pointer
	bcount	number of over-writes

	Returns:

	>0		skip this directory entry in directory walk
	0		continue with directory walk as usual
	<0		exit directory walk altogether


***********************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<randomvar.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 10),2048)
#endif

#define	NPAGES		16		/* number of pages to write */


/* external subroutines */

extern int	hashelf(const void *,int) ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int burn(rvp,bcount,name)
randomvar	*rvp ;
int		bcount ;
const char	name[] ;
{
	struct ustat	sb ;

	randomvar	x ;

	ULONG	*buf = NULL ;

	int	rs ;
	int	rs1 ;
	int	i, j ;
	int	fd ;
	int	pagesize = getpagesize() ;
	int	bufsize ;
	int	tlen = 0 ;
	int	f_rv = FALSE ;


#if	CF_DEBUGS
	debugprintf("burn: ent %s\n",name) ;
#endif

	if (bcount < 1)
	    bcount = 1 ;

	rs = uc_open(name,O_WRONLY,0666) ;
	fd = rs ;
	if (rs < 0)
	    goto ret0 ;

	rs = u_fstat(fd,&sb) ;
	if (rs < 0)
	    goto ret1 ;

	if ((! S_ISREG(sb.st_mode)) || (sb.st_size == 0)) {
	    rs = SR_OK ;
	    goto ret1 ;
	}

#if	CF_DEBUGS
	debugprintf("burn: file size=%ld\n",sb.st_size) ;
#endif

	if (rvp == NULL) {
	    uint	h = hashelf(name,-1) ;

	    rs = randomvar_start(&x,FALSE,h) ;
	    if (rs >= 0) f_rv = TRUE ;
	    rvp = &x ;
	    if (rs < 0) goto ret1 ;

	} /* end if */

/* do it */

	bufsize = NPAGES * pagesize ;
	if ((rs = uc_valloc(bufsize,&buf)) >= 0) {

	    for (i = 0 ; (rs >= 0) && (i < bcount) ; i += 1) {
	        int	wlen ;

#if	CF_DEBUGS
	        debugprintf("burn: popping %d\n",i) ;
#endif

	        if (bcount > 1) {
	            u_seek(fd,0L,SEEK_SET) ;
	            uc_fdatasync(fd) ;
	        }

	        tlen = 0 ;
	        while ((rs >= 0) && (tlen < sb.st_size)) {

	            wlen = MIN((sb.st_size - tlen),bufsize) ;

	            wlen = (wlen + (pagesize - 1)) & (~ (pagesize - 1)) ;

	            for (j = 0 ; j < (wlen / sizeof(ULONG)) ; j += 1)
	                randomvar_getulong(rvp,buf + j) ;

	            rs = u_write(fd,buf,wlen) ;
	            tlen += bufsize ;

	        } /* end while */

	    } /* end while (burn count) */

	    uc_free(buf) ;
	} /* end if (memory allocation) */

	if (f_rv) {
	    rs1 = randomvar_finish(&x) ;
	    if (rs >= 0) rs = rs1 ;
	}

ret1:
	u_close(fd) ;

#if	CF_DEBUGS && 0
	if (sb.st_size > 0) {
	    char	cmd[MAXPATHLEN + 1] ;
	    debugprintf("burn: ret\n") ;
	    bufprintf(cmd,MAXPATHLEN,"cp %s /tmp/%s",
	        name,strbasename(name)) ;
	    system(cmd) ;
	}
#endif /* CF_DEBUGS */

ret0:
	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (burn) */



