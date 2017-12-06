/* main (benford) */



#include	<stdio.h>


int main()
{
	double		r ;
	int		rs ;
	int		i ;
	int		p[10] ;
	int		sum ;
	int		size, iw ;
	short		sw ;
	char		digbuf[24] ;
	char		*cp ;

	size = 10 * sizeof(int) ;
	(void) memset(p,0,size) ;

	while ((rs = fread(&sw,sizeof(short),1,stdin)) > 0) {

		iw = sw ;
		ctdeci(digbuf,20,iw) ;

		cp = digbuf ;
		while (*cp == '\0') {
			cp += 1 ;
		}

		iw = (*cp - '0') & 0xff ;
		p[iw] += 1 ;

	} /* end while */

	sum = 0 ;
	for (i = 0 ; i < 10 ; i += 1) {
		sum += p[i] ;
	}

	printf("sum=%d\n",sum) ;

	for (i = 0 ; i < 10 ; i += 1) {
		r = ((double) p[i]) / ((double) sum) ;
		printf("c[%d]=%d p[%d]=%6.3f\n",i,p[i],i,r) ;
	}

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */


