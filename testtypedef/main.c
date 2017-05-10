/* main */



#include	<stdio.h>



#define	MAXNAME		32
#define	NCOL		7



typedef char	name[MAXNAME] ;



/* extern subroutines */

extern int	other(name) ;


/* forward references */

static int	sub(name) ;




int main()
{
	name	a ;

	int	rs ;
	int	i, j ;


	for (i = 0 ; i < MAXNAME ; i += 1)
	    a[i] = i ;


	j = 0 ;
	for (i = 0 ; i < MAXNAME ; i += 1) {

	    if ((j % NCOL) == 0)
	        fprintf(stdout," ") ;

	    fprintf(stdout," %2d",a[i]) ;

	    if ((j % NCOL) == (NCOL - 1))
	        fprintf(stdout,"\n") ;

	    j = (j + 1) ;

	} /* end for */

	if ((j % NCOL) != 0)
	    fprintf(stdout,"\n") ;


	fprintf(stdout,"\n") ;

	rs = sub(a) ;


	fprintf(stdout,"\n") ;

	rs = other(a) ;


	fclose(stdout) ;

	return ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int sub(a)
name	a ;
{
	int	i, j ;


	j = 0 ;
	for (i = 0 ; i < MAXNAME ; i += 1) {

	    if ((j % NCOL) == 0)
	        fprintf(stdout," ") ;

	    fprintf(stdout," %2d",a[i]) ;

	    if ((j % NCOL) == (NCOL - 1))
	        fprintf(stdout,"\n") ;

	    j = (j + 1) ;

	} /* end for */

	if ((j % NCOL) != 0)
	    fprintf(stdout,"\n") ;


	return j ;
}
/* end subroutine (sub) */



