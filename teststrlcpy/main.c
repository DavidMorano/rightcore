/* main */

/* test the 'strlcpy()' subroutine for performance */


#define	CF_DEBUGS	1


/******************************************************************************

	This little program tests the 'strlcpy()' subroutine
	for performance.


******************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	NLOOPS		1000
#define	NSTRS		10000
#define	NSTRLEN		10


/* external subroutines */

extern int	strlcpy2(char *,const char *,int) ;

extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	process(struct proginfo *,int (*)()) ;


/* local variables */

static unsigned char	tterms[] = {
	0x00, 0x1B, 0x00, 0x00,
	0x01, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static unsigned char	dterms[] = {
	0x00, 0x1B, 0x00, 0x00,
	0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const char	*types[] = {
	"STD",
	"MSG",
	"STRDIG",
	"TOUCHT",
	"LOGZ",
	NULL
} ;

enum types {
	type_std,
	type_msg,
	type_strdig,
	type_toucht,
	type_logz,
	type_overlast
} ;





int main()
{
	struct proginfo	pi, *pip = &pi ;

	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;

	time_t	daytime ;

	int	rs, i, n ;
	int	len, sl, cl ;
	int	itype, otype ;
	int	fd_debug ;

	char	timebuf[TIMEBUFLEN + 1] ;
	char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	bopen(ifp,BFILE_STDIN,"dr",0666) ;

	bopen(ofp,BFILE_STDOUT,"dwct",0666) ;



	process(pip,strlcpy2) ;

	process(pip,(int (*)()) strlcpy) ;



	bclose(ofp) ;

	fclose(stdout) ;

	return EX_OK ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int process(pip,func)
struct proginfo	*pip ;
int		(*func)() ;
{
	struct rusage	ru1, ru2 ;

	struct timeval	result ;

	int	rs ;
	int	i, j ;
	int	size ;

	char	*src ;
	char	dst[NSTRLEN + 1] ;
	char	*sp ;


	size = NSTRS * (NSTRLEN + 1) ;
	rs = uc_malloc(size,&src) ;

	if (rs < 0)
		return rs ;

/* fill in some strings */

	for (i = 0 ; i < NSTRS ; i += 1) {

		sp = src + (i * (NSTRLEN + 1)) ;
		for (j = 0 ; j < NSTRLEN ; j += 1)
			sp[j]  = 'a' ;

		sp[j] = '\0' ;

	} /* end for */

/* do the test */

	uc_getrusage(RUSAGE_SELF,&ru1) ;

	for (j = 0 ; j < NLOOPS ; j += 1) {

	for (i = 0 ; i < NSTRS ; i += 1) {

		sp = src + (i * (NSTRLEN + 1)) ;
		(*func)(dst,sp) ;

	}

	}

	uc_getrusage(RUSAGE_SELF,&ru2) ;

	timeval_sub(&result,&ru2,&ru1) ;

	fprintf(stdout,"%6u:%6u\n",
		result.tv_sec,result.tv_usec) ;

	return 0 ;
}
/* end subroutine (process) */



