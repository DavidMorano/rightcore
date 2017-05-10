/* testfd */

#include	<stdio.h>

int main()
{
	const char	*cp = "hello world!\n" ;
	int	cl ;
	int	rs ;

	cl = strlen(cp) ;
	rs = u_write(6,cp,cl) ;

	fprintf(stderr,"testfd: rs=%d\n",rs) ;

	return 0 ;
}


