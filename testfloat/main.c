

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<stdio.h>

#include	"localmisc.h"



extern int	sub(FILE *,double) ;





int main()
{
	FILE	*fp ;

	double	v, two = 0.2 ;
	double	e ;

	int	fa, fs ;
	int	i ;

	char	*cp ;


	fp = stdout ;


	cp = ecvt(two,25,&fa,&fs) ;

	fprintf(fp,"%s n=%d fa=%d fs=%d\n",cp,strlen(cp),fa,fs) ;

	e = 1.0e-20 ;
	for (i = 0 ; i < 30 ; i += 1) {

	v = two + e ;
	cp = ecvt(v,25,&fa,&fs) ;

	fprintf(fp,"i=%d %s n=%d fa=%d fs=%d\n",
		i,cp,strlen(cp),fa,fs) ;

		e = e * 2 ;

	} /* end for */
	

	sub(fp,0.2) ;



	fclose(stdout) ;

	return 0 ;
}


