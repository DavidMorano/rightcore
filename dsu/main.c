/* main */

/* program to exec a program with the named arg0 */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_USER		1


/* revision history:

	= 90/11/01, David A­D­ Morano

	This program was originally written.


	= 96/01/10, David A­D­ Morano

	I fixed a potentially fatal bug when incorrect arguments
	are given !


*/



/**************************************************************************

	Execute as :

		dsu prog arg0 arg1 ...



**************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<bfile.h>

#include	"config.h"
#include	"localmisc.h"



/* external subroutines */

extern char	*strbasename(char *) ;


/* external variables */

extern char	*users[] ;


/* forward references */

static int	userok() ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct passwd	*pp ;
	bfile		errfile, *efp = &errfile ;

#ifdef	COMMENT
	struct ustat	ss ;
#endif

	uid_t		uid, euid ;

	int		rs, i ;
	int		err_fd ;

	char		*progname ;
	char		*cp ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;


	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"wca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;


#if	CF_DEBUGS
	debugprintf("main: progname=%s\n",progname) ;
#endif


/* check arguments */

	if (argc < 3) 
		goto notenough ;

	if ((argv[1] == NULL) || (argv[1][0] == '\0'))
		goto badarg ;

	if ((argv[2] == NULL) || (argv[2][0] == '\0'))
		goto badarg ;


#ifdef	COMMENT
	stat(argv[2],&ss) ;
	oldmode = (int) ss.st_mode ;
#endif

#ifdef	CF_USER
	if ((euid = geteuid()) == 0) {

		char	username[USERNAMELEN + 1] ;


		uid = getuid() ;
		getusername(username,USERNAMELEN,uid) ;

		if (! userok(users,username))
			seteuid(uid) ;

	} /* end if */
#endif /* CF_USER */

#if	CF_DEBUGS
	debugprintf("main: execvp file=%s\n") ;
	for (i = 0 ; argv[i + 2] != NULL ; i += 1) {
	    debugprintf("main: argv[%d]=%s\n",i,argv[i + 2]) ;
	}
#endif

	rs = u_execve(argv[1],(argv+2),envv) ;

#if	CF_DEBUGS
	debugprintf("main: execvp rs=%d\n",rs) ;
#endif

/* we are bad already ! */

	bprintf(efp,
		"%s: program \"%s\" would not 'exec' correctly (rs %d)\n",
		progname,argv[1],rs) ;

	goto badret ;

notenough:
	bprintf(efp,"%s: not enough arguments given\n",
		progname) ;

	goto badusage ;

badarg:
	bprintf(efp,"%s: a bad (NULL, whatever !) argument was given\n",
		progname) ;

	goto badusage ;

badusage:
	bprintf(efp,"%s: USAGE> %s program [arg0 [args ...]]\n",
		progname,progname) ;

badret:
	bclose(efp) ;

	return BAD ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int userok(ua,username)
char	*ua[] ;
char	username[] ;
{
	int	i ;


	for (i = 0 ; ua[i] != NULL ; i += 1) {

		if (strcmp(username,ua[i]) == 0)
			return TRUE ;

	} /* end for */

	return FALSE ;
}
/* end subroutine (userok) */



