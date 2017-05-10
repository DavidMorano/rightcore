/* runlocal */

/* execute the ADVICE program on the local machine */


#define	CF_DEBUG	0


/* revision history:

	- David A.D. Morano, October 1993

	This program was originally written.

	- David A.D. Morano, Febuary 1996

	This program was pretty extensively modified to take
	much more flexible combinations of user supplied paramters.


*/


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

#include	<bfile.h>
#include	<baops.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<vecstr.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"varsub.h"
#include	"paramopt.h"
#include	"configfile.h"



/* defines */

#define		MAXARGINDEX	100

#define		MAXARGGROUPS	(MAXARGINDEX/8 + 1)
#define		LINELEN		200
#define		BUFLEN		(MAXPATHLEN + (LINELEN * 2))




/* external subroutines */

extern int	cfdec() ;
extern int	matstr() ;
extern int	configread(), configfree() ;
extern int	paramloads(), paramloadu(), paramload() ;
extern int	cfdouble() ;
extern int	machineinit(), machineadd() ;
extern int	runinit(), runadd(), rundel() ;

extern char	*strbasename() ;

extern void	vsaprint() ;
extern void	fixdisplay() ;


/* forward references */

void	int_signal() ;


/* global data */

extern struct global	g ;




int runlocal(nslave,pid)
int	nslave ;
pid_t	pid ;
{
	struct paramname	*pp ;

	struct paramvalue	*vp ;

	varsub		vsh ;

	struct confighead	ch ;

	int	maxai, pan, npa, kwi, i, j ;
	int	f_puttmp = TRUE ;
	int	len, l, rs ;
	int	f_removerun = FALSE ;
	int	f_machines = FALSE ;
	int	n ;
	int	childstat ;

	char	linebuf[LINELEN + 1], *lbp ;
	char	buf[BUFLEN + 1] ;
	char	*configfname = NULL ;
	char	tmpfname[MAXNAMELEN + 1] ;
	char	tmprunfname[MAXPATHLEN + 1] ;
	char	tmpconfname[MAXPATHLEN + 1] ;
	char	tmpparamfname[MAXPATHLEN + 1] ;
	char	tmpmainfname[MAXPATHLEN + 1] ;
	char	tmpoutfname[MAXPATHLEN + 1] ;
	char	cmd[(MAXPATHLEN * 2) + 1] ;
	char	*cp, *sp ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("runlocal: entered slave=%d\n",nslave) ;
#endif


/* initialize the substitution array for 'subfile' */

	if ((rs = varsub_start(&vsh,0)) < 0)
		goto badinit ;

/* create a substitution array of the parameters */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("runlocal: creating substitution array\n") ;
#endif

	rs = 0 ;
	for (pp = g.plist ; (rs >= 0) && (pp != NULL) ; pp = pp->next) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("runlocal: name=%s value=%s\n",
	            pp->name,(pp->current)->value) ;
#endif

	    rs = varsub_add(&vsh,pp->name,-1,(pp->current)->value,-1) ;

	    if (rs >= 0) {

	        if (strcmp(pp->name,"TIMESTEP") == 0)
	            rs = varsub_add(&vsh,"STEP",-1,(pp->current)->value,-1) ;

	        else if (strcmp(pp->name,"TIMELEN") == 0)
	            rs = varsub_add(&vsh,"LENGTH",-1,(pp->current)->value,-1) ;

	    }

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("runlocal: added to VSA %d\n",rs) ;
#endif

	} /* end for (creating substitution array) */

	if (rs < 0) goto badvsa ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("runlocal: about to perform\n") ;
#endif

	rs = perform(&vsh,nslave,pid) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("runlocal: performed w/ rs=%d\n",rs) ;
#endif

/* now that all of the parameters substitutions are done, free the array */

	varsub_finish(&vsh) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("runlocal: returning OK\n") ;
#endif

	return rs ;

/* we got a bad VSA initialization (probably bad memory allocation) */
badinit:
#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("runlocal: bad VSA init w/ rs=%d\n",rs) ;
#endif

	return rs ;

/* we got a bad return from the VSA object (probably bad memory allocation) */
badvsa:
#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("runlocal: bad VSA w/ rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (runlocal) */

