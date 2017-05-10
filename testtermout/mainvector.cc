/* mainvector (C++98) */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdio.h>
#include	<vector>
#include	<string>

#include	<localmisc.h>


struct termout_gch {
	uchar		ch, gr ;
} ;

int main() {
	using namespace std ;
	struct termout_gch	gch ;
	vector<struct termout_gch>	a ;
	vector<string>		vs ;
	int		i ;
	int		sz ;
	int	&szr = sz ;
	int	*ip = new int[10] ;

	for (i = 0 ; i < 20 ; i += 1) {
	    gch.ch = i ;
	    gch.gr = 0 ;
	    a.push_back(gch) ;
	}

	sz = a.size() ;
	for (i = 0 ; i < sz ; i += 1) 
	printf("a[%u]=%d\n",i,a[i].ch) ;

	for (i = 0 ; i < sz ; i += 1) a[i].ch += 1 ;

	for (i = 0 ; i < sz ; i += 1) 
	printf("a[%u]=%d\n",i,a[i].ch) ;

	for (i = 0 ; i < 10 ; i += 1) ip[i] = i ;

	for (i = 0 ; i < 10 ; i += 1) 
	printf("a[%u]=%d\n",i,ip[i]) ;

	delete ip ;
	return 0 ;
}
/* end subroutine (main) */


