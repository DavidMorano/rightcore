/* main */


#define	CF_SLEEP	0		/* sleep while holding the lock */
#define	CF_LOCKF	1		/* do the locking */
#define	CF_UNLOCKF	0		/* unlock */
#define	CF_OUTPUT	0



#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<exitcodes.h>

#include	"localmisc.h"



/* local defines */

#define	SLEEPTIME	(5 * 60)






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	pid_t	pid ;

	int	rs ;
	int	fd ;
	int	oflags ;

	char	*fname ;


	if (argc < 2)
	    goto bad1 ;

	fname = argv[1] ;
	if ((argv[1] == NULL) || (argv[1][0] == '\0'))
	    goto bad1 ;

#if	CF_OUTPUT
	fprintf(stdout,"file=%s\n",fname) ;
#endif

	oflags = O_RDWR | O_CREAT ;
	rs = u_open(fname,oflags,0666) ;

	fd = rs ;
	if (rs < 0)
	    goto bad2 ;

#if	CF_OUTPUT
	fprintf(stdout,"locking\n") ;

	fflush(stdout) ;
#endif

#if	CF_LOCKF
	rs = uc_lockf(fd,F_LOCK,0L) ;

	if (rs < 0)
	    goto bad3 ;
#endif /* CF_LOCKF */

#if	CF_OUTPUT
	fprintf(stdout,"locked\n") ;

	fflush(stdout) ;
#endif

#if	CF_SLEEP
	sleep(SLEEPTIME) ;
#endif

	rs = uc_fork() ;
	pid = rs ;

#if	CF_OUTPUT
	fprintf(stdout,"fork=%d\n",rs) ;
	fflush(stdout) ;
#endif

	if (pid == 0) {

#if	CF_OUTPUT
	    fprintf(stdout,"child\n") ;

	    fflush(stdout) ;
#endif

	    sleep(SLEEPTIME) ;

	    uc_exit(EX_OK) ;

	} /* end if (child) */

#if	CF_OUTPUT
	fprintf(stdout,"parent\n") ;

	fflush(stdout) ;
#endif

#if	CF_LOCKF && CF_UNLOCKF
	rs = uc_lockf(fd,F_ULOCK,0L) ;
#endif

	u_close(fd) ;

ret1:
	fclose(stdout) ;

	fclose(stderr) ;

ret0:
	return EX_OK ;

/* bad stuff comes down here */
bad1:
	fprintf(stderr,"need a filename argument to be specified\n") ;

	goto ret1 ;

bad2:
	fprintf(stderr,"could not open the file (%d)\n",rs) ;

	goto ret1 ;

bad3:
	fprintf(stderr,"could not lock the file (%d)\n",rs) ;

	goto ret1 ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



