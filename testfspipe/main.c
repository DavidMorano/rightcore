/* main */

/* program to test out a FSPIPE communication mechanism */
/* last modified %G% version %I% */


#define	CF_DEBUGS	1


/* revision history:

	= 1988-01-01, David A­D­ Morano

	This subroutine (it's the whole program also)
	was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	This program tests out the FSPIPE communication mechanism.
	This is both a local and cross machine pipe-flavored mechanism
	that exists in the address space consisting of the machine's
	file systems.  Only machines that cross-mount file systems can
	communicate with each other through this mechanism since the
	address of the thing exists some place in the machine's file
	systems.


***************************************************************************/


#include	<envstandards.h>

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
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<field.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"fspipe.h"


/* local defines */

#define	LINELEN		2048		/* must be greater then 1024 */
#define	BUFLEN		1024

#define	O_CLIFLAGS	(O_WRONLY | O_CREAT | O_NONBLOCK)
#define	O_SRVFLAGS	(O_RDONLY | O_CREAT | O_NONBLOCK)


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;

extern char	*strbasename(char *) ;


/* external variables */


/* forward references */

static int	readthem(), writethem() ;

static void	int_alarm() ;
static void	int_signal() ;


/* gloabal variables */

static int	f_alarm ;
static int	f_signal ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct termios		ots, nts ;

	struct sigaction	sigs ;

	bfile	input, *ifp = &input ;
	bfile	output, *ofp = &output ;
	bfile	error, *efp = &error ;

	sigset_t	signalmask ;

	int	rs = SR_OK ;
	int	len ;
	int	i, j ;
	int	fd_debug = -1 ;

	char	tmpbuf[100] ;
	char	*progname ;
	char	*buf, *bp ;
	char	*cp ;


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

	sigs.sa_handler = (void (*)(int)) SIG_IGN ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGPIPE,&sigs,NULL) ;


#if	CF_DEBUGS
	    debugprintf("main: argc=%u\n",argc) ;
#endif

	if (argc == 2) {

#if	CF_DEBUGS
	    debugprintf("main: client writing\n") ;
#endif

	    if ((rs = bopen(ifp,BFILE_STDIN,"dr",0666)) < 0)
	        goto badin ;

	    rs = writethem(argv[1],ifp) ;

#if	CF_DEBUGS
	    debugprintf("main: writethem rs=%d\n",rs) ;
#endif

	    bclose(ifp) ;

	} else if (argc == 3) {

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
	    debugprintf("main: here is some test output !\n") ;

	    bprintf(ofp,"are we ready to play ?\n") ;

	    bflush(ofp) ;
#endif /* CF_DEBUGS */

	    rs = readthem(argv[1],ofp) ;

#if	CF_DEBUGS
	    debugprintf("main: readthem rs=%d\n",rs) ;
#endif

	    bclose(ofp) ;

	} /* end if (reader or writer) */


done:
	bclose(efp) ;

	return OK ;

badret:
	bclose(efp) ;

	return BAD ;

/* bad stuff */
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


/* the server reads */
static int readthem(name,fp)
char	name[] ;
bfile	*fp ;
{
	int	rs ;
	int	len ;
	int	fd ;

	char	linebuf[LINELEN + 1] ;


#if	CF_DEBUGS
	debugprintf("readthem: name=%s\n",name) ;
#endif

/* make the FIFO and open it for reading in message discard mode */

	uc_mkfifo(name,0666) ;

	rs = u_open(name,(O_RDONLY | O_NONBLOCK),0666) ;

#if	CF_DEBUGS
	debugprintf("readthem: open() rs=%d\n",rs) ;
#endif

	fd = rs ;
	if (rs < 0)
		return rs ;

/* set message discard mode */

	rs = u_ioctl(fd,I_SRDOPT,RMSGD) ;

#if	CF_DEBUGS
	debugprintf("readthem: ioctl rs=%d\n",rs) ;
#endif

	while (! f_signal) {

	    int	time ;


	    time = 0 ;
	    while (TRUE) {

	        len = u_read(fd,linebuf,LINELEN) ;

#if	CF_DEBUGS
	        if (len == 0)
	            debugprintf("readthem: no writers !!\n") ;
#endif

	        if ((len != SR_AGAIN) && (len != 0)) break ;

	        if (f_signal) break ;

	        sleep(1) ;

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


/* the client writes */
static int writethem(name,fp)
char	name[] ;
bfile	*fp ;
{
	FSPIPE		p ;

	struct ustat	sb ;

	int	rs ;
	int	len ;
	int	fd ;

	char	linebuf[LINELEN + 1] ;


	rs = fspipe_open(&p,name) ;

#if	CF_DEBUGS
	    debugprintf("writethem: fspipe_open() rs=%d\n",rs) ;
#endif

	if (rs < 0) {

#if	CF_DEBUGS
	    debugprintf("writethem: fspipe_open rs=%d\n",rs) ;

	    if (rs == SR_NXIO)
	        debugprintf("writethem: no readers !\n") ;
#endif

	    return rs ;
	}

	fd = rs ;
	while (! f_signal) {

	    if ((len = fspipe_read(&p,linebuf,LINELEN)) <= 0) 
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

	if (len < 0)
		rs = len ;

	fspipe_close(&p) ;

	return rs ;
}
/* wnd subroutine (writethem) */



