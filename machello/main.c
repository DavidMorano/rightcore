/* main */
/* lang=C++11 */


#include	<iostream>
#include	<iomanip>

#ifndef	TYPEDEF_CCHAR
#define	RTPEDEF_CCHAR
typedef const char	cchar ;
#endif

using namespace std ;

/* ARGSUSED */
int main(int argc,char **aargv,char **aenvv)
{
	cchar	**argv = (cchar **) aargv ;
	cchar	**envv = (cchar **) aenvv ;

	cout << "Hello world!\n" ;
	if (argc) {
	    int	ai ;
	    for (ai = 0 ; argv[ai] != NULL ; ai += 1) {
		cout << "a=" << argv[ai] << "\n" ;
	    }
	}
	return 0 ;
}
/* end subroutine (main) */


