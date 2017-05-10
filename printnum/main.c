/* main (printnum) */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdio.h>

#include	<localmisc.h>


/* external subroutines */

extern int	cfdecui(const char *,int,uint *) ;
extern int	isprintlatin(int) ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	int	rs ;
	int	i ;
	int	c ;

	for (i = 1 ; i < argc ; i += 1) {

	    if ((argv[i] != NULL) && (argv[i][0] != '\0')) {

		rs = cfdecui(argv[i],-1,&c) ;

		if (rs >= 0) {
		    if (isprintlatin(c)) {
	                fprintf(stdout,">%c<\n",c) ;
		    } else
		        fprintf(stderr,"not printable (%u)\n",c) ;
		} else
		    fprintf(stderr,"invalid number specified (%d)\n",rs) ;

	    }

	} /* end for */

	return 0 ;
}
/* end subroutine (main) */


