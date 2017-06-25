/* main (testmod) */
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	<mp.h>
int main(int argc,const char **argv,const char **envv)
{
	MINT		*mnp, q ;
	long long int	n ;
	int		m = 35 ;
	short		r ;


	if (argc > 1) {
		n = atoll(argv[1]) ;
		printf("n=%llu mod35=%u\n",n,(int) (n%m)) ;
	}

	{
		char	*s = "bf6306521cb5ac214c87353d3a35b469" ;
		short	d = (short) m ;
		memset(&q,0,sizeof(MINT)) ;
		mnp = mp_xtom(s) ;
		mp_sdiv(mnp,d,&q,&r) ;
		printf("r=%hu\n",r) ;
	}

	return 0 ;
}
/* end subroutine (main) */

