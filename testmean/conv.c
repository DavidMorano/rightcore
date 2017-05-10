/* conv */



#define	CF_DEBUGS	1
#define	F_EXPLICIT	1



#include	<sys/types.h>
#include	<stdlib.h>
#include	<math.h>
#include	<stdio.h>

#include	"localmisc.h"



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;





int main()
{
	double	x ;

	ULONG	ulw ;

	LONG	lw ;

	uint	uiw ;

	int	i, iw ;
	int	err_fd ;

	char	*cp ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;



	for (i = 0 ; i < 4 ; i += 1) {

#if	F_EXPLICIT
	    ulw = (ULONG) i ;
	    lw = (LONG) i ;
	    uiw = (uint) i ;
#else
	    ulw = i ;
	    lw = i ;
	    uiw = i ;
#endif

	    iw = i ;

	    printf("\ni=%d\n\n",i) ;

	    x = (double) ulw ;
	    printf("ulw=%llu ulw=%016llx x=%8.2f\n",
	        ulw,ulw,x) ;

#if	CF_DEBUGS
	debugprintf("conv: x=%8.2f\n",x) ;
#endif

	    x = (double) lw ;
	    printf("lw=%lld lw=%016llx x=%8.2f\n",
	        lw,lw,x) ;

	    x = (double) uiw ;
	    printf("uiw=%u %08x x=%8.2f\n",
	        uiw,uiw,x) ;

	    x = (double) iw ;
	    printf("iw=%d iw=%08x x=%8.2f\n",
	        iw,iw,x) ;

	}

	fclose(stdout) ;

	return 0 ;
}



