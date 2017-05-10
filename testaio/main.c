/* main (testaio) */

/* test front-end */


#define	CF_DEBUGS	1
#define	CF_DEBUG	1
#define	CF_WAIT		0
#define	CF_SUSPEND	0		/* syspend to wait for completion */
#define	CF_STDIN	1		/* use STDIN */


/* revision history:

	= 1988-02-01, David A­D­ Morano

	This was started.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	This is a test program for the AIO package.


*************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<aio.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NPARG		2

#ifndef	BUFLEN
#define	BUFLEN		(8 * 1024)
#endif

#define	HEREFNAME	"here"


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;


/* external variables */


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct aiocb	ab ;
	struct aiocb	*alist[3] ;

	bfile	errfile, *efp = &errfile ;

	int	rs = SR_OK ;
	int	len ;
	int	pan ;
	int	argl, aol ;
	int	ex = EX_INFO ;
	int	sfd, dfd ;
	int	i ;
	int	fd_debug = -1 ;
	int	f_usage = FALSE ;

	const char	*progname, *argp, *aop ;
	const char	*cp ;

	char	*buf ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
		cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


#if	CF_DEBUGS
	debugprint("main: entered\n") ;
		debugprintf("main: about to open error\n") ;
#endif

	sfbasename(argv[0],-1,&progname) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;



	buf = NULL ;

	if ((rs = uc_valloc(BUFLEN,&buf)) < 0)
		goto badalloc ;


#if	CF_STDIN
	sfd = FD_STDIN ;
#else
	rs = u_open(HEREFNAME,O_RDONLY,0666) ;
	sfd = rs ;
	if (rs < 0)
		goto badopen ;
#endif /* CF_STDIN */

#if	CF_DEBUGS
	debugprintf("main: u_open() rs=%d\n",rs) ;
#endif

/* setup the read */

	memset(&ab,0,sizeof(struct aiocb)) ;
	ab.aio_fildes = sfd ;
	ab.aio_offset = 0 ;
	ab.aio_buf = buf ;
	ab.aio_nbytes = BUFLEN ;

/* issue the read */

	rs = uc_aioread(&ab) ;

#if	CF_DEBUGS
	debugprintf("main: uc_aioread() rs=%d\n",rs) ;
#endif

#if	CF_SUSPEND

	alist[0] = &ab ;
	alist[1] = NULL ;
	rs = uc_aiosuspend(alist,1,NULL) ;

#if	CF_DEBUGS
	debugprintf("main: uc_aiosuspend() rs=%d\n",rs) ;
#endif

	if (rs >= 0)
		rs = uc_aioreturn(&ab) ;

#else /* CF_SUSPEND */

#if	CF_WAIT
	sleep(2) ;
#endif

	rs = SR_INPROGRESS ;
	for (i = 0 ; rs == SR_INPROGRESS ; i += 1) {

	    rs = uc_aioreturn(&ab) ;
	    len = rs ;

#if	CF_DEBUGS
	debugprintf("main: uc_aioreturn() rs=%d\n",rs) ;
#endif

	    if (rs != SR_INPROGRESS)
		break ;

	    if ((i % 3) == 1)
		bprintf(efp,"%s: in progress %u\n",progname,i) ;

	    sleep(1) ;

	} /* end for */

#endif /* CF_SUSPEND */

	bprintf(efp,"%s: read len=%d\n", progname,len) ;

	rs = u_write(FD_STDOUT,buf,len) ;


	u_close(sfd) ;


	ex = EX_OK ;

done:
ret2:
	if (buf != NULL)
		free(buf) ;

ret1:
retearly:
	bclose(efp) ;

ret0:
	return ex ;

/* some help stuff */
usage:
	bprintf(efp,
	    "usage: %s \n",
	    progname) ;

	goto retearly ;

badnotenough:
	bprintf(efp,"%s: not enough arguments supplied\n",
	    progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	bprintf(efp,"%s: bad argument value specified\n",
	    progname) ;

	goto ret2 ;

badalloc:
	goto badret ;

badopen:
	bprintf(efp,"%s: could not open (%d)\n",
	    progname,rs) ;
	goto badret ;

badret:
	ex = EX_DATAERR ;
	goto ret2 ;
}
/* end subroutine (main) */



