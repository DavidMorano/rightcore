/* main */

/* strip (some) garbage */


#define	CF_DEBUGS	0




#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdio.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	LINELEN		200
#define	CHARISW(c)	(((c) == ' ') || ((c) == '\t'))
#define	CHARISWE(c)	(((c) == ' ') || ((c) == '\t') || ((c) == '\n'))
#define	fwritebytes(a,b,c)	fwrite((b),1,(c),(a))



/* external subroutines */

extern int	freadline(FILE *,char *,int) ;


/* forward references */

static int	hasgarb(char *,int,char **) ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	int	rs, len ;
	int	cl ;
	int	i ;
	int	fd_debug ;
	int	f_all = 0 ;

	char	linebuf[LINELEN + 1] ;
	char	*sp, *cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	f_all = (argc > 1) ;


	while ((rs = freadline(stdin,linebuf,LINELEN)) > 0) {

	    len = rs ;
	    linebuf[len] = '\0' ;

#if	CF_DEBUGS
	debugprintf("main: line=>%t<\n",
		linebuf,((linebuf[len - 1] == '\n') ? (len - 1) : len)) ;
#endif

	    if (i = hasgarb(linebuf,len,&cp)) {

	        cl = len - i ;

#if	CF_DEBUGS
	debugprintf("main: cp=>%t<\n",
		cp,((cp[cl - 1] == '\n') ? (cl - 1) : cl)) ;
#endif

	        fwritebytes(stdout,cp,cl) ;

	    } else
	        fwritebytes(stdout,linebuf,len) ;

	} /* end while (reading lines) */


	fclose(stdout) ;

#if	CF_DEBUGS
	fclose(stderr) ;
#endif

	return 0 ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int hasgarb(linebuf,len,rpp)
char	linebuf[] ;
int	len ;
char	**rpp ;
{
	int	sl, i ;

	char	*sp ;


	sp = linebuf ;
	sl = len ;
	while ((sl > 0) && CHARISWE(*sp)) {

	    sp += 1 ;
	    sl -= 1 ;
	}

#if	CF_DEBUGS
	debugprintf("main: sp=>%t<\n",
		sp,MAX(((sp[sl - 1] == '\n') ? (sl - 1) : sl),0)) ;
#endif

	i = sp - linebuf ;

#if	CF_DEBUGS
	debugprintf("main: i=%d\n",i) ;
#endif

	*rpp = sp ;
	return ((i > 0) && (sl > 0) && (sp[0] == '#')) ? i : 0 ;
}
/* end subroutine (hasgarb) */



