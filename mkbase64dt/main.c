/* main */


#define	CF_DEBUGS	0



#include	<sys/types.h>
#include	<stdio.h>

#include	<localmisc.h>


/* external variables */

extern unsigned char	base64_et[] ;


/* exported subroutines */


int main()
{
	int	i, j ;
	int	c ;


	c = 0 ;
	for (i = 0 ; i < 256 ; i += 1) {

	    fprintf(stdout,((c == 0) ? "\t" : " ")) ;

#if	CF_DEBUGS
	fprintf(stderr,"i=%d\n",i) ;
#endif

	    for (j = 0 ; j < 64 ; j += 1) {

	        if (i == base64_et[j])
	            break ;

	    }

	    if (i == '=') {
	        fprintf(stdout,"0x%02X,",0xFE) ;

	    } else if (j >= 64) {
	        fprintf(stdout,"0x%02X,",0xFF) ;

	    } else
	        fprintf(stdout,"0x%02X,",j) ;

	    c += 1 ;
	    if (c >= 8) {

	        c = 0 ;
	        fprintf(stdout,"\n") ;

	    }

	} /* end for (outer) */

	fclose(stdout) ;

#if	CF_DEBUGS
	fclose(stderr) ;
#endif

	return 0 ;
}
/* end subroutine (main) */



