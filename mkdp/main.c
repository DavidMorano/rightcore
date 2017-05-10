/* main */



#include	<stdio.h>



int main()
{
	int	j ;
	int	b ;
	int	addr, size ;
	int	a, s ;
	int	dp ;


	fprintf(stdout,"static unsigned short dps[] = {\n") ;

	j = 0 ;
	for (addr = 0 ; addr < 8 ; addr += 1) {

	    for (size = 0 ; size < 16 ; size += 1) {

	        dp = 0 ;
	        a = addr ;
	        for (s = 0 ; s < size ; s += 1) {

	            b = a + s ;
	            if (b >= 8) {

	                dp = 0xFFFF ;
	                break ;

	            } else
	                dp |= (1 << b) ;

	        }

	        if ((j % 8) == 0)
	            fprintf(stdout,"\t") ;

	        else
	            fprintf(stdout," ") ;

	        fprintf(stdout,"0x%04x,",dp) ;

	        if ((j % 8) == 7)
	            fprintf(stdout,"\n") ;

	        j += 1 ;

	    } /* end for */

	} /* end for */

	fprintf(stdout,"} ;\n") ;

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



