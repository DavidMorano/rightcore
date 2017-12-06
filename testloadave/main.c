/* main */

/* test out Sun Solaris UNIX 'kstat's */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1


/* revision history:

	= 1988-01-10, David A­D­ Morano

	This subroutine was written (originally) as a test of
	the Sun Solaris UNIX 'kstat' facility.


*/

/* Copyright © 1988 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Test the Sun Solaris® UNIX 'kstat' facility.


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
#include	<kstat.h>

#include	<vsystem.h>
#include	<field.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"loadave.h"


/* local defines */

#define	TO		11

#ifndef	BUFLEN
#define	BUFLEN		(MAXPATHLEN * 2)
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	LOOPS		10


/* external subroutines */

extern int	sfbasename(const char *,int,const char **) ;

extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* forward references */

static void	int_alarm() ;
static void	int_signal() ;

static int	bprintla() ;


/* local variables */

static volatile int	f_alarm ;
static volatile int	f_signal ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct sigaction	sigs ;
	LOADAVE			la ;
	LOADAVE_VALUES		lav ;
	LOADAVE_MID		mid ;
	bfile		input, *ifp = &input ;
	bfile		output, *ofp = &output ;
	bfile		error, *efp = &error ;
	sigset_t	signalmask ;
	kstat_ctl_t	*kcp ;
	kstat_t		*ksp = NULL ;
	kid_t		kid ;
	ulong		hostid = gethostid() ;
	int		rs = SR_OK ;
	int		maxmsglen, trylen, len, conlen ;
	int		i, j ;
	int		sl ;
	int		s ;
	int		fd = -1 ;
	int		err_fd = -1 ;
	const char	*progname ;
	cchar		*cp ;
	char		buf[BUFLEN + 1], buf2[BUFLEN + 1], *bp ;
	char		timebuf[TIMEBUFLEN + 1] ;


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


#if	CF_DEBUGS
	debugprintf("main: hostid rs=%d\n",rs) ;
#endif

	bprintf(ofp,"hostid> %08x (%u)\n",
	    (uint) hostid,
	    (uint) hostid) ;



#if	CF_DEBUGS
	debugprintf("main: about to open the KSTATs\n") ;
#endif

	if ((rs = loadave_start(&la)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("main: loadave_start() rs=%d\n",rs) ;
#endif

	    rs = loadave_readmid(&la,&mid) ;

	    bprintf(ofp,"mid_time> %s\n",
	        timestr_log(mid.tim_read,timebuf)) ;

	    bprintf(ofp,"provider> %s\n",mid.provider) ;

	    bprintf(ofp,"serial> %s\n",mid.serial) ;


	    rs = loadave_readvalues(&la,&lav) ;

	    for (i = 0 ; (rs >= 0) && (i < LOOPS) ; i += 1) {

	        if (i != 0) {

	            sleep(5) ;

	            rs = loadave_readvalues(&la,&lav) ;

	        }

#if	CF_DEBUGS
	        debugprintf("main: loadave_read() rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {

	            bprintf(ofp,"la_time> %s\n",
	                timestr_log(lav.tim_read,timebuf)) ;

	            bprintf(ofp,"nproc=%d\n",lav.nproc) ;

	            bprintf(ofp,"ncpu=%d\n",lav.ncpu) ;

	            bprintla(ofp,lav.la1min,"la1min") ;

	            bprintla(ofp,lav.la5min,"la5min") ;

	            bprintla(ofp,lav.la15min,"la15min") ;

	        }

	    } /* end for */

	    loadave_finish(&la) ;
	} /* end if (opened) */


	bclose(ofp) ;

/* finished */
done:
	bclose(efp) ;

	return EX_OK ;
}
/* end subroutine (main) */


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



/* local subroutines */



static int bprintla(ofp,v,s)
bfile		*ofp ;
uint		v ;
char		s[] ;
{
	int	la_i, la_f ;
	int	la ;
	int	rs = BAD ;


	bprintf(ofp,"%-14s",s) ;

	bprintf(ofp," ul=%lu",v) ;

	la_i = v / FSCALE ;
	la_f = +((v % FSCALE) * 1000) / 256 ;

	bprintf(ofp," ula=%d.%03d\n",la_i,la_f) ;

	return rs ;
}
/* end subroutine (bprintla) */



