/* run */

/* subroutines to maintain status on ADVICE runs */


#define	CF_DEBUG	0


/* revision history:

	- David A.D. Morano, February 1996

	This program was originally written.


*/


/*******************************************************************

	This subroutine will allocate structures for maintaining 	
	information on ADVICE runs in progress.


*********************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/wait.h>
#include	<rpc/rpc.h>
#include	<unistd.h>
#include	<signal.h>
#include	<fcntl.h>
#include	<time.h>
#include	<ctype.h>
#include	<string.h>
#include	<stdlib.h>
#include	<errno.h>

#include	<bfile.h>
#include	<baops.h>
#include	<userinfo.h>
#include	<vecitem.h>
#include	<vecstr.h>
#include	<field.h>
#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"varsub.h"
#include	"paramopt.h"
#include	"configfile.h"



/* local defines */

#ifdef	LINELEN
#undef	LINELEN
#endif
#define	LINELEN		100



/* external subroutines */

extern int	cfdec() ;
extern int	matstr() ;
extern int	configread(), configfree() ;
extern int	paramloads(), paramloadu(), paramload() ;
extern int	cfdouble() ;

extern char	*strbasename() ;


/* forward subroutines */


/* external variables */


/* local static data */

static int	f_alarm ;




/* initialize the run structure */
int runinit(rhp,n)
vecitem	*rhp ;
int		n ;
{


	return vecitem_start(rhp,n,VECITEM_PSORTED) ;
}
/* end subroutine (runinit) */


int runcount(rhp)
vecitem	*rhp ;
{


	if (rhp == NULL) 
		return BAD ;

	return rhp->i ;
}


/* add an entry to the run list */
int runadd(rhp,hostname,pid)
vecitem	*rhp ;
char		hostname[] ;
pid_t		pid ;
{
	struct run	r ;

	int		i, rs, len ;


	if (rhp == NULL) 
		return BAD ;

	r.pid = pid ;

/* link this entry into the list */

	if ((rs = vecitem_add(rhp,&r,sizeof(struct run))) >= 0)
	    return rs ;

/* release the allocated memory */

	return rs ;
}
/* end subroutine (runadd) */


/* delete a run entry */
int rundel(rhp,i)
machinehead	*rhp ;
int		i ;
{


	return vecitem_del(rhp,i) ;
}
/* end subroutine (rundel) */


/* wait for some run to complete */

/*
	Returns:
	- the number of runs still outstanding
*/

static void int_alarm() ;

int runwait(rhp,timeout)
machinehead	*rhp ;
int		timeout ;
{
	sigset_t		signalmask ;

	struct sigaction	sigs, oldsigs ;

	struct run	*rp ;

	long		time ;

	int		childstat ;
	int		count, i, rs ;
	pid_t		pid ;


/* if we have no child processes out, return immediately */

	if ((count = vecitem_count(rhp)) <= 0) 
		return -1 ;

#if	CF_DEBUG
	debugprintf(
	    "runwait: about to do the wait stuff count=%d\n",count) ;
#endif

/* set up the handler for alarms */

	if (timeout < 0) 
		timeout = 30 ;

	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = int_alarm ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;		/* let 'waitpid' get interrupted */
	sigaction(SIGALRM,&sigs,&oldsigs) ;

	f_alarm = FALSE ;
	while (timeout > 0) {

	    alarm(MIN(timeout,5)) ;

#if	CF_DEBUG
	        debugprintf(
	            "runwait: about to waitpid\n") ;
#endif

	    pid = waitpid((pid_t) 0,&childstat,WUNTRACED) ;

	    if (pid > 0) {

#if	CF_DEBUG
	        debugprintf(
	            "runwait: process exited pid=%d stat=%04X\n",
	            pid,childstat) ;
#endif

/* delete the "run" list entry for the process which exited */

	        for (i = 0 ; vecitem_get(rhp,i,&rp) >= 0 ; i += 1) {

	            if (rp == NULL) continue ;

#if	CF_DEBUG
	            debugprintf(
	                "runwait: looking at entry i=%d pid=%d\n",i,rp->pid) ;
#endif

	            if (rp->pid == pid) {

#if	CF_DEBUG
	                debugprintf(
	                    "runwait: deleting entry i=%d pid=%d\n",i,pid) ;
#endif

	                vecitem_del(rhp,i) ;

	                break ;
	            }

	        } /* end for */

#if	CF_DEBUG
	        debugprintf(
	            "runwait: out of for loop\n") ;
#endif

	        break ;

	    } /* end if (a process exited) */

#if	CF_DEBUG
	    if ((pid < 0) && (errno == EINTR)) debugprintf(
	        "runwait: got an interrupt\n") ;
#endif

	if ((pid < 0) && (errno == ECHILD)) {

#if	CF_DEBUG
	    debugprintf(
	        "runwait: no more children\n") ;
#endif

		alarm(0) ;

		sigaction(SIGALRM,&oldsigs,NULL) ;

		return 0 ;
	}

	    timeout -= MIN(timeout,5) ;

	} /* end while */

#if	CF_DEBUG
	debugprintf(
	    "runwait: exiting timeout loop, timeout=%d\n",timeout) ;
#endif

/* let's move out */

	alarm(0) ;

	sigaction(SIGALRM,&oldsigs,NULL) ;

	return vecitem_count(rhp) ;
}
/* end subroutine (runwait) */




static void int_alarm(sig)
int	sig ;
{

	f_alarm = TRUE ;
}



