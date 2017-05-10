/* main */

/* program to test out the Posix Message Queue (FMQ) API */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_UNLINK	0		/* unlink file when done */
#define	CF_RECVDELAY	0
#define	CF_SLOWPOLL	1
#define	CF_SMALLBUF	0


/* revision history:

	= 88/01/01, David A­D­ Morano

	This subroutine (it's the whole program -- same as
	the FIFO test) was originally written.


*/


/************************************************************************

	This program tests the Posix Message Queue (FMQ) mechanism
	that exists in newer UNIX-type (or other) OSes that have
	this newer POSIX feature.  Most all versions of Solaris
	before release 2.5.1 do NOT support Posix Message Queues!


***************************************************************************/


#include	<sys/types.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<termios.h>
#include	<signal.h>

#include	<vsystem.h>
#include	<field.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"fmq.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048		/* must be greater then 1024 */
#endif

#ifndef	BUFLEN
#define	BUFLEN		(100 * 1024)
#endif

#if	CF_DEBUGS
#define	O_CFLAGS	(O_RDWR )
#define	O_SRVFLAGS	(O_RDWR | O_NONBLOCK | O_CREAT)
#else
#define	O_CFLAGS	(O_RDWR )
#define	O_SRVFLAGS	(O_RDWR | O_NONBLOCK | O_CREAT | O_NONBLOCK)
#endif

#define	POLLINT		1

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	msleep(uint) ;

extern char	*strbasename(char *) ;


/* external variables */


/* forward references */

static int	readthem(), writethem() ;

static int	debugprintmask(const char *) ;

static void	int_alarm() ;
static void	int_signal() ;


/* gloabal variables */

static int	f_alarm = FALSE ;
static int	f_signal = FALSE ;






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct sigaction	sigs ;

	bfile	input, *ifp = &input ;
	bfile	output, *ofp = &output ;
	bfile	error, *efp = &error ;

	sigset_t	signalmask ;

	int	rs, i, j ;
	int	len ;
	int	ex = EX_OK ;

	char	tmpbuf[TIMEBUFLEN + 1] ;
	char	*progname ;
	char	*ifname = NULL ;
	char	*ofname = "o" ;
	char	*buf, *bp ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

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


	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = int_signal ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGHUP,&sigs,NULL) ;


	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = int_signal ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGINT,&sigs,NULL) ;



	if (argc == 2) {

#if	CF_DEBUGS
	    debugprintf("main: client writing\n") ;
#endif

	    if ((ifname != NULL) && (ifname[0] != '\0'))
	        rs = bopen(ifp,ifname,"r",0666) ;

	    else
	        rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	    if (rs < 0)
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

	    if ((ofname != NULL) && (ofname[0] != '\0'))
	        rs = bopen(ofp,ofname,"wct",0666) ;

	    else
	        rs = bopen(ofp,BFILE_STDOUT,"wct",0666) ;

	    if (rs < 0)
	        goto badout ;

	    bcontrol(ofp,BC_UNBUF,0) ;

#ifdef	COMMENT
	    nprintf("d","main: bio stat=%04X\n",ofp->stat) ;
#endif

#if	CF_DEBUGS
	    debugprintf("main: here is some test output!\n") ;
#endif /* CF_DEBUGS */

	    bflush(ofp) ;

	    rs = readthem(argv[1],ofp) ;

#if	CF_DEBUGS
	    debugprintf("main: readthem rs=%d\n",rs) ;
#endif

	    bclose(ofp) ;

	} /* end if (reading) */

done:
ret0:
	bclose(efp) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

badret:
	ex = EX_DATAERR ;
	goto ret0 ;

badin:
	ex = EX_NOINPUT ;
	bprintf(efp,"%s: could not open input (%d)\n",
	    progname,rs) ;

	goto ret0 ;

badout:
	ex = EX_CANTCREAT ;
	bprintf(efp,"%s: could not open output (%d)\n",
	    progname,rs) ;

	goto ret0 ;

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


static int readthem(name,fp)
const char	name[] ;
bfile		*fp ;
{
	FMQ	q ;

	int	rs, len ;
	int	mode ;
	int	bufsize ;
	int	to, opts ;

	char	linebuf[LINEBUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("readthem: name=%s\n",name) ;
#endif

/* make the message queue accessible to everyone! */

#if	CF_SMALLBUF
	bufsize = 16 ;
#else
	bufsize = BUFLEN ;
#endif

	mode = umask(0000) ;

	rs = fmq_open(&q,name,O_SRVFLAGS,0666,bufsize) ;

#if	CF_DEBUGS
	debugprintmask("main: ") ;
#endif

	umask(mode) ;

	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("readthem: open rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS && 0

	rs = fmq_getattr(&q,&mqa) ;

#if	CF_DEBUGS
	debugprintf("readthem: fmq_getattr() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    debugprintf("readthem: flags=%08x\n",mqa.mq_flags) ;
	    debugprintf("readthem: max messages=%d\n",mqa.mq_maxmsg) ;
	    debugprintf("readthem: max message size=%d\n",mqa.mq_msgsize) ;
	    debugprintf("readthem: current messages=%d\n",mqa.mq_curmsgs) ;

	}

#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	debugprintf("readthem: final test output!\n") ;

#endif

#if	CF_RECVDELAY
	sleep(5) ;
#endif

#if	CF_SLOWPOLL
	opts = FM_SLOWPOLL ;
#else
	opts = 0 ;
#endif

	while (! f_signal) {

	    while (TRUE) {

	        to = -1 ;
	        rs = fmq_recve(&q,linebuf,LINEBUFLEN,to,opts) ;

#if	CF_DEBUGS
	        debugprintf("readthem: fmq_recv() rs=%d\n",rs) ;
#endif

	        if (rs != SR_AGAIN)
	            break ;

#if	CF_DEBUGS
	        debugprintmask("readthem: looping fmq_recv() ") ;
	        debugprintf("readthem: waiting f_signal=%u\n",f_signal) ;
#endif

	        msleep(400) ;

	        if (f_signal)
	            break ;

	    } /* end while */

#if	CF_DEBUGS
	    debugprintmask("readthem: ret fmq_recv() ") ;
	    debugprintf("readthem: poll f_signal=%u\n",f_signal) ;
#endif

	    if (f_signal)
	        break ;

	    len = rs ;
	    if (rs < 0)
	        break ;

#if	CF_DEBUGS
	    debugprintf("readthem: receive len=%d\n",len) ;
	    debugprintf("readthem: msg=|%t",linebuf,MIN(len,60)) ;
#endif

	    rs = bwrite(fp,linebuf,len) ;

#if	CF_DEBUGS
	    bflush(fp) ;
	    debugprintf("readthem: bwrite() rs=%d\n",rs) ;
#endif

	} /* end while (signal) */

	fmq_close(&q) ;

#if	CF_UNLINK
	u_unlink(name) ;
#endif

#if	CF_DEBUGS
	debugprintf("readthem: ret rs=%d len=%d\n",rs,len) ;
#endif

ret0:
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (readthem) */


static int writethem(name,fp)
const char	name[] ;
bfile		*fp ;
{
	FMQ	q ;

	int	rs, len ;
	int	bufsize ;
	int	to, opts ;

	char	linebuf[LINEBUFLEN + 1] ;


#if	CF_SMALLBUF
	bufsize = 16 ;
#else
	bufsize = BUFLEN ;
#endif

	rs = fmq_open(&q,name,O_CFLAGS,0666,0) ;

#if	CF_DEBUGS
	debugprintf("main/writethem: fmq_open() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    return rs ;

	while ((rs = breadline(fp,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    to = -1 ;
	    rs = fmq_sende(&q,linebuf,len,to,0) ;

#if	CF_DEBUGS
	    debugprintf("main/writethem: fmq_send() rs=%d\n",rs) ;
#endif

	    if (rs == SR_AGAIN) {

	        rs = SR_OK ;
	        msleep(100) ;

	    }

	    if (rs < 0)
	        break ;

	    if (f_signal)
	        break ;

	} /* end while */

	fmq_close(&q) ;

#if	CF_DEBUGS
	debugprintf("main/writethem: ret rs=%d len=%d\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (writethem) */


static int debugprintmask(s)
const char	s[] ;
{
	sigset_t	cur ;

	int	f ;


	pthread_sigmask(SIG_BLOCK,NULL,&cur) ;

	f = sigismember(&cur,SIGTERM) ;

	debugprintf("%s sigterm=%u\n",s,f) ;

	f = sigismember(&cur,SIGINT) ;

	debugprintf("%s sigint=%u\n",s,f) ;

	f = sigismember(&cur,SIGHUP) ;

	debugprintf("%s sighup=%u\n",s,f) ;

	return 0 ;
}
/* end subroutine (debugprintmask) */



