/* other */



#include	<stdio.h>



#define	MAXNAME		32
#define	NCOL		7



typedef char	name[] ;




int other(a)
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
/* end subroutine (other) */



