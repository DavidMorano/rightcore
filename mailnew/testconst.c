/* main (testconst) */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<stddef.h>
#include	<stdio.h>
#include	<localmisc.h>

#if	defined(_WCHAR_T)
#define	WCHAR	1
#else
#define	WCHAR	0
#endif

typedef int	xchar_t ;

int main(int argc,cchar **argv,cchar **envv)
{
	const int64_t	a = 0 ;
	const xchar_t	j = 1 ;
	wchar_t		b ;

	printf("%lld wchar=%u\n",a,WCHAR) ;

	return 0 ;
}
/* end subroutine (main) */


