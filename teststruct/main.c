/* main */


#include	<sys/types.h>

#include	<bfile.h>



/* external subroutines */

extern struct thing	sub() ;



/* private module data structures */

struct thing {
	int	a, b ;
} ;






int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile	outfile, *ofp = &outfile ;

	struct thing	mine = { 0, 0 } ;
	struct thing	yours = { 0, 0 } ;

	int	rs, i ;


	(void) bopen(ofp,BFILE_STDOUT,"dwct",0666) ;


	bprintf(ofp,"entered\n") ;

	mine = sub(1,2) ;

	bprintf(ofp,"mine= %d %d \n",mine.a,mine.b) ;

	yours = sub(3,4) ;

	bprintf(ofp,"mine= %d %d \n",mine.a,mine.b) ;


	bclose(ofp) ;

	return 0 ;
}
/* end subroutine (main) */



