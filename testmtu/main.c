/* main */

/* find the maximum MTU size for the established route */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0


/* revision history:

	= 99/01/10, David A­D­ Morano

	This subroutine was written (originally) as a test of
	the maximum MTU size for UDP packets on IP.


*/



/************************************************************************

	Find the maximum MTU for UDP/IP.


***************************************************************************/




#include	<sys/types.h>
#include	<sys/mman.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<field.h>
#include	<bfile.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"




/* local defines */

#define	TO		11



/* external subroutines */

extern int	uc_readn(), uc_writen() ;
extern int	dialudp(const char *,const char *,int,int) ;

extern char	*strbasename(char *) ;


/* external variables */


/* forward references */

void		int_alarm() ;
void		int_signal() ;


/* gloabal variables */

int		f_alarm ;
int		f_signal ;





int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile	input, *ifp = &input ;
	bfile	output, *ofp = &output ;
	bfile	error, *efp = &error ;

	sigset_t		signalmask ;

	struct sockaddr_in	local, remote ;

	struct termios		ots, nts ;

	struct sigaction	sigs ;

	long		addr ;

	int	ex = EX_INFO ;
	int		maxmsglen, trylen, len, conlen, rs ;
	int		i, j ;
	int		s, fd = 0 ;
	int		fd_debug ;

	char	*progname ;
	char	*rhost ;
	char	*rport ;
	char	buf[MAXMSGLEN + 1], buf2[MAXMSGLEN + 1], *bp ;
	char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
		cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0664) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;


	if ((argc < 2) || (argv[1][0] == '\0'))
		return EX_USAGE ;

		rhost = argv[1] ;

	rport = "echo" ;
	if ((argc >= 3) && (argv[2][0] != '\0'))
		rport = argv[2] ;


	rs = dialudp(rhost,rport,TO,0) ;

	s = rs ;
	if (rs < 0) {

#if	CF_DEBUGS
	    debugprintf("main: connect rs=%d\n",rs) ;
#endif

	ex = EX_DATAERR ;
	    bprintf(efp,"%s: bad connect, rs=%d\n",rs) ;

	    goto badret ;
	}

#if	CF_DEBUGS
	debugprintf("main: looping\n") ;
#endif

	maxmsglen = 2048 ;

	ex = EX_OK ;
	for (trylen = 990 ; trylen < maxmsglen ; trylen += 1) {

#if	CF_DEBUGS
	    debugprintf("main: trying MTU=%d\n",trylen) ;
#endif

/* send */

#if	CF_DEBUGS
	    debugprintf("main: write()\n") ;
#endif

	    rs = u_write(s,buf,trylen) ;

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

	    len = uc_reade(s,buf2,MAXMSGLEN,TO,0) ;

	    while (len == 0) {

#if	CF_DEBUGS
	        debugprintf("main: missed a packet\n") ;
#endif

	        rs = u_write(s,buf,trylen) ;

	        len = uc_reade(s,buf2,MAXMSGLEN,TO,0) ;

	    }


#if	CF_DEBUGS
	    debugprintf("main: data check\n") ;
#endif

	    if ((len < trylen) || (memcmp(buf,buf2,trylen) != 0)) {

		ex = EX_DATAERR ;
	        bprintf(efp,"%s: MTU=%d\n",
	            progname,trylen - 1) ;

	        break ;
	    }


	} /* end for */

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

	return EX_USAGE ;
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




