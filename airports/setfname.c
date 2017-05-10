/* setfname */

/* set a filename (?) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_GETEXECNAME	1		/* get the 'exec(2)' name */
#define	CF_CPUSPEED	1		/* calculate CPU speed */


/* revision history:

	= 89/03/01, David A­D­ Morano

	This subroutine was originally written.  


	= 98/06/01, David A­D­ Morano

	I enhanced the program a little to print out some other
	information.


	= 99/03/01, David A­D­ Morano

	I enhanced the program a little to to do something (I forget
	what).


	= 04/01/10, David A­D­ Morano

	The KSH program switched to using a fakey "large file" (64-bit
	fake-out mode) compilation mode on Solaris.  This required
	some checking to see if any references to 'u_stat()' had to be
	updated to work with the new KSH.  Although we call 'u_stat()'
	here, its structure is not passed to other subroutines expecting
	the regular 32-bit structure.


	= 05/04/20, David A­D­ Morano

	I changed the program so that the configuration file is consulted
	even if the program is not run in daemon-mode.	Previously, the
	configuration file was only consulted when run in daemon-mode.
	The thinking was that running the program in regular (non-daemon)
	mode should be quick.  The problem is that the MS file had to
	be guessed without the aid of consulting the configuration file.
	Although not a problem in most practice, it was not aesthetically
	appealing.  It meant that if the administrator changed the MS file
	in the configuration file, it also had to be changed by specifying
	it explicitly at invocation in non-daemon-mode of the program.
	This is the source of some confusion (which the world really
	doesn't need).	So now the configuration is always consulted.
	The single one-time invocation is still fast enough for the
	non-smoker aged under 40 ! :-) :-)


*/


/**************************************************************************

	This subroutine sets a filename in some way.


*****************************************************************************/


#if	defined(SFIO) || defined(KSHBUILTIN)
#undef	CF_SFIO
#define	CF_SFIO	1
#endif

#if	CF_SFIO
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<netdb.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<paramfile.h>
#include	<logfile.h>
#include	<msfile.h>
#include	<kinfo.h>
#include	<lfm.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"shio.h"


/* local defines */

#define	VBUFLEN		(2 * MAXPATHLEN)
#define	EBUFLEN		(3 * MAXPATHLEN)

#define	DEBUGFNAME	"/tmp/msu.deb"

#ifndef	DEVTTY
#define	DEVTTY		"/dev/tty"
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif



/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matstr(const char **,char *,int) ;
extern int	matstr2(const char **,char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_loga(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

#if	(! CF_SFIO)
extern char	**environ ;
#endif


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int setfname(pip,fname,ebuf,el,f_def,dname,name,suf)
struct proginfo	*pip ;
char		fname[] ;
const char	ebuf[] ;
const char	dname[], name[], suf[] ;
int		el ;
int		f_def ;
{
	int	rs = 0 ;
	int	ml ;

	char	tmpname[MAXNAMELEN + 1], *np ;


	if ((f_def && (ebuf[0] == '\0')) ||
	    (strcmp(ebuf,"+") == 0)) {

	    np = (char *) name ;
	    if ((suf != NULL) && (suf[0] != '\0')) {

	        np = (char *) tmpname ;
	        mkfnamesuf1(tmpname,name,suf) ;

	    }

	    if (np[0] != '/') {

	        if ((dname != NULL) && (dname[0] != '\0')) {
	            rs = mkpath3(fname,pip->pr,dname,np) ;

	        } else
	            rs = mkpath2(fname, pip->pr,np) ;

	    } else
	        rs = mkpath1(fname, np) ;

	} else if (strcmp(ebuf,"-") == 0) {

	    fname[0] = '\0' ;

	} else if (ebuf[0] != '\0') {

	    np = (char *) ebuf ;
	    if (el >= 0) {

	        np = tmpname ;
	        ml = MIN(MAXPATHLEN,el) ;
	        strwcpy(tmpname,ebuf,ml) ;

	    }

	    if (ebuf[0] != '/') {

	        if (strchr(np,'/') != NULL) {

	            rs = mkpath2(fname,pip->pr,np) ;

	        } else {

	            if ((dname != NULL) && (dname[0] != '\0')) {
	                rs = mkpath3(fname,pip->pr,dname,np) ;

	            } else
	                rs = mkpath2(fname,pip->pr,np) ;

	        } /* end if */

	    } else
	        rs = mkpath1(fname,np) ;

	} /* end if */

	return rs ;
}
/* end subroutine (setfname) */



