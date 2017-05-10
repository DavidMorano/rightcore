/* main */


/******************************************************************************

	This program prints out all of the service entries in the
	system 'services' database.


******************************************************************************/



#include	<sys/types.h>
#include	<netdb.h>
#include	<stdio.h>




int main()
{
	struct servent	*sep ;

	int	i ;

	char	*cp ;


	setservent(1) ;

	while ((sep = getservent()) != NULL) {

	    fprintf(stdout,"%-16s %-8s %5u\n",
	        sep->s_name,sep->s_proto,ntohs(sep->s_port)) ;

	    if (sep->s_aliases != NULL) {

	        for (i = 0 ; sep->s_aliases[i] != NULL ; i += 1) {

	            fprintf(stdout,"  %-16s\n",
	                sep->s_aliases[i]) ;

	        } /* end for */

	    } /* end if (aliases) */

	} /* end while */

	endservent() ;

	fflush(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



