


#include	<stdio.h>



int printer(s)
const char	s[] ;
{
	int	rs ;


	rs = fprintf(stdout,">%s<\n",s) ;

	return rs ;
}
/* end subroutine (printer) */


