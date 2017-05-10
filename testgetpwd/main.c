/* main */


#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdio.h>

#include	<exitcodes.h>



#define	BUFLEN	MAXPATHLEN



int main()
{
	int	rs ;

	char	buf[BUFLEN + 1] ;


	rs = getpwd(buf,BUFLEN) ;

	if (rs >= 0) {

		buf[rs] = '\0' ;
		fprintf(stdout,"%s\n",buf) ;

		fclose(stdout) ;

	}

	return (rs >= 0) ? EX_OK : EX_DATAERR ;
}
/* end subroutine (main) */



