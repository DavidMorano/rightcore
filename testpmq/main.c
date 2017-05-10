/* main */

/* program to test out the Posix Message Queue (PMQ) API */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 2001-88-01, David A­D­ Morano

	This subroutine (it's the whole program -- same as the FIFO test) was
	originally written.


*/
/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program tests the Posix Message Queue (PMQ) mechanism that exists
	in newer UNIX-type (or other) OSes that have this newer POSIX feature.
	Most all versions of Solaris before release 2.5.1 do NOT support Posix
	Message Queues!


*******************************************************************************/


#include	<envstandards.h>

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
#include	<pmq.h>
#include	<localmisc.h>

#include	"config.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048		/* must be greater then 1024 */
#endif

#define	BUFLEN		1024

#define	O_SRVFLAGS	(O_RDONLY | O_CREAT | O_NONBLOCK)


/* external subroutines */

extern int	sfbasename(cchar *,int,cchar **) ;
extern int	cfdeci(const char *,int,int *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	debugprinthexblock(const char *,int,const void *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* forward references */

static int	readthem() ;
static int	writethem() ;

static void	int_alarm() ;
static void	int_signal() ;


/* gloabal variables */

int		f_alarm ;
int		f_signal ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct termios		ots, nts ;
	struct sigaction	sigs ;
	sigset_t		signalmask ;
	bfile		input, *ifp = &input ;
	bfile		output, *ofp = &output ;
	bfile		error, *efp = &error ;

	int		rs, i, j ;
	int		len ;
	int		fd_debug ;
	int		ex = 0 ;

	const char	*progname ;
	char		*buf, *bp ;
	char		*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	sfbasename(argv[0],-1,&progname) ;

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
#endif /* CF_DEBUGS */

	    bprintf(ofp,"reading message queue\n") ;

	    bflush(ofp) ;

	    rs = readthem(argv[1],ofp) ;

#if	CF_DEBUGS
	    debugprintf("main: readthem rs=%d\n",rs) ;
#endif

	    bprintf(ofp,"\rserver exiting\n") ;

	    bclose(ofp) ;
	} /* end if (reading) */

done:
	bclose(efp) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return OK ;

badret:
	bclose(efp) ;
	ex = EX_DATAERR ;
	goto ret0 ;

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


/* local subroutines */


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
char	name[] ;
bfile	*fp ;
{
	PMQ		q ;
	mode_t		om ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("readthem: ent name=%s\n",name) ;
#endif

#if	CF_DEBUGS
	debugprintf("readthem: here is some test output!\n") ;
	bprintf(fp,"are we ready to play?\n") ;
#endif

/* make the message queue accessible to everyone ! */

	if ((rs = pmq_open(&q,name,O_SRVFLAGS,0666,NULL)) >= 0) {
	    struct mq_attr	mqa ;
	    if ((rs = pmq_getattr(&q,&mqa)) >= 0) {
		const int	llen = LINEBUFLEN ;
		int		len ;
		char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("readthem: pmq_getattr() rs=%d\n",rs) ;
#endif

		bprintf(fp,"flags=%08x\n",mqa.mq_flags) ;
		bprintf(fp,"max messages=%d\n",mqa.mq_maxmsg) ;
		bprintf(fp,"max message size=%d\n",mqa.mq_msgsize) ;
		bprintf(fp,"current messages=%d\n",mqa.mq_curmsgs) ;

#if	CF_DEBUGS
	debugprintf("readthem: final test output !\n") ;
	bprintf(fp,"are we ready to play ?\n") ;
#endif

	while ((rs >= 0) && (! f_signal)) {

	    while ((rs = pmq_receive(&q,lbuf,llen,NULL)) == SR_AGAIN) {
		if (f_signal) break ;
	        sleep(1) ;
	    } /* end while */
	    len = rs ;

#if	CF_DEBUGS
	    debugprintf("readthem: receive len=%d\n",len) ;
#endif

	    if (rs >= 0) {
	        rs = bwrite(fp,lbuf,len) ;
	    }

#if	CF_DEBUGS
	    bflush(fp) ;
	    debugprintf("readthem: bwrite rs=%d\n",rs) ;
#endif

	} /* end while */

	    } /* end if (pmq_getattr) */
	    rs1 = pmq_close(&q) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (pmq) */

	uc_unlinkpmq(name) ;

#if	CF_DEBUGS
	debugprintf("readthem: ret rs=%d len=%d\n",rs,len) ;
#endif

	return rs ;
}
/* end subroutine (readthem) */


static int writethem(name,fp)
char	name[] ;
bfile	*fp ;
{
	PMQ		q ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((rs = pmq_open(&q,name,O_WRONLY,0666,NULL)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    char	lbuf[LINEBUFLEN + 1] ;
	    while ((rs = breadline(fp,lbuf,llen)) > 0) {
	        len = rs ;
	        rs = pmq_send(&q,lbuf,len,0) ;
		wlen += rs ;
	        if (rs < 0) break ;
	    } /* end while */
	    rs1 = pmq_close(&q) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (pmq) */

#if	CF_DEBUGS
	debugprintf("main/writethem: ret rs=%d len=%d\n",rs,len) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (writethem) */


