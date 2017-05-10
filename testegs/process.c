/* process */

/* program to test out the Entropy Gathering Daemon (EGD) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	1
#define	CF_DEBUG	1
#define	F_SIGNAL	0


/* revision history:

	= 88/01/01, David A­D­ Morano

	This subroutine (it's the whole program -- same as
	the FIFO test) was originally written.


*/



/************************************************************************

	This subroutine tests the ENTROPY object.



***************************************************************************/




#include	<sys/types.h>
#include	<sys/mman.h>
#include	<sys/msg.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<field.h>
#include	<bfile.h>
#include	<netorder.h>

#include	"entropy.h"

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"




/* local defines */

#define	LINELEN		2048		/* must be greater then 1024 */
#ifndef	BUFLEN
#define	BUFLEN		1024
#endif

#define	O_SRVFLAGS	(O_RDONLY | O_CREAT | O_NONBLOCK)

#undef	DEFPATHNAME
#define	DEFPATHNAME	"/tmp/entropy"

#define	COLUMNS		8
#define	SLEEPTIME	0



/* external subroutines */


/* external variables */


/* forward references */

static void	int_alarm() ;
static void	int_signal() ;


/* gloabal variables */

int		f_alarm ;
int		f_signal ;






int process(pip,pathname)
struct proginfo	*pip ;
char		pathname[] ;
{
	struct sigaction	sigs ;

	sigset_t		signalmask ;

	int		len, rs ;
	int		i, j ;
	int		iw ;

	char	buf[BUFLEN + 1], *bp ;
	char	*cp ;



	f_signal = FALSE ;
	f_alarm = FALSE ;


	if ((pathname == NULL) || (pathname[0] == '\0'))
	    pathname = DEFPATHNAME ;


/* set the signals that we want to catch */

#if	F_SIGNAL
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
#endif /* F_SIGNAL */


	{
	    ENTROPY	e ;

	    pid_t	pid ;


	    if ((rs = entropy_start(&e,pathname)) >= 0) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("process: opened rs=%d\n",rs) ;
#endif

	        rs = entropy_getpid(&e,&pid) ;


#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("process: daemon at pid=%d\n",pid) ;
#endif

	        j = 0 ;
	        for (i = 0 ; i < 100 ; i += 1) {

#if	SLEEPTIME > 0
	            sleep(SLEEPTIME) ;
#endif

	            rs = entropy_level(&e) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("process: level rs=%d\n",rs) ;
#endif

	            rs = entropy_read(&e,buf,MIN(BUFLEN,4)) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("process: entropy_read() rs=%d\n",rs) ;
#endif

	            if (rs >= 0) {

	                netorder_rint(buf,&iw) ;

	                bprintf(pip->ofp," %08x",iw) ;

	                if (j == (COLUMNS - 1)) {

	                    j = 0 ;
	                    bprintf(pip->ofp,"\n") ;

	                } else
	                    j += 1 ;

	            } /* end if */

	        } /* end for */

	        if ((i > 0) && (j != 0))
	            bprintf(pip->ofp,"\n") ;

	        entropy_finish(&e) ;

	    } /* end if (opened entropy) */

	} /* end block */


badret:
	return rs ;

badin:
	bprintf(pip->efp,"%s: could not open input, rs=%d\n",
	    pip->progname,rs) ;

	goto badret ;

badout:
	bprintf(pip->efp,"%s: could not open output, rs=%d\n",
	    pip->progname,rs) ;

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


