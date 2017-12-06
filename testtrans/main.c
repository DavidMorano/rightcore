/* main */

/* test something */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1999-01-10, David A­D­ Morano

	This subroutine was borrowed from someplace else and adapted.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Test something.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/mman.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<field.h>
#include	<bfile.h>
#include	<randomvar.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"


/* local defines */

#define	TO		11


/* external subroutines */

extern char	*strbasename(char *) ;


/* external variables */


/* forward references */

void		int_alarm() ;
void		int_signal() ;


/* gloabal variables */

int		f_alarm ;
int		f_signal ;


/* exported subroutines */


int main(int argc,cchar *argv,cchar *envv)
{
	struct sockaddr_in	local, remote ;
	struct termios		ots, nts ;
	struct sigaction	sigs ;
	RANDOMVAR	r ;
	bfile		input, *ifp = &input ;
	bfile		output, *ofp = &output ;
	bfile		error, *efp = &error ;
	time_t		daytime ;
	sigset_t	signalmask ;
	LONG		lbuf1[4096 / sizeof(LONG)] ;
	LONG		lbuf2[4096 / sizeof(LONG)] ;
	long		addr ;

	int		rs = SR_OK ;
	int		ex ;
	int		maxmsglen, trylen, len, conlen, rs ;
	int		i, j ;
	int		s ;
	int		fd = -1 ;
	int		fd_debug = -1 ;

	const char	*progname ;
	const char	*rhost = NULL ;
	const char	*rport = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0664) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;


	if ((argc < 2) || (argv[1][0] == '\0'))
		return EX_USAGE ;

	rhost = argv[1] ;

	rport = "echo" ;
	if ((argc >= 3) && (argv[2][0] != '\0')) {
		rport = argv[2] ;
	}

/* UDP socket */

#if	CF_DEBUGS
	debugprintf("main: dialudp\n") ;
#endif

	rs = dialudp(rhost,rport,TO,0) ;

	s = rs ;
	if (rs < 0) {

#if	CF_DEBUGS
	    debugprintf("main: dialudp() rs=%d\n",rs) ;
#endif
	    bprintf(efp,"%s: bad connect, rs=%d\n",rs) ;

	    goto badret ;
	}


	ex = EX_OK ;


#if	CF_DEBUGS
	debugprintf("main: looping\n") ;
#endif

	dayutime = time(NULL) ;

	randomvar_start(&r,FALSE,daytime) ;

	trylen = 1024 ;
	for (i = 0 ; i < 100000 ; i += 1) {
		LONG	cksum ;
		LONG	*bp ;

/* send */

		bp = lbuf1 ;
		cksum = 0 ;
		for (j = 1 ; j < (trylen / sizeof(LONG)) ; j += 1) {
			randomvar_getulong(&r,bp + j) ;
			cksum += bp[j] ;
		}

		bp[0] = (- cksum) ;

#if	CF_DEBUGS
	    debugprintf("main: write()\n") ;
#endif

	    rs = u_write(s,lbuf1,trylen) ;

	    if (rs < 0) {

#if	CF_DEBUGS
	        debugprintf("main: bad write rs=%d\n",rs) ;
#endif

	        bprintf(efp,"%s: bad write, rs=%d\n",
	            progname,rs) ;

	        break ;
	    }

#ifdef	COMMENT
	    sleep(5) ;
#endif

#if	CF_DEBUGS
	    debugprintf("main: read()\n") ;
#endif

	    len = uc_reade(s,lbuf2,MAXMSGLEN,TO,0) ;

	    while (len == 0) {

#if	CF_DEBUGS
	        debugprintf("main: missed a packet\n") ;
#endif

	        bprintf(efp,"%s: missed a packet\n", progname) ;

	        rs = u_write(s,lbuf1,trylen) ;

	        len = uc_reade(s,lbuf2,MAXMSGLEN,TO,0) ;

	    } /* end while */


#if	CF_DEBUGS
	    debugprintf("main: data check\n") ;
#endif

	    if ((len < trylen) || (memcmp(lbuf1,lbuf2,trylen) != 0)) {

		ex = EX_DATAERR ;
	        bprintf(efp,"%s: data mismatch i=%d\n",
	            progname,i) ;

	        break ;
	    }

		sleep(SLEEPINT) ;

	} /* end for */

	randomvar_finish(&r) ;

#if	CF_DEBUGS
	debugprintf("main: out of loop\n") ;
#endif


	u_close(s) ;

	u_close(fd) ;


done:
	bclose(efp) ;

	return ex ;

badret:
	bclose(efp) ;

	return EX_DATAERR ;
}
/* end subroutine (main) */


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


