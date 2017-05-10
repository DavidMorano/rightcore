/* main */


#define	CF_SOCKETPAIR	0
#define	CF_SLEEP	0
#define	CF_WRITE	1	
#define	CF_ECHOD	0
#define	CF_MSGMODE	1
#define	CF_REPORT	1



#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/conf.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stropts.h>
#include	<stdio.h>

#include	<vsystem.h>

#include	"libff.h"
#include	"defs.h"



/* local defines */

#define	LOCALMNT	"mntpoint"

#ifndef	BUFLEN
#define	BUFLEN		1024
#endif





int main()
{
	struct ustat	sb ;

	int	rs ;
	int	pipes[2] ;
	int	sfd, cfd ;
	int	len ;
	int	i ;
	int	cl ;

	char	buf[BUFLEN + 1] ;
	char	*cp ;


	fprintf(stderr,"pid=%d\n",getpid()) ;

#if	CF_SOCKETPAIR
	u_socketpair(PF_UNIX,SOCK_STREAM,0,pipes) ;
#else
	u_pipe(pipes) ;

#if	CF_MSGMODE
	{
		int	flags ;


		flags = 0 ;
		flags |= RMSGD ;
		u_ioctl(pipes[0],I_SRDOPT,flags) ;

	}
#endif /* CF_MSGMODE */

#endif

	cfd = pipes[0] ;
	sfd = pipes[1] ;

	rs = u_access(LOCALMNT,W_OK) ;

	if (rs == SR_NOENT)
	    rs = u_creat(LOCALMNT,0666) ;

	if (rs >= 0) {

	    rs = uc_fattach(cfd,LOCALMNT) ;

	    fprintf(stderr,"mountpoint=%s rs=%d\n",LOCALMNT,rs) ;

	    u_fstat(cfd,&sb) ;

	    fprintf(stderr,"dev=%08lx rdev=%08lx ino=%ld\n",
	        sb.st_dev,sb.st_rdev,sb.st_ino) ;

		if (S_ISFIFO(sb.st_mode))
		fprintf(stderr," FIFO") ;

		if (S_ISSOCK(sb.st_mode))
		fprintf(stderr," SOCK") ;

		if (S_ISCHR(sb.st_mode))
		fprintf(stderr," CHR") ;

		fprintf(stderr,"\n") ;

	    fprintf(stderr,"free end fd=%d\n",pipes[0]) ;

	    u_fstat(sfd,&sb) ;

	    fprintf(stderr,"dev=%08lx rdev=%08lx ino=%ld\n",
	        sb.st_dev,sb.st_rdev,sb.st_ino) ;


	    if (rs >= 0) {

#if	CF_SLEEP
	        sleep(60) ;
#else

#if	CF_ECHOD

	        while ((rs = u_read(sfd,buf,BUFLEN)) > 0) {

	            len = rs ;

#if	CF_REPORT
	            ffprintf(stdout,"len=%u\n",len) ;
#endif

	            rs = u_write(sfd,buf,len) ;

#if	CF_REPORT
		if (rs >= 0)
	            ffwrite(stdout,buf,len) ;
#endif

		if (rs < 0)
			break ;

	        } /* end while */

#else /* CF_ECHOD */

#if	CF_WRITE

	for (i = 0 ; (rs >= 0) && (i < 20) ; i += 1) {

	        cp = "hello from inside the matrix!\n" ;
	        cl = strlen(cp) ;

	        rs = u_write(sfd,cp,cl) ;

	        sleep(2) ;

	} /* end for */

#else /* CF_WRITE */

	        while ((rs = u_read(sfd,buf,BUFLEN)) > 0) {

	            len = rs ;
	            ffwrite(stdout,buf,len) ;

	        } /* end while */

#endif /* CF_WRITE */

#endif

#endif /* CF_SLEEP */

	    } /* end if (successful attach) */

	    rs = uc_fdetach(LOCALMNT) ;

	} /* end if (mount point exists) */

	ffclose(stdout) ;

	fclose(stderr) ;

	return 0 ;
}
/* end subroutine (main) */



