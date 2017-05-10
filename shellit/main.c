/* shellit */

/* program to EXEC a program with the named arg0 */
/* last modified %G% version %I% */


/*
	90/11/01, David A­D­ Morano


*/


/**************************************************************************

	Execute as :

	$ shellit prog arg0 arg1 ...


**************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>

#include	<bfile.h>

#include	"localmisc.h"



/* local defines */

#define		VERSION		"0"



/* external subroutines */

extern char	*strbasename() ;






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile		errfile, *efp = &errfile ;

	char		*title ;
	char		newname[256] ;


	if ((rs = bopen(efp,BERR,"wca",0666)) < 0) return (rs) ;

	title = strbasename(argv[0]) ;

	sprintf(newname,"%s.sh",argv[0]) ;

	if (access(newname,X_OK) != 0) goto nofile ;

	execvp(newname,argv) ;

	bprintf(efp,"%s: program would not 'exec' correctly\n",title) ;

	goto badret ;

nofile:
	bprintf(efp,"%s: there was no file \"%s.sh\" to execute\n",
		title,argv[0]) ;

badret:
	bclose(efp) ;

	return BAD ;
}
/* end subroutine (main) */



