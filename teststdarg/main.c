/* main */


#define	CF_VARG		1		/* use 'varg(3dam)' */
#define	CF_STDARG	1		/* force include of 'stdarg.h' */
#define	CF_OLDARG	0		/* use old-style arguments */
#define	CF_FORWARD	1


#include	<envstandard.h>

#include	<stdarg.h>

#include	<stdio.h>



/* forward reference */

static int	sub(FILE *,...) ;




int main()
{
	int	a, b, c ;


	a = 1 ;
	b = 2 ;

	c = sub(stdout,1,2) ;

	fprintf(stdout,"c=%d\n",c) ;

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int sub(FILE *fp,...)
{
	va_list	ap ;

	int	i ;
	int	a, b ;


	va_begin(ap,fp) ;

	for (i = 0 ; i < 2 ; i += 1) {

	    fprintf(fp,"ap=%08lx\n",ap) ;

	    if (i == 0) {
	        a = va_arg(ap,int) ;
	    }else
	        b = va_arg(ap,int) ;

	} /* end for */

	va_end(ap) ;

	return (a + b) ;
}
/* end subroutine (sub) */



