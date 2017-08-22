/* main (TESTASSIGN) */

/* test features of ANSI assignment */


/* revision history:

	= 1999-03-01, David A­D­ Morano

*/

#include	<sys/types.h>
#include	<limits.h>
#include	<stdio.h>


int main()
{
	const signed char	sch = '¿' ;
	const unsigned char	uch = '¿' ;
	int		ch = 0xff ;
	unsigned int	ui ;

	ui = ch ;
	fprintf(stdout,"ui=%08x\n",ui) ;

	ch = sch ;
	fprintf(stdout,"sch=%08x\n",ch) ;

	ch = uch ;
	fprintf(stdout,"uch=%08x\n",ch) ;
	fprintf(stdout,"uch=%08x\n",uch) ;

	ch = 0xff ;
	fprintf(stdout,"greater=%u\n",(ch > uch)) ;

	{
	    char tch = 0x80 ;
	    fprintf(stdout,"greater=%u\n",(tch > uch)) ;
	}

	{
	    signed char		t1c = 0x81 ;
	    unsigned char	t2c = 0x80 ;
	    fprintf(stdout,"greater=%u\n",(t1c > t2c)) ;
	}

	{
	    int	t1 = (uint) sch ;
	    int	t2 = (sch & 0xff) ;
	    uint t3= (uint) sch ;
	    fprintf(stdout,"ti=%08x t2=%08x t3=%08x\n",t1,t2,t3) ;
	}

	return 0 ;
}
/* end subroutine (main) */


