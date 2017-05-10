/* main */


#define	CF_DEBUGS	0
#define	CF_STAND		0


/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)in.talkd.c	1.5	97/05/16 SMI"	/* SVr4.0 1.3	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */


/*
 * Invoked by the Internet daemon to handle talk requests
 * Processes talk requests until MAX_LIFE seconds go by with 
 * no action, then dies.
 */



#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/systeminfo.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include	<logfile.h>
#include	<mallocstuff.h>

#include	"config.h"
#include	"defs.h"
#include	"ctl.h"


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	listenudp(int,const char *,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;

extern void print_error(char *string);
extern void print_response(CTL_RESPONSE *response);
extern void print_request(CTL_MSG *request);
extern void process_request(struct proginfo *,
		CTL_MSG *request, CTL_RESPONSE *response);


/* forward references */

static CTL_MSG swapmsg();


/* global variables */

CTL_MSG 	request ;

CTL_RESPONSE 	response ;

char hostname[HOST_NAME_LENGTH];

int debug = 1 ;


/* local variables */

static char	*const reqtypes[] = {
	"invitation",
	"lookup",
	"deletion",
	"announcement",
	NULL
} ;

enum reqtypes {
	reqtype_invitation,
	reqtype_lookup,
	reqtype_deletion,
	reqtype_announcement,
	reqtype_overlast
} ;


/* exported subroutines */


int main(argc,argv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
    struct sockaddr_in from;

	struct proginfo	pi, *pip = &pi ;

    struct timeval tv;

    socklen_t from_size = (socklen_t)sizeof(from);

    fd_set rfds;

	const int	af = AF_INET ;

    int cc;
    int name_length = sizeof(hostname);
	int	rs, len, sl ;
	int	fd_listen ;
	int	fd_debug = -1 ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*cp, *cp2 ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) && (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;

#if	CF_DEBUGS
	debugprintf("main: entered\n") ;
#endif


	memset(pip,0,sizeof(struct proginfo)) ;

	pip->version = VERSION ;
	pip->progname = strbasename(argv[0]) ;


        (void) sysinfo(SI_HOSTNAME, hostname, name_length);

	pip->pid = getpid() ;

	if ((cp = getenv(LOGFILEVAR)) == NULL)
		cp = LOGFNAME ;

	pip->logfname = mallocstr(cp) ;

	pip->daytime = time(NULL) ;

#if	CF_DEBUGS
	debugprintf("main: logfname=%s daytime=>%s<\n",
		pip->logfname,
		timestr_log(pip->daytime,timebuf)) ;
#endif


/* create a log ID */

	cp = strwcpy(timebuf,hostname,4) ;

	len = cp - timebuf ;
	if (len > 4) {

		cp = timebuf + 4 ;
		len = 4 ;
	}

	rs = ctdeci(cp,(TIMEBUFLEN - len),(int) pip->pid) ;

	if (rs > 0)
		timebuf[4 + rs] = '\0' ;

	pip->logid = mallocstr(timebuf) ;

#if	CF_DEBUGS
	debugprintf("main: logid=%s\n",pip->logid) ;
#endif


/* open the log file (if we can) */

	rs = logfile_open(&pip->actlog,pip->logfname,0,0666,pip->logid) ;

#if	CF_DEBUGS
	debugprintf("main: logfile_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

		pip->f.logfile = TRUE ;

		logfile_printf(&pip->actlog,"%s %s\n",
			timestr_log(pip->daytime,timebuf),
			BANNER) ;

		logfile_printf(&pip->actlog,"node=%s version=%s\n",
			hostname,VERSION) ;

	} /* end if (logfile) */


#if	CF_STAND
	rs = listenudp(af,NULL,"talk",0) ;

	if (rs < 0) {

#if	CF_DEBUGS
	debugprintf("main: listenudp() rs=%d\n",rs) ;
#endif

		logfile_printf(&pip->actlog,
			"could not listen to our port (%d)\n",rs) ;

		goto done ;
	}

	fd_listen = rs ;
	u_close(FD_STDIN) ;

	u_close(FD_STDOUT) ;

	u_dup(fd_listen) ;

	u_dup(fd_listen) ;

	u_close(fd_listen) ;

#endif /* CF_STAND */


/* loop until time-out */

#if	CF_DEBUGS
	debugprintf("main: looping\n") ;
#endif

    for (;;) {

	tv.tv_sec = MAX_LIFE;
	tv.tv_usec = 0;
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	if (select(1, &rfds, 0, 0, &tv) <= 0)
		goto done ;

	pip->daytime = time(NULL) ;

#if	CF_DEBUGS
	debugprintf("main: got something %s\n",
		timestr_log(pip->daytime,timebuf)) ;
#endif

	cc = recvfrom(0, (char *)&request, sizeof (request), 0, 
		      (struct sockaddr *)&from, &from_size);

	if (cc != sizeof(request)) {

	    if (cc < 0 && errno != EINTR) {
		print_error("receive");
	    }

	} else {

	    if (debug) printf("Request received : \n");

	    if (debug) print_request(&request);

/* check for endian problems */

	    request = swapmsg(request);

/* log this */

	if (pip->f.logfile) {

		cp = "unknown" ;
		if (request.type < reqtype_overlast)
			cp = reqtypes[request.type] ;

		sl = strnlen(request.l_name,NAME_SIZE) ;

		rs = logfile_printf(&pip->actlog,"%s %s u=%t id=%d\n",
			timestr_log(pip->daytime,timebuf),
			cp,
			request.l_name,sl,
			request.id_num) ;

#if	CF_DEBUGS && 0
	debugprintf("main: logfile_printf() rs=%d\n",rs) ;
#endif

	} /* end if */

/* process the message */

	    process_request(pip,&request, &response);


	    if (debug) printf("Response sent : \n");

	    if (debug) print_response(&response);

		/* can block here, is this what I want? */

/* send response back to sender */

	    cc = sendto(0, (char *) &response, sizeof(response), 0,
			(struct sockaddr *)&request.ctl_addr,
			(socklen_t)sizeof(request.ctl_addr));

	    if (cc != sizeof(response)) {

#if	CF_DEBUGS
	debugprintf("main: sendto() rs=%d\n",cc) ;
#endif

		print_error("Send");
	    }

	} /* end if */

    } /* end for */


/* we're done */
done:

#if	CF_DEBUGS
	debugprintf("main: done rs=%d\n",rs) ;
#endif

	if (pip->f.logfile)
		logfile_close(&pip->actlog) ;

	return 0 ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



#define swapshort(a) (((a << 8) | ((unsigned short) a >> 8)) & 0xffff)
#define swaplong(a) ((swapshort(a) << 16) | (swapshort(((unsigned)a >> 16))))


/*  
 * heuristic to detect if need to swap bytes
 */

static CTL_MSG
swapmsg(req)
	CTL_MSG req;
{
	CTL_MSG swapreq;


	if (req.ctl_addr.sin_family == swapshort(AF_INET)) {

		swapreq = req;
		swapreq.id_num = swaplong(req.id_num);
		swapreq.pid = swaplong(req.pid);
		swapreq.addr.sin_family = swapshort(req.addr.sin_family);
		swapreq.ctl_addr.sin_family =
			swapshort(req.ctl_addr.sin_family);
		return swapreq;

	} else
		return req;

}



