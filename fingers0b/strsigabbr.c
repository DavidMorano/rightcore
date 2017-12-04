/* strsigabbr */

/* return a signal abbreviation string given a signal number */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We take a signal number and we return a corresponding signal
        abbreviation string.

	Synopsis:

	cchar *strsigabbr(uint n)

	Arguments:

	n		signal number to lookup

	Returns:

	-		character-string representation of signal


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<signal.h>

#include	<localmisc.h>


/* local structures */

struct sigabbr {
	int		n ;
	const char	*s ;
} ;


/* local variables */

static const struct sigabbr	cvts[] = {
	{ 0, "TEST" },
	{ SIGHUP, "HUP" },
	{ SIGINT, "INT" },
	{ SIGQUIT, "QUIT" },
	{ SIGILL, "ILL" },
	{ SIGTRAP, "TRAP" }, /* 5 */
	{ SIGABRT, "ABRT" },
	{ SIGEMT, "EMT" },
	{ SIGFPE, "FPE" },
	{ SIGKILL, "KILL" },
	{ SIGBUS, "BUS" }, /* 10 */
	{ SIGSEGV, "SEGV" },
	{ SIGSYS, "SYS" },
	{ SIGPIPE, "PIPE" },
	{ SIGALRM, "ALRM" },
	{ SIGTERM, "TERM" }, /* 15 */
	{ SIGUSR1, "USR1" },
	{ SIGUSR2, "USR2" },
	{ SIGCLD, "CLD" },
	{ SIGCHLD, "CHLD" },
	{ SIGPWR, "PWR" }, /* 20 */
	{ SIGWINCH, "WINCH" },
	{ SIGURG, "URG" },
	{ SIGPOLL, "POLL" },
	{ SIGSTOP, "STOP" },
	{ SIGTSTP, "TSTP" }, /* 25 */
	{ SIGCONT, "CONT" },
	{ SIGTTIN, "TTIN" },
	{ SIGTTOU, "TTOU" },
	{ SIGVTALRM, "VTALRM" },
	{ SIGPROF, "PROF" }, /* 30 */
	{ SIGXCPU, "XCPU" },
	{ SIGXFSZ, "XFSZ" },
#ifdef	SIGWAITING
	{ SIGWAITING, "WAITING" },
#endif
#ifdef	SIGLWP
	{ SIGLWP, "LWP" },
#endif
#ifdef	SIGFREEZE
	{ SIGFREEZE, "FREEZE" },
#endif
#ifdef	SIGTHAW
	{ SIGTHAW, "THAW" },
#endif
	{ SIGCANCEL, "CANCEL" },
	{ SIGLOST, "LOST" },
#if	defined(_SIGRTMIN)
	{ _SIGRTMIN, "RTMIN" },
#endif
#if	defined(_SIGRTMAX)
	{ _SIGRTMAX, "RTMAX" },
#endif
	{ -1, NULL }
} ;


/* exported subroutines */


const char *strsigabbr(uint n)
{
	const char	*s = NULL ;

#if	defined(_SIGRTMIN) && defined(_SIGRTMAX)
	if ((n > _SIGRTMIN) && (n < _SIGRTMAX)) {
	    s = "RTXXX" ;
	}
#endif /* SIGRTXXX */

	if (s == NULL) {
	    int	i ;
	    for (i = 0 ; cvts[i].n >= 0 ; i += 1) {
	        if (cvts[i].n == n) {
		    s = cvts[i].s ;
		    break ;
	        }
	    } /* end for */
	}

	return s ;
}
/* end subroutine (strsigabbr) */


