/* main */

/* strip (some) garbage */




#include	<string.h>
#include	<ctype.h>
#include	<stdio.h>



/* local defines */

#define	LINELEN		100
#define	MATHX(b)	(((b)[0] == '0') && ((b)[1] == 'x'))
#define	CHARISW(c)	(((c) == ' ') || ((c) == '\t'))
#define	CHARISWE(c)	(((c) == ' ') || ((c) == '\t') || ((c) == '\n'))
#define	fwritebytes(a,b,c)	fwrite((b),1,(c),(a))



/* external subroutines */

extern int	freadline(FILE *,char *,int) ;


/* forward references */

static int	matinstr(char *,char **) ;






int main(argc,argv,envv)
int	argc ;
char	*argv[], *envv[] ;
{
	int	rs, len ;
	int	f_all = 0 ;

	char	linebuf[LINELEN + 1] ;
	char	*sp, *cp ;


	f_all = (argc > 1) ;


	while ((rs = freadline(stdin,linebuf,LINELEN)) > 0) {

	    len = rs ;
	    if (matinstr(linebuf,&sp)) {

	            fwritebytes(stdout,linebuf,(sp - linebuf)) ;

	                fputc('\n',stdout) ;

	    } else
	        fwritebytes(stdout,linebuf,len) ;

	} /* end while (reading lines) */


	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int matinstr(linebuf,rpp)
char	linebuf[] ;
char	**rpp ;
{
	char	*sp ;


	if (! MATHX(linebuf))
	    return 0 ;

	sp = linebuf + 2 ;
	while (*sp && (! CHARISWE(*sp)))
	    sp += 1 ;

	while (CHARISWE(*sp))
	    sp += 1 ;

	if (*sp == '\0')
	    return 0 ;

	if (! isalpha(*sp))
	    return 0 ;

	while (*sp && (! CHARISWE(*sp)))
	    sp += 1 ;

	*rpp = sp ;
	return 1 ;
}
/* end subroutine (matinstr) */



