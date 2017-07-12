/* process */

/* program to test out the UNIX Message Queue IPC API */
/* last modified %G% version %I% */


#define	CF_DEBUGS	1


/* revision history:

	= 1998-05-23, David A­D­ Morano
        This subroutine (it's the whole program -- same as the FIFO test) was
        originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This program tests the UNIX Message Queue IPC facility that exists in
        newer UNIX-type (or other) OSes that have SYSV type features (this was
        originated on System V). All versions of Solaris have UNIX Message
        Queues and most other UNIXes that have any SYSV features usually also
        have this. Note that some UNIXes, like Sun Solaris 1 (SunOS 4.x) can
        disable these features by not including them in a newly built kernel!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/mman.h>
#include	<sys/msg.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<termios.h>
#include	<signal.h>

#include	<vsystem.h>
#include	<field.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	LINELEN		2048		/* must be greater then 1024 */
#ifndef	BUFLEN
#define	BUFLEN		1024
#endif

#define	O_SRVFLAGS	(O_RDONLY | O_CREAT | O_NONBLOCK)


/* external subroutines */

extern int	cfdeci(char *,int,int *), cfhex() ;


/* external variables */


/* forward references */

static int		server(struct proginfo *,int), client() ;

static void		int_alarm() ;
static void		int_signal() ;


/* gloabal variables */

int		f_alarm ;
int		f_signal ;


/* exported subroutines */


/* start of program */
int process(gp,mqid)
struct proginfo	*gp ;
int		mqid ;
{
	struct sigaction	sigs ;

	sigset_t		signalmask ;

	int		len, rs ;
	int		i, j ;
	int		err_fd ;

	char	*progname ;
	char	tmpbuf[100] ;
	char	*buf, *bp ;
	char	*cp ;



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




	if (! gp->f.server) {

	    bfile	infile, *ifp = &infile ;


#if	CF_DEBUGS
	    debugprintf("process: client starting w/ MQID=%d\n",mqid) ;
#endif

	    if ((rs = bopen(ifp,BFILE_STDIN,"dr",0666)) >= 0) {

	        rs = client(gp,mqid,ifp) ;

#if	CF_DEBUGS
	        debugprintf("process: client() rs=%d\n",rs) ;
#endif

	        bclose(ifp) ;

	    }

	} else {

#if	CF_DEBUGS
	    debugprintf("process: server starting\n") ;
#endif

/* create the MQ */

	    rs = u_msgget(IPC_PRIVATE,0660 | IPC_CREAT) ;

#if	CF_DEBUGS
	    debugprintf("process: u_msgget rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

	        mqid = rs ;
	        bprintf(gp->ofp,"created MQID=%d\n",
	            mqid) ;


	        bflush(gp->ofp) ;


	        rs = server(gp,mqid) ;

#if	CF_DEBUGS
	        debugprintf("process: server rs=%d\n",rs) ;
#endif

	        u_msgctl(mqid,IPC_RMID,NULL) ;


	    } else {

	        bprintf(gp->ofp,"could not create MQ (rs %d)\n",
	            rs) ;

	    }

	} /* end if (client or server) */



badret:
	return rs ;

badin:
	bprintf(gp->efp,"%s: could not open input, rs=%d\n",
	    progname,rs) ;

	goto badret ;

badout:
	bprintf(gp->efp,"%s: could not open output, rs=%d\n",
	    progname,rs) ;

	goto badret ;

}
/* end subroutine (process) */


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


static int server(gp,mqid)
struct proginfo	*gp ;
int		mqid ;
{
	struct msgbuffer	in, out ;

	long	msgtype = 1 ;

	int	rs, len ;
	int	i, j ;



#if	CF_DEBUGS
	debugprintf("server: open rs=%d\n",rs) ;
#endif

	bprintf(gp->ofp,"are we ready to play ?\n") ;

	in.msgtype = msgtype ;
	while (! f_signal) {

	    while (TRUE) {

	    len = u_msgrcv(mqid,&in,BUFLEN,msgtype,IPC_NOWAIT) ;

	    if (len != SR_NOMSG)
		break ;

	        if (f_signal) break ;

	        sleep(1) ;

	    } /* end while */

	    if (len < 0) break ;

#if	CF_DEBUGS
	    debugprintf("server: receive len=%d\n",len) ;
#endif

	    rs = bwrite(gp->ofp,in.buf,len) ;

	    j = 0 ;
	    for (i = 0 ; (i < len) && (in.buf[i] != '\n') ; i += 1) {

	        out.buf[j++] = in.buf[i] ;
	        out.buf[j++] = ' ' ;

	        if (j >= BUFLEN)
	            break ;

	    } /* end for */

	out.buf[j++] = '\n' ;

#if	CF_DEBUGS
	    debugprintf("server: send len=%d\n",j) ;
#endif


	    out.msgtype = 2 ;
	    u_msgsnd(mqid,&out,j,0) ;

#if	CF_DEBUGS
	    bflush(gp->ofp) ;
	    debugprintf("server: bwrite rs=%d\n",rs) ;
#endif

	} /* end while */

#if	CF_DEBUGS
	debugprintf("server: exiting len=%d\n",len) ;
#endif

	return len ;
}
/* end subroutine (server) */


static int client(gp,mqid,fp)
struct proginfo	*gp ;
int		mqid ;
bfile		*fp ;
{
	struct msgbuffer	b ;

	long	mytype = 2 ;

	int	rs, len ;



	while ((len = breadline(fp,b.buf,BUFLEN)) > 0) {

#if	CF_DEBUGS
	debugprintf("client: sending to server> %W",b.buf,len) ;
#endif

	    b.msgtype = 1 ;
	    rs = u_msgsnd(mqid,&b,len,0) ;

#if	CF_DEBUGS
	debugprintf("client: sending u_msgsnd rs=%d\n",rs) ;
#endif

	    while (TRUE) {

	    len = u_msgrcv(mqid,&b,BUFLEN,mytype,IPC_NOWAIT) ;

	    if (len != SR_NOMSG)
		break ;

	        if (f_signal) break ;

	        sleep(1) ;

	    } /* end while */

	    if (len < 0) break ;


	rs = len ;

#if	CF_DEBUGS
	debugprintf("client: u_msgrcv rs=%d\n",rs) ;
#endif

	    if (rs > 0) {

	        bwrite(gp->ofp,b.buf,len) ;

		bprintf(gp->ofp,"\n") ;

	    }

	} /* end while */


	return len ;
}
/* end subroutine (client) */



