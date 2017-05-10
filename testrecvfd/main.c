/* main */

/* test receiving an FD (only on real UNIXi) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time print-outs */


/* revision history:

	= 2008-07-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This subroutine tests receiving an FD on a mounted
	pipe-file.


***************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<stropts.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>
#include	<pwd.h>
#include	<netdb.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<field.h>
#include	<vecstr.h>
#include	<hostent.h>
#include	<sockaddress.h>
#include	<inetaddr.h>
#include	<pwfile.h>
#include	<svcfile.h>
#include	<connection.h>
#include	<storebuf.h>
#include	<getax.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#define	PASSFNAME	"here"

#define	TO_RUN		60 /* seconds */
#define	TO_RECVFD	10 /* seconds */



/* external subroutines */

extern int acceptpass(int,struct strrecvfd *,int) ;
extern int listenpass(const char *,mode_t,int) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	hasalldig(const char *,int) ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct strrecvfd	passinfo ;

	struct pollfd		pfds[4] ;

	FILE	*ofp = stdout ;

	mode_t	operms = 0666 ;

	time_t	daytime = time(NULL) ;
	time_t	ti_start = 0 ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	ex = EX_INFO ;
	int	pfd = -1 ;
	int	fd ;
	int	re ;
	int	v ;
	int	mlen ;
	int	nfds ;
	int	i ;
	int	c = 0 ;
	int	to_run = TO_RUN ;
	int	to = TO_RECVFD ;
	int	f_output = FALSE ;

	const char	*mbuf = "hello world from the under!\n" ;
	const char	*passfname = PASSFNAME ;


	if (argc > 1) {
		for (i = 1 ; argv[i] != NULL ; i += 1) {
		    if (strcmp(argv[i],"-l") == 0) {
			f_output = TRUE ;
		    } else if (hasalldig(argv[i],-1)) {
			rs1 = cfdecti(argv[i],-1,&v) ;
			if (rs1 >= 0) to_run = v ;
		    }
		} /* end for */
	}

	if (! f_output) ofp = NULL ;

	if (ofp != NULL)
	    fprintf(ofp,"starting\n") ;

	rs = listenpass(passfname,operms,0) ;
	pfd  = rs ;
	if (rs < 0)
	    goto done ;

	nfds = 0 ;
	pfds[nfds].fd = pfd ;
	pfds[nfds].events = (POLLIN | POLLPRI) ;
	nfds += 1 ;

	ti_start = daytime ;
	while ((rs = u_poll(pfds,nfds,1000)) >= 0) {

	    if (ofp != NULL)
	        fprintf(ofp,"waking up c=%u\n",c) ;

	    daytime = time(NULL) ;

	    if (rs > 0) {

	        if (ofp != NULL)
	            fprintf(ofp,"work n=%u\n",rs) ;

	        for (i = 0 ; i < nfds ; i += 1) {
	            fd = pfds[i].fd ;
	            re = pfds[i].revents ;
	            if ((fd == pfd) && ((re & POLLIN) || (re & POLLPRI))) {

	                if (ofp != NULL)
	                    fprintf(ofp,"input\n") ;

	                if ((rs = acceptpass(pfd,&passinfo,to)) >= 0) {

	                    if (ofp != NULL)
	                        fprintf(ofp,"accept\n") ;

	                    fd = rs ;
	                    mlen = strlen(mbuf) ;
	                    rs1 = u_write(fd,mbuf,mlen) ;

	                    if (ofp != NULL)
	                        fprintf(ofp,"write() rs=%d\n",rs1) ;

	                    u_close(fd) ;

	                } /* end if (handled client) */

	                if (ofp != NULL)
	                    fprintf(ofp,"accept() rs=%d\n",rs) ;

	            } /* end if (found ownership) */
	            if (rs < 0)
	                break ;
	        } /* end for */
	    } /* end if (poll found some) */

	    if (rs < 0)
	        break ;

	    if ((daytime - ti_start) >= to_run)
	        break ;

	    c += 1 ;
	    fflush(ofp) ;

	} /* end while (polling) */

badaccept:
	if (pfd > 0)
	    u_close(pfd) ;

done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

ret0:
	return ex ;
}
/* end subroutine (main) */



