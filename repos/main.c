/* main (repos) */
/* lang=C89 */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<string.h>
#include	<stdio.h>

/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

/* external subroutines */

extern int	strwcmp(const char *,const char *,int) ;

extern char	*strnchr(const char *,int,int) ;


/* forward references */

static int hmat(const char *,int) ;
static int rmat(const char *,int,const char *) ;

/* exported subroutines */

int main(int argc,const char **argv,const char **envv)
{
	FILE		*ifp = stdin ;
	FILE		*ofp = stdout ;
	const int	llen = LINEBUFLEN ;
	int		f = 0 ;
	char		lbuf[LINEBUFLEN+1] ;
	const char	*name = "REPOS" ;

	if (argc > 1) {
	    name = argv[1] ;
	}

	while (fgets(lbuf,llen,ifp) > 0) {
	   int	ll = strlen(lbuf) ;
	   if (lbuf[ll-1] == '\n') ll -= 1 ;
	   if (hmat(lbuf,ll)) {
		f = rmat(lbuf,ll,name) ;
	   }
 	   if (f) {
		if (strstr(lbuf,"enabled=1") != NULL) {
		    fprintf(ofp,"enabled=0\n") ;
		} else
		    fprintf(ofp,"%s",lbuf) ;
	   } else
		fprintf(ofp,"%s",lbuf) ;
	} /* end while */

	return 0 ;
}
/* end subroutine (main) */

/* local subrouines */

static int hmat(const char *sp,int sl)
{
	int		f = 0 ;
	const char	*cp ;
	const char	*tp ;
	if ((tp = strnchr(sp,sl,'{')) != NULL) {
	    cp = (tp+1) ;
	    sl -= ((tp+1)-sp) ;
	    sp = (tp+1) ;
	    if ((tp = strnchr(sp,sl,'}')) != NULL) {
		f = 1 ;
	    }
	}
	return f ;
}

static int rmat(const char *sp,int sl,const char *name)
{
	int		cl ;
	int		f = 0 ;
	const char	*cp ;
	const char	*tp ;
	if ((tp = strnchr(sp,sl,'{')) != NULL) {
	    cp = (tp+1) ;
	    sl -= ((tp+1)-sp) ;
	    sp = (tp+1) ;
	    if ((tp = strnchr(sp,sl,'}')) != NULL) {
		cl = (tp-sp) ;
		f = (strwcmp(name,cp,cl) == 0) ;
	    }
	}
	return f ;
}
/* end subroutine (rmat) */


