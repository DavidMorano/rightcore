/* main */


#define	CF_DEBUGS	0


#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#include	"config.h"
#include	"defs.h"



/* local defines */

#undef	BUFLEN
#define	BUFLEN		10000




int main()
{
	int	rs, len ;
	int	off ;
	int	i ;

	char	buf[BUFLEN + 1] ;


	off = 0 ;
	i = 0 ;
	while ((rs = u_read(FD_STDIN,buf,BUFLEN)) > 0) {

		len = rs ;

#if	CF_DEBUGS
			fprintf(stdout,"len=%d\n",len) ;
#endif

		for (i = 0 ; i < len ; i += 1) {

#if	CF_DEBUGS
			fprintf(stdout,"c=%c\n",buf[i]) ;
#endif

			if (strncmp("forward_DCT",(buf + i),11) == 0)
				break ;

		} /* end for */

		if (i < len)
			break ;

		off += len ;

	} /* end while */


	if (i < len)
	fprintf(stdout,"offset=%d (\\x%08x)\n",
		(off + i),
		(off + i)) ;


	fclose(stdout) ;

	return 0 ;
}



