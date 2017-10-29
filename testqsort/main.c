/* main */


#define	CF_DEBUGS	1
#define	F_VSTRSORT	0	/* use 'vstrsort()' instead of 'qsort(3c)' */
#define	F_VSTRCMP	0	/* which sort function */


/******************************************************************************

	This little program shows a bad (core-dump quality) bug in
	the Solaris 'qsort(3c)' subroutine !


******************************************************************************/


#include	<sys/types.h>
#include	<stdlib.h>
#include	<stdio.h>

#include	"config.h"
#include	"defs.h"



/* external variables */

extern int	cfdeci(const char *,int,int *) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;


/* local variables */

static const char	*array[] = {
	"_=/bin/cat",
	"PATH=/bin",
	NULL
} ;






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	int	rs, i ;
	int	fd_debug ;
	int	(*fn)(const void *,const void *) ;

	char	*cp ;


#if	CF_DEBUGS
	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;
#endif /* CF_DEBUGS */


	fprintf(stdout,"unsorted\n") ;

	for (i = 0 ; array[i] != NULL ; i += 1)
		fprintf(stdout,"a[%d]=>%s<\n",i,array[i]) ;


#if	F_VSTRCMP
	fn = vstrcmp ;
#else
	fn = vstrkeycmp ;
#endif

#if	F_VSTRSORT
	vstrsort(array,2,fn) ;
#else
	qsort(array,2,sizeof(char *),fn) ;
#endif


	fprintf(stdout,"sorted\n") ;

	for (i = 0 ; array[i] != NULL ; i += 1)
		fprintf(stdout,"a[%d]=>%s<\n",i,array[i]) ;


	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



