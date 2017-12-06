/* main */

/* test out Sun Solaris UNIX 'kstat's */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1
#define	CF_TESTSYSINFO	0


/* revision history:

	= 1988001-10, David A­D­ Morano

	This subroutine was written (originally) as a test of
	the Sun Solaris UNIX 'kstat' facility.


*/

/* Copyright © 1988 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Test the Sun Solaris UNIX® 'kstat' facility.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<sys/systeminfo.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<field.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	TO		11


/* external subroutines */

extern int	sfbasename(const char *,int,const char **) ;
extern int	cfdeci(char *,int,int *) ;


/* external variables */


/* forward references */

void		int_alarm() ;
void		int_signal() ;

static int	bprintla() ;


/* gloabal variables */

static volatile int	f_alarm ;
static volatile int	f_signal ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	bfile	input, *ifp = &input ;
	bfile	output, *ofp = &output ;
	bfile	error, *efp = &error ;

	sigset_t		signalmask ;

	struct sigaction	sigs ;

	ulong	hostid = gethostid() ;

	int	rs = SR_OK ;
	int	maxmsglen, trylen, len, conlen ;
	int	i, j ;
	int	sl ;
	int	s ;
	int	fd = -1 ;
	int	err_fd = -1 ;

	const char	*progname ;

	char	buf[BUFLEN + 1], buf2[BUFLEN + 1], *bp ;
	char	*cp ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;


	sfbasename(argv[0],-1,&progname) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0664) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

	bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

/* print out some common accessible machine parameters */

	sl = u_sysinfo(SI_HW_PROVIDER,buf,BUFLEN) ;

#if	CF_DEBUGS
	debugprintf("main: hw_provider rs=%d\n",sl) ;
#endif

	bprintf(ofp,"provider> %W\n",buf,sl) ;


	sl = u_sysinfo(SI_HW_SERIAL,buf,BUFLEN) ;

	bprintf(ofp,"serial> %W\n",buf,sl) ;


#if	CF_TESTSYSINFO
	for (i = 7 ; i < 12 ; i += 1) {
		rs = u_sysinfo(SI_HW_SERIAL,buf,i) ;
		bprintf(ofp,"testsysinfo> len=%d rs=%d\n",i,rs) ;
	}
#endif /* CF_TESTSYSINFO */

#if	CF_DEBUGS
	debugprintf("main: hostid rs=%d\n",rs) ;
#endif

	bprintf(ofp,"hostid> %u (%08x)\n",
		(uint) hostid,
		(uint) hostid) ;

	bclose(ofp) ;

/* finished */
done:
	bclose(efp) ;

	return EX_OK ;

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



