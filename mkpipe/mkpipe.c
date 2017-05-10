/* make a FIFO special filo */

/* 
	David A.D. Morano
	June 1984
*/


/* ************************************************************************

	This program will a FIFO special file.


****************************************************************************/



#include	<fcntl.h>

#include	"localmisc.h"



int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	int	i, rs ;

	char	*s ;


	if (argc < 2) {

		s = "usage: mkpipe filename\n" ;
		write(2,s,strlen(s)) ;

		return BAD ;
	}

	for (i = 1 ; i < argc ; i += 1) {

		rs = mknod(argv[i],0010666) ;

		if (rs < 0) return BAD ;

	}

	return OK ;
}


