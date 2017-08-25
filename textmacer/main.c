/* main (textmacer) */


#include	<envstandards.h>

#include	<stdio.h>



/* ARGSUSED */
int main(int argc,const char **argv,const char **envv)
{
	FILE		*ifp = stdin ;
	FILE		*ofp = stdout ;
	int		c ;

	while ((c = fgetc(ifp)) >= 0) {
		if (c == '\r') c = '\n' ;
		fputc(c,ofp) ;
	} /* end while */

	return 0 ;
}
/* end subroutine (main) */


