/* main */



#include	<sys/types.h>
#include	<stdio.h>

#include	"localmisc.h"



/* local defines */

#define	LINELEN		1100





int main()
{
	int	rs, len ;

	char	linebuf[LINELEN + 1] ;


	while ((len = fgetline(stdin,linebuf,LINELEN)) > 0) {

		if ((len == 1) && (linebuf[0] == '\n'))
			break ;

		if ((len == 2) && (linebuf[0] == '\r') && (linebuf[1] == '\n'))
			break ;

		fwrite(linebuf,1,len,stdout) ;

	} /* end while */

	fclose(stdin) ;

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



