/* testmean */



#include	<sys/types.h>
#include	<stdlib.h>
#include	<math.h>

#include	<vsystem.h>
#include	<bfile.h>

#include	"localmisc.h"



/* local defines */

#define	LINELEN	100
#define	N	20



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecull(const char *,int,ULONG *) ;

extern char	*strshrink(char *) ;






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;

	double	mean, var ;

	ULONG	a[N + 1] ;

	uint	t ;

	int	rs, i, len ;
	int	n ;
	int	err_fd ;

	char	linebuf[LINELEN + 1] ;
	char	*cp ;


	if (((cp = getenv("DEBUGFD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;


	bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	bcontrol(ofp,BC_LINEBUF,0) ;

	bopen(ifp,BFILE_STDIN,"dr",0666) ;

	while (TRUE) {

	    for (n = 0 ; n < N ; n += 1) {

	        bprintf(ofp,"enter number %2d> ",n) ;

	        len = breadline(ifp,linebuf,LINELEN) ;

	        if (len <= 0)
	            break ;

	        linebuf[len] = '\0' ;
	        cp = strshrink(linebuf) ;

	        if (cp[0] == '\0')
	            continue ;

	        rs = cfdecull(cp,-1,&a[n]) ;

	        if (rs < 0) {

	            n -= 1 ;
	            bprintf(ofp,"bad number specified\n") ;

	        }

	    } /* end for */

	    if (n <= 0)
	        break ;

	    bprintf(ofp,"\n") ;

	    t = 0 ;
	    for (i = 0 ; i < n ; i += 1)
	        t += (uint) a[i] ;

	    bprintf(ofp,"numbers=%d total=%d\n",n,t) ;

	    rs = fmeanvaral(a,n,&mean,&var) ;

	    if (rs >= 0) {

	        bprintf(ofp,"mean=%.2f variance=%.2f stddev=%.2f\n",
	            mean,var,sqrt(var)) ;

/* calculate the 'chi' value */

	        if (mean > 0.0) {

	            double	fn, x2 ;


	            fn = (double) n ;
	            x2 = var * fn / mean ;
	            bprintf(ofp,"chi=%.2f \n",
	                chi(x2,(t - 1))) ;

	        }

	    }

	} /* end while */

	bprintf(ofp,"\n") ;


	bclose(ifp) ;

	bclose(ofp) ;

	return 0 ;
}
/* end subroutine (main) */



