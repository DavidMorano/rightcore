/* main */

/* program to test a PIPE communication mechanism */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	CF_CATCH	1
#define	CF_EARLY	1
#define	CF_SPECINT	1
#define	CF_SLEEP	0


/* revision history:

	= 88/01/01, David A­D­ Morano

	This subroutine (it's the whole program also)
	was originally written.


*/


/************************************************************************

	This program tests to see if we can communicate through
	a PIPE mechanism.

	Notes:

	Traditionally (long ago in the old days), of the two FDs
	returned from the 'pipe(2)' call, the first was the one that
	was for reading and the second was the one that was for
	writing.  Now-a-days, of course (and for almost two decades),
	both FDs are readable and writable.


***************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<sys/conf.h>
#include	<stropts.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<field.h>
#include	<bfile.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#ifndef	LINELEN
#define	LINELEN		2048		/* must be greater then 1024 */
#endif

#ifndef	BUFLEN
#define	BUFLEN		1024
#endif

#define	O_CLIFLAGS	(O_WRONLY | O_CREAT | O_NONBLOCK)
#define	O_SRVFLAGS	(O_RDONLY | O_CREAT | O_NONBLOCK)



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;

extern char	*strbasename(char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	readthem(), writethem() ;

static void	int_alarm(int) ;
static void	int_signal(int) ;
static void	int_pipe(int) ;
static void	int_piper(int,siginfo_t *,void *) ;


/* local variables */

static int	f_alarm ;
static int	f_signal ;
static int	f_pipe ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile	input, *ifp = &input ;
	bfile	output, *ofp = &output ;
	bfile	error, *efp = &error ;

	struct termios		ots, nts ;

	struct sigaction	sigs ;

	sigset_t		signalmask ;

	int	rs, len ;
	int	ex = EX_INFO ;
	int	i, j ;
	int	pipefds[2] ;
	int	fd_debug ;

	char	buffer[BUFLEN + 1], *bp ;
	char	tmpbuf[100] ;
	char	*progname ;
	char	*cp ;


	cp = getenv(VARDEBUGFD) ;

	if ((cp != NULL) && (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0664) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

	f_alarm = FALSE ;
	f_signal = FALSE ;
	f_pipe = FALSE ;



#if	CF_CATCH

/* set the signals that we want to catch */

	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = int_signal ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGTERM,&sigs,NULL) ;

	sigs.sa_handler = int_signal ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGHUP,&sigs,NULL) ;

	sigs.sa_handler = int_signal ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGINT,&sigs,NULL) ;

#if	CF_SPECINT

	(void) memset(&sigs,0,sizeof(struct sigaction)) ;

	sigs.sa_handler = int_pipe ;
	sigs.sa_sigaction = int_piper ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = SA_SIGINFO ;
	sigaction(SIGPIPE,&sigs,NULL) ;

#else

	sigs.sa_handler = (void (*)(int)) SIG_IGN ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGPIPE,&sigs,NULL) ;

#endif /* CF_SPECINT */

#endif /* CF_CATCH */


	u_pipe(pipefds) ;

	rs = uc_fork() ;

	if (rs > 0) {

/* parent */

		u_close(pipefds[0]) ;

		while ((rs = u_read(FD_STDIN,buffer,BUFLEN)) > 0) {

			len = rs ;
			rs = u_write(pipefds[1],buffer,len) ;

			if (rs < 0)
				bprintf(efp,"%s: write error (%d)\n",
					progname,rs) ;

#if	CF_SLEEP
			sleep(1) ;
#endif

			if (f_pipe) {

				f_pipe = FALSE ;
				bprintf(efp,"%s: received pipe signal\n",
					progname) ;

			}

			rs = u_read(pipefds[1],buffer,BUFLEN) ;

			if (rs > 0) {

			len = rs ;
			u_write(FD_STDOUT,buffer,len) ;

			}

		} /* end while */

	} else if (rs == 0) {

/* child */

#if	CF_DEBUGS
	    debugprintf("main: server reading\n") ;
#endif

		u_close(pipefds[1]) ;

		while ((rs = u_read(pipefds[0],buffer,BUFLEN)) > 0) {

			len = rs ;
			u_write(pipefds[0],buffer,len) ;

#if	CF_EARLY
			if (buffer[0] == 'd')
				break ;
#endif

		} /* end while */

		uc_exit(EX_OK) ;

	} /* end if (parent/child) */

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

done:
	bclose(efp) ;

	return ex ;

/* bad stuff */
badret:
	bclose(efp) ;

	return EX_DATAERR ;

badin:
	bprintf(efp,"%s: could not open input, rs=%d\n",
	    progname,rs) ;

	goto badret ;

badout:
	bprintf(efp,"%s: could not open output, rs=%d\n",
	    progname,rs) ;

	goto badret ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static void int_alarm(sig)
int	sig ;
{


	f_alarm = TRUE ;
}


static void int_signal(sig)
int	sig ;
{


	f_signal = TRUE ;
}


static void int_pipe(sig)
int	sig ;
{
	int	oen = errno ;


	f_pipe = TRUE ;
	errno = oen ;
}
/* end subroutine (int_pipe) */


static void int_piper(sn,sip,vp)
int		sn ;
siginfo_t	*sip ;
void		*vp ;
{
	int	oen = errno ;


	f_pipe = TRUE ;
	errno = oen ;
}
/* end subroutine (int_piper) */


/* the server reads */
static int readthem(name,fp)
char	name[] ;
bfile	*fp ;
{
	int	rs ;
	int	len ;
	int	fd ;
	int	oflags ;
	(O_RDONLY | O_NONBLOCK,0666) ;

	char	linebuf[LINELEN + 1] ;


#if	CF_DEBUGS
	debugprintf("readthem: name=%s\n",name) ;
#endif

/* make the FIFO and open it for reading in message discard mode */

	(void) uc_mkfifo(name,0666) ;

	oflags = (O_RDONLY | O_NONBLOCK) ;
	rs = u_open(name,oflags,0666)) < 0)
	fd = rs ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("readthem: open rs=%d\n",rs) ;
#endif

/* set message discard mode */

	rs = u_ioctl(fd,I_SRDOPT,RMSGD) ;

#if	CF_DEBUGS
	debugprintf("readthem: ioctl rs=%d\n",rs) ;
#endif

	while ((rs >= 0) && (! f_signal)) {

	    int	time = 0 ;

	    while (TRUE) {

	        len = u_read(fd,linebuf,LINELEN) ;

#if	CF_DEBUGS
	        if (len == 0)
	            debugprintf("readthem: no writers !!\n") ;
#endif

	        if ((len != SR_AGAIN) && (len != 0)) 
			break ;

	        if (f_signal) 
			break ;

#if	CF_SLEEP
	        sleep(1) ;
#endif

#if	CF_DEBUGS
	        time += 1 ;
	        if (time >= 4) {

	            time = 0 ;
	            debugprintf("readthem: looping\n") ;

	        }
#endif /* CF_DEBUGS */

	    } /* end while */

	    if (len < 0) break ;

#if	CF_DEBUGS
	    debugprintf("readthem: receive len=%d\n",len) ;
#endif

	    rs = bwrite(fp,linebuf,len) ;

#if	CF_DEBUGS
	    debugprintf("readthem: bwrite rs=%d\n",rs) ;
#endif

	} /* end while */

#if	CF_UNLINK
	u_unlink(name) ;
#endif

	u_close(fd) ;

	if (f_signal)
		len = SR_INTR ;

ret0:

#if	CF_DEBUGS
	debugprintf("readthem: ret rs=%d len=%d\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* wnd subroutine (readthem) */


/* the client writes */
static int writethem(name,fp)
char	name[] ;
bfile	*fp ;
{
	struct ustat	sb ;

	int	rs ;
	int	len ;
	int	fd ;

	char	linebuf[LINELEN + 1] ;


	rs = u_stat(name,&sb) ;

#if	CF_DEBUGS
	    debugprintf("writethem: stat rs=%d mode=%08o\n",rs,sb.st_mode) ;
#endif

	if (rs < 0) {

	    (void) uc_mkfifo(name,0666) ;

	} else if (! S_ISFIFO(sb.st_mode)) {

	    (void) u_unlink(name) ;

	    (void) uc_mkfifo(name,0666) ;

	}

	if ((rs = u_open(name,O_CLIFLAGS,0666)) < 0) {

#if	CF_DEBUGS
	    debugprintf("writethem: open rs=%d\n",rs) ;

	    if (rs == SR_NXIO)
	        debugprintf("writethem: no readers !\n") ;
#endif

	    return rs ;
	}

	fd = rs ;
	while (! f_signal) {

	    if ((len = breadline(fp,linebuf,LINELEN)) <= 0) 
		break ;

#if	CF_DEBUGS
	        debugprintf("writethem: breadline len=%d\n",len) ;
#endif

	    while (((rs = u_write(fd,linebuf,len)) < 0) &&
			((rs == SR_HANGUP) || (rs == SR_PIPE))) {

#if	CF_DEBUGS
	        debugprintf("writethem: write rs=%d\n",rs) ;
	        debugprintf("writethem: no readers !\n") ;
#endif



	        sleep(1) ;

	        if (f_signal) 
			break ;

	    } /* end while */

#if	CF_DEBUGS
	        debugprintf("writethem: bottom loop\n") ;
#endif

	} /* end while (reading lines) */

#if	CF_DEBUGS
	debugprintf("writethem: exiting, len=%d\n",len) ;
#endif

	u_close(fd) ;

	return len ;
}
/* wnd subroutine (writethem) */



