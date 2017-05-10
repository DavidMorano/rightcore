/* malloctest */


#define	CF_DEBUG	0



#include	<envstandards.h>	/* MUST be first to configure */

#include	<stdlib.h>




char *malloctest(size)
int	size ;
{
	char	*cp ;


#if	CF_DEBUG
	debugprintf("malloctest: malloc test, size=%d\n",size) ;

	while (size > 0) {

	debugprintf("malloctest: trying size=%d\n",size) ;

	cp = malloc(size) ;

	if (cp != NULL) {

	    debugprintf("malloctest: malloc worked !\n") ;

	    free(cp) ;

	} else {

	    debugprintf("malloctest: malloc failed !\n") ;

		break ;

	}
		size -= 1 ;

	} /* end while */
#else
	cp = "malloctest: good" ;
#endif

	return cp ;
}
/* end subroutine (malloctest) */



