/* main */

/* program to test a FIFO communication mechanism */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1988-01-01, David A­D­ Morano

        This subroutine (it's the whole program also) was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This program tests to see if we can communicate through a FIFO file in
        the filesystem!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

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

#include	<vsystem.h>
#include	<field.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"sigdumpmsg.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048		/* must be greater then 1024 */
#endif

#ifndef	BUFLEN
#define	BUFLEN		1024
#endif

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#define	O_CLIFLAGS	(O_WRONLY | O_CREAT | O_NONBLOCK)
#define	O_SRVFLAGS	(O_RDONLY | O_CREAT | O_NONBLOCK)


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;

extern char	*strbasename(char *) ;


/* external variables */


/* forward references */

static int	readthem() ;
static int	process_request(bfile *,struct sigdumpmsg_request *) ;

void		int_alarm() ;
void		int_signal() ;


/* gloabal variables */

static int		f_alarm ;
static int		f_signal ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	struct termios		ots, nts ;
	struct sigaction	sigs ;
	bfile		input, *ifp = &input ;
	bfile		output, *ofp = &output ;
	bfile		error, *efp = &error ;

	sigset_t	signalmask ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	ex = EX_INFO ;
	int	rs = SR_OK ;
	int	rs1 ;
	int	len ;
	int	i, j ;
	int	fd_debug = -1 ;

	cchar	*progname ;
	char	*buf, *bp ;
	cchar	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	if ((cp = getenv(VARDEBUGFD1)) == NULL)
		cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0664) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

	f_signal = FALSE ;
	f_alarm = FALSE ;


	if (argc < 2) {

		goto badin ;
	}


/* set the signals that we want to catch */

	(void) memset(&sigs,0,sizeof(struct sigaction)) ;

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

	sigs.sa_handler = (void (*)(int)) SIG_IGN ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGPIPE,&sigs,NULL) ;


#if	CF_DEBUGS
	    debugprintf("main: server reading\n") ;
#endif

#ifdef	COMMENT
	    if ((rs = bopen(ofp,"file.out","wct",0666)) < 0)
	        goto badout ;
#else
	    if ((rs = bopen(ofp,BFILE_STDOUT,"dwct",0666)) < 0)
	        goto badout ;
#endif

	    bcontrol(ofp,BC_UNBUF,0) ;

#ifdef	COMMENT
	    nprintf("d","main: bio stat=%04X\n",ofp->stat) ;
#endif

#if	CF_DEBUGS
	    debugprintf("main: here is some test output!\n") ;

	    bprintf(ofp,"are we ready to play?\n") ;

	    bflush(ofp) ;
#endif /* CF_DEBUGS */

	    rs = readthem(argv[1],ofp) ;

#if	CF_DEBUGS
	    debugprintf("main: readthem rs=%d\n",rs) ;
#endif

	    bclose(ofp) ;

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;


done:
retearly:
ret1:
	bclose(efp) ;

ret0:
	return ex ;

badin:
	bprintf(efp,"%s: could not open input, rs=%d\n",
	    progname,rs) ;

	goto retearly ;

badout:
	bprintf(efp,"%s: could not open output, rs=%d\n",
	    progname,rs) ;

	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


void int_alarm(sig)
int	sig ;
{

	f_alarm = TRUE ;
}


void int_signal(sig)
int	sig ;
{

	f_signal = TRUE ;
}


/* the server reads */
static int readthem(name,fp)
char	name[] ;
bfile	*fp ;
{
	struct ustat	sb ;

	struct sigdumpmsg_request	m0 ;

	int	rs, len, ml ;
	int	fd ;

	char	msgbuf[MSGBUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("readthem: name=%s\n",name) ;
#endif

/* make the FIFO and open it for reading in message discard mode */

	if ((u_stat(name,&sb) < 0) || (! S_ISFIFO(sb.st_mode))) {

		(void) u_unlink(name) ;

		(void) uc_mkfifo(name,0666) ;

	}

	rs = u_open(name,O_RDONLY | O_NONBLOCK,0666) ;
	fd = rs ;
	if (rs < 0)
	    return rs ;

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

	        len = u_read(fd,msgbuf,MSGBUFLEN) ;

#if	CF_DEBUGS
	        if (len == 0)
	            debugprintf("readthem: no writers!!\n") ;
#endif

	        if ((len != SR_AGAIN) && (len != 0)) 
			break ;

	        if (f_signal) 
			break ;

	        sleep(1) ;

#if	CF_DEBUGS
	        time += 1 ;
	        if (time >= 4) {

	            time = 0 ;
	            debugprintf("readthem: looping\n") ;

	        }
#endif /* CF_DEBUGS */

	    } /* end while */

	    if (len < 0) 
		break ;

#if	CF_DEBUGS
	    debugprintf("readthem: receive len=%d\n",len) ;
#endif

/* process this message */

		ml = sigdumpmsg_request(&m0,1,msgbuf,len) ;

#if	CF_DEBUGS
	    debugprintf("readthem: sigdumpmsg_request() rs=%d\n",mls) ;
#endif

		if (ml > 0) {

			rs = process_request(fp,&m0) ;

		}

	} /* end while */

#if	F_UNLINK
	u_unlink(name) ;
#endif

	u_close(fd) ;

	if (f_signal)
		len = SR_INTR ;

#if	CF_DEBUGS
	debugprintf("readthem: exiting len=%d\n",len) ;
#endif

	return len ;
}
/* wnd subroutine (readthem) */


static int process_request(ofp,mp)
bfile				*ofp ;
struct sigdumpmsg_request	*mp ;
{


	sigdump(mp->pid,mp->fname) ;

	bprintf(ofp,"pid=%d tag=%d fname=%s\n",
		mp->pid,mp->tag,mp->fname) ;

	return mp->pid ;
}
/* wnd subroutine (process_request) */


