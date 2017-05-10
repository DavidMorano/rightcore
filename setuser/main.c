/* setuser */

/* program to exec a program with the named arg0 */
/* last modified %G% version %I% */


/* revision history:

	= 90/11/01, David A­D­ Morano

	This subroutine was originally written.


*/


/**************************************************************************

	Program to set the real UID and GID to that of the user name
	given as the first argument.

		setuser user_name prog arg0 arg1 ...

	This program will 'exec' to the program given.


**************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<pwd.h>

#include	<bfile.h>

#include	"localmisc.h"



/* local defines */

#define		VERSION		"0"



/* external subroutines */

extern struct passwd	*getpwnam() ;


/* forward references */





int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct passwd	*pwp ;

#ifdef	COMMENT
	struct ustat	ss ;
#endif

	bfile		errfile, *efp = &errfile ;

	int		rs ;

	char		*title ;


	if ((rs = bopen(efp,BERR,"wca",0666)) < 0) return BAD ;

	title = argv[0] ;
	if (argc < 3) goto notenough ;


/* check arguments */

#ifdef	COMMENT
	stat(argv[2],&ss) ;

	oldmode = (int) ss.st_mode ;
#endif

/* get the UID and GID which corresponds to the user name given */

	pwp = getpwnam(argv[1]) ;

	if (pwp == ((struct passwd *) 0)) {

	    bprintf(efp,
		"%s: user \"%s\" could not be found\n",title,argv[1]) ;

	    goto badret ;
	}

	rs = setgid(pwp->pw_gid) ;

	rs |= setuid(pwp->pw_uid) ;

	if (rs != 0) {

	    bprintf(efp,
		"%s: could not change IDs for user \"%s\"\n",title,argv[1]) ;

	    goto badret ;
	}

	execvp(argv[2],argv + 2) ;

	bprintf(efp,"%s: program would not 'exec' correctly\n",title) ;

	goto badret ;

notenough:
	bprintf(efp,"%s: not enough arguments given\n",title) ;

	bprintf(efp,"usage: %s file1 [file2] [-b]\n",title) ;

badret:
	bclose(efp) ;

	return BAD ;
}
/* end subroutine (main) */


