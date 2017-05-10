/* runremote */

/* execute the ADVICE program remotely */


#define	CF_DEBUG	0


/* revision history:

	= David A.D. Morano, October 1993

	This program was originally written.

	= David A.D. Morano, Febuary 1996

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



/* local defines */

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

extern void	fixdisplay() ;


/* forward references */

void	int_signal() ;


/* external variables */

int	environ ;


/* global data */

extern struct global	g ;



/* local data structures */




int runremote(slave,host)
int	slave ;
char	host[] ;
{
	bfile			rinfile, *fpa[3] ;

	pid_t			pid ;

	struct paramname	*pp ;

	struct paramvalue	*vp ;

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
	if (g.debuglevel > 1) debugprintf(
	"runremote: entered slave=%d host=%s\n",slave,host) ;
#endif

/* spawn a remote slave of ourselves */

	for (i = 0 ; i < 3 ; i += 1) fpa[i] = NULL ;

	fpa[0] = &rinfile ;

	buf[0] = '\0' ;
	if (g.debuglevel > 0)
		sprintf(buf,"-D=%d",g.debuglevel) ;

#if	CF_DEBUG
	sprintf(cmd,"/home/dam/src/runadvice_1/runadvice.x -SLAVE=%d %s",
		slave,buf) ;
#else
	sprintf(cmd,"runadvice -SLAVE=%d %s",
		slave,buf) ;
#endif

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	"runremote: about to execute >\n%s\n",cmd) ;
#endif

	if ((rs = bopenremote(fpa,g.environ,host,cmd)) < 0) return rs ;

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	"runremote: remote executed w/ rs=%d\n",rs) ;
#endif

#if	CF_DEBUG && 0
	system("rsh -n hodid ps -fu dam") ;

	system("rsh -n hodif ps -fu dam") ;
#endif

	pid = rs ;

/* print some junk for the far end to synchronize up (just for fun) */

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	"runremote: about to print 1\n") ;
#endif

	    bprintf(fpa[0],"### hello there\nSTART\n") ;

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	"runremote: about to flush\n") ;
#endif

	if ((rs = bflush(fpa[0])) < 0) goto badpipe ;

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	"runremote: flushed f_pipe=%d\n",g.f_pipe) ;
#endif

/* send over our log ID */

	rs = bprintf(fpa[0],"%s\n",g.logid) ;


/* send over the ADVICE program to execute */

	rs = bprintf(fpa[0],"%s\n",g.prog_advice) ;

	if (rs < 0) goto badpipe ;

/* send over the file names */

	    rs = bprintf(fpa[0],"%s\n",g.runfname) ;

	    rs = bprintf(fpa[0],"%s\n",g.outfname) ;

	    rs = bprintf(fpa[0],"%s\n",g.confname) ;

	    rs = bprintf(fpa[0],"%s\n",g.paramfname) ;

	    rs = bprintf(fpa[0],"%s\n",g.mainfname) ;


/* send over the substitution parameters */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("runremote: sending over the parameters \n") ;
#endif

	bprintf(fpa[0],"%d\n",g.nparams) ;

	for (pp = g.plist ; pp != NULL ; pp = pp->next) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("runremote: name=%s value=%s\n",
	            pp->name,(pp->current)->value) ;
#endif

	    rs = bprintf(fpa[0],"%s=%s\n",pp->name,(pp->current)->value) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("runremote: added to VSA len=%d\n",rs) ;
#endif

	} /* end for (creating substitution array) */


/* flush it, send it over, what ever ! */
exit:
	bclose(fpa[0]) ;

	return (rs < 0) ? rs : pid ;

badpipe:

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	"runremote: bad pipe rs=%d\n",rs) ;
#endif

	goto exit ;
}
/* end subroutine (runremote) */



