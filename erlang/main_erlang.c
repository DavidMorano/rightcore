/* deal with digital line terminations */

/*

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



long int factorial() ;



int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		error, *efp = &error ;
	bfile		output, *ofp = &output ;

	double	a = 3.0 ;
	double	pb, pb_n, pb_d ;

	long	int	n, m ;
	int		rs ;


	rs = bopen(efp,BFILE_STDERR,"dwca",0666) ;


	rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0) goto no_open ;

/* get the values */

	for (m = 0 ; m < 1000 ; m += 1) {

	pb_n = pow(a,(double) m) / ((double) factorial(m)) ;

	pb_d = 0.0 ;
	for (n = 1 ; n <= m ; n += 1)
		pb_d += pow(a,(double) n) / ((double) factorial(n)) ;

	pb = pb_n / pb_d ;

	if (pb < 0.01) break ;

	}


	bprintf(ofp,"m=%ld\n",m) ;


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



