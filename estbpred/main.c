/* main */

/* estimate for branch prediction */


#include	<sys/types.h>
#include	<stdio.h>

#include	"localmisc.h"



#define	N_ROWS		32
#define	N_ENTRIES	64
#define	N_OP		2
#define	N_STRIDE	8

#define	NB_BIT	8
#define	NB_NAND	4
#define	NB_INV	2
#define	NB_XOR	(4 * NB_NAND)



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	flbsi(uint) ;
extern int	costbpred(int) ;




int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	int	rs ;
	int	rows ;

	int	nand = 4 ;

	int	cs, cd, cc, cm, csh, ci, ca ;
	int	a, h, j, k, s, p ;
	int	i ;
	int	cost ;
	int	cost_bht, cost_pht ;


	rows = 32 ;

	a = 32 ;
	cost = costbpred(a) ;

	fprintf(stdout,
		"32-bit address cost=%d total=%d\n",cost,(cost * rows)) ;


	a = 64 ;
	cost = costbpred(a) ;

	fprintf(stdout
		,"64-bit address cost=%d total=%d\n",cost,(cost * rows)) ;


	fclose(stdout) ;

ret1:
	fclose(stderr) ;

	return 0 ;

bad:
	fprintf(stderr,"bad argument specified\n") ;

	goto ret1 ;
}
/* end subroutine (main) */


int costbpred(a)
int	a ;
{
	int	nand = 4 ;

	int	cs, cd, cc, cm, csh, ci, ca ;
	int	h, j, k, s, p ;
	int	i ;
	int	cost ;
	int	cost_bht, cost_pht ;


	j = 0 ;
	h = 1024 ;
	k = 12 ; /* 4096 */
	s = 2 ;
	p = 4096 ;
	i = flbsi(h) ;

	cs = 8 ;
	cd = 10 ;	/* decoder transitors per bit */
	cc = (4 * nand) ;
	cm = cd ;
	csh = (2 * nand) ;
	ci = (4 * nand) ;
	ca = (6 * nand) ;


	cost_bht = h * ((a + 2 * j + k + 1 - i) * cs + k * csh) ;
	cost_pht = (1 << k) * (s * cs + cd) ;
	cost = cost_bht + cost_pht ;

	return cost ;
}



