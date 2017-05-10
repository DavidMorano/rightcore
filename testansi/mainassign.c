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
	signed char	sch = '¿' ;
	unsigned char	uch = '¿' ;
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

	sch = 0x80 ;
	fprintf(stdout,"greater=%u\n",(sch > uch)) ;

	sch = 0x81 ;
	uch = 0x80 ;
	fprintf(stdout,"greater=%u\n",(sch > uch)) ;

	return 0 ;
}
/* end subroutine (main) */


