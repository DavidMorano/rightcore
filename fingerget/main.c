/* main */

/* generic tiny front-end */


#define	CF_DEBUGS	0		/* compile-time */



#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>

#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN	(4 * 1024)
#endif



/* external subroutines */

extern int	dialuss(char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;

extern char	*strwcpy(char *,const char *,int) ;






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	int	rs ;
	int	fd ;
	int	len ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;

	char	buf[BUFLEN + 1], *bp ;
	char	*cp ;


	cp = getenv(VARDEBUGFD1) ;

	if (cp == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if (cp == NULL)
	    cp = getenv(VARDEBUGFD3) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	if (argc < 3)
		return EX_USAGE ;

	if ((len = strlen(argv[2])) == 0)
		return EX_USAGE ;

#if	CF_DEBUGS
	debugprintf("main: past arguments uss=%s\n",argv[1]) ;
#endif

	rs = dialuss(argv[1],10,0) ;

#if	CF_DEBUGS
	debugprintf("main: dialuss() rs=%d\n",rs) ;
#endif

	if (rs < 0)
		return EX_TEMPFAIL ;

	fd = rs ;

#if	CF_DEBUGS
	debugprintf("main: opened USS fd=%d\n",fd) ;
#endif

	bp = strwcpy(buf,argv[2],(BUFLEN - 1)) ;

	*bp++ = '\n' ;
	*bp = '\0' ;

#if	CF_DEBUGS
	debugprintf("main: formed query\n") ;
#endif

	rs = u_write(fd,buf,(bp - buf)) ;

	if (rs < 0) {

		ex = EX_IOERR ;
		goto bad3 ;
	}

#if	CF_DEBUGS
	debugprintf("main: wrote query\n") ;
#endif

	while ((rs = u_read(fd,buf,BUFLEN)) > 0) {

		len = rs ;
		rs = u_write(FD_STDOUT,buf,len) ;

		if (rs < 0)
			break ;

	} /* end while */

#if	CF_DEBUGS
	debugprintf("main: ret rs=%d\n",rs) ;
#endif

	ex = (rs >= 0) ? EX_OK : EX_IOERR ;

bad3:
bad2:
	u_close(fd) ;

	return ex ;
}
/* end subroutine (main) */



