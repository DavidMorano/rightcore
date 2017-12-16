/* testsub */

#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdio.h>

#include	<localmisc.h>


/* forward references */

static int foo(void) ;


/*ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	int		a = 0 ;
	ushort		us = 256 ;
	uchar		uc = 130 ;

	printf("main: %d\n",(us-uc)) ;

	foo() ;
	foo() ;
	a = foo() ;
	printf("main: a=%d\n",a) ;

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int foo(void)
{
	static a = 0 ;
	a += 1 ;
	return a ;
}
/* end subroutine (foo) */


