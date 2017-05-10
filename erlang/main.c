/* main */

/* deal with digital line terminations */



/* revistion history :

	= August 1988, David A­D­ Morano


*/


/* 

*/




#include	<sys/types.h>
#include	<fcntl.h>
#include	<math.h>

#include	<bfile.h>

#include	"localmisc.h"



#define		DATA_EOF	0x1A

#define		BUFLEN		0x1000
#define		LINELEN		100

#define		TERMTIMEOUT	10L



long int factorial() ;





int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		error, *efp = &error ;
	bfile		output, *ofp = &output ;

	double	a = 3.0 ;
	double	pb, pb_n, pb_d ;
	double	pq ;
	double	roe = (4.0 / 5.0) ;
	double	p0, p0_a, p0_b ;
	double	nq, dq ;
	double	lam = 0.1 ;


	long	int	i, n, m = 5 ;
	int		rs ;


	rs = bopen(efp,BFILE_STDERR,"dwca",0666) ;


	rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0) goto no_open ;

/* get the values */


	p0_a = 0.0 ;
	for (n = 0 ; n < m ; n += 1)
		p0_a += (pow((m * roe),(double) n) / ((double) factorial(n))) ;

	p0_b = 
	  pow((m * roe),(double) m) / ((double) factorial(m)) / (1 - roe) ;

	p0 = 1.0 / (p0_a + p0_b) ;

	pq = p0 * pow((m * roe),(double) m) / ((double) factorial(m)) /
		(1.0 - roe) ;
	
	nq = pq * roe / (1.0 - roe) ;

	dq = nq / lam ;


	bprintf(ofp,"dq=%e\n",dq) ;


exit:
	bclose(ofp) ;

	bclose(efp) ;

	return OK ;

no_open:
	bprintf(efp,"can't open output file %d\n",rs) ;

bad_exit:
	bclose(efp) ;

	return BAD ;
}


long int factorial(n)
long int	n ;
{

	if (n <= 1)  return 1 ;

	return (n * factorial(n - 1)) ;
}



