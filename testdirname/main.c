/* main */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	fgetline(FILE *,char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;

extern char	*strwcpy(char *,const char *,int) ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	int	rs = SR_OK ;
	int	sl, cl, dl ;
	int	len ;

	const char	*pp ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	dirbuf[LINEBUFLEN + 1] ;
	char	*sp ;
	char	*cp ;
	char	*dp ;


	fprintf(stdout,"arg0=%s\n",argv[0]) ;

	pp = getexecname() ;

	if (pp != NULL)
		fprintf(stdout,"execname=%s\n",pp) ;


	while ((len = fgetline(stdin,linebuf,LINEBUFLEN)) > 0) {

	    sl = sfshrink(linebuf,len,&sp) ;

	    if (sl > 0) {

	        sp[sl] = '\0' ;

	    cl = sfdirname(sp,sl,&cp) ;

		dp = dirbuf ;
		dl = cl ;
	        strwcpy(dirbuf,cp,cl) ;

	        fprintf(stdout,"%-40s => %s (%d)\n",sp,dp,dl) ;

	    }

	} /* end while */

	fclose(stdin) ;

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



