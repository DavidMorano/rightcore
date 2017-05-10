/* main (testmalloc) */


#define	CF_VALLOC	0		/* call 'valloc(3c)' */



#include	<sys/types.h>
#include	<stdlib.h>
#include	<malloc.h>
#include	<stdio.h>



#define	AMOUNT	100




int main()
{
	struct mallinfo	mi ;

	char	*p ;
	char	*vp ;


	mi = mallinfo() ;
	fprintf(stderr,"in-use small    %lu\n",
		mi.usmblks) ;
	fprintf(stderr,"in-use large    %lu\n",
		mi.uordblks) ;
	fprintf(stderr,"in-use total    %lu\n",
		(mi.usmblks + mi.uordblks)) ;

	p = malloc(AMOUNT) ;

#if	CF_VALLOC
	vp = valloc(AMOUNT) ;
#endif

	mi = mallinfo() ;
	fprintf(stderr,"in-use small    %lu\n",
		mi.usmblks) ;
	fprintf(stderr,"in-use large    %lu\n",
		mi.uordblks) ;
	fprintf(stderr,"in-use total    %lu\n",
		(mi.usmblks + mi.uordblks)) ;

#if	CF_VALLOC
	free(vp) ;
#endif

	free(p) ;

	mi = mallinfo() ;
	fprintf(stderr,"in-use small    %lu\n",
		mi.usmblks) ;
	fprintf(stderr,"in-use large    %lu\n",
		mi.uordblks) ;
	fprintf(stderr,"in-use total    %lu\n",
		(mi.usmblks + mi.uordblks)) ;

	fclose(stderr) ;

	return 0 ;
}
/* end subroutine (main) */



