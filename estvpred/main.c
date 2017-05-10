/* main */


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




extern int	cfdeci(const char *,int,int *) ;
extern int	flbsi(uint) ;


/* forward references */

static int costvpred(int,int) ;





int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	int	rs ;
	int	nentries ;		/* VP table entries */
	int	a ;

	int	decoder ;
	int	comparator ;
	int	entry ;
	int	ebuffer ;
	int	tagbits ;
	int	operand ;
	int	value ;
	int	stride ;
	int	tag, counter ;
	int	total ;
	int	outarray ;
	int	vpred ;
	int	add32 ;
	int	n ;



	nentries = N_ENTRIES ;
	if ((argc > 1) && (argv[1] != NULL) && (argv[1][0] != '\0')) {

		rs = cfdeci(argv[1],-1,&nentries) ;

		if (rs < 0)
			goto bad ;

	}


	fprintf(stdout,"rows=%d entries=%d\n",N_ROWS,nentries) ;


	a = 32 ;
	vpred = costvpred(a,nentries) ;

	total = vpred * N_ROWS ;
	fprintf(stdout,"abits=%d vpred=%d total=%d\n",a,vpred,total) ;


	a = 64 ;
	vpred = costvpred(a,nentries) ;

	total = vpred * N_ROWS ;
	fprintf(stdout,"abits=%d vpred=%d total=%d\n",a,vpred,total) ;



	fclose(stdout) ;

ret1:
	fclose(stderr) ;

	return 0 ;

bad:
	fprintf(stderr,"bad argument specified\n") ;

	goto ret1 ;
}
/* end subroutine (main) */


int costvpred(a,nentries)
int	a, nentries ;
{
	int	decoder ;
	int	comparator ;
	int	entry ;
	int	ebuffer ;
	int	tagbits ;
	int	operand ;
	int	value ;
	int	stride ;
	int	tag, counter ;
	int	total ;
	int	outarray ;
	int	vpred ;
	int	add32 ;
	int	n ;


	n = flbsi(nentries) ;

	tagbits = a - 2 - n ;


#ifdef	COMMENT
	decoder = (nentries * NB_NAND) + (nentries / 2 ) ;
#else
	decoder = (nentries * NB_NAND) + ((nentries / 2 ) * NB_INV) ;
#endif

	comparator = tagbits * (4 * NB_NAND) ;


	tag = tagbits * NB_BIT ;

	counter = (2 * NB_BIT) + (8 * NB_NAND) ;
	value = a * NB_BIT ;
	stride = N_STRIDE * NB_BIT ;
	operand = value + stride + counter ;

	entry = operand * N_OP + tag ;

	ebuffer = (2 * a * N_OP) ;

	add32 = a * (5 * NB_NAND) ;

	outarray = decoder ;

	vpred = 0 ;
	vpred = decoder + outarray + ebuffer + comparator + (nentries * entry) ;
	vpred += (2 * add32) ;

	return vpred ;
}



