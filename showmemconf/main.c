/* main */

/* test out Sun Solaris UNIX 'kstat's */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1988-01-10, David A­D­ Morano

	This subroutine was written (originally) as a test of the Sun
	Solaris UNIX 'kstat' facility.


*/

/* Copyright © 1988 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	Show the total and available memory on this system.


***************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<field.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	BUFLEN		1024

#define	TO		11


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;

extern char	*strbasename(char *) ;


/* external variables */


/* forward references */

void		int_alarm() ;
void		int_signal() ;

static int	bprintla() ;


/* gloabal variables */

int		f_alarm ;
int		f_signal ;






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	long	lw ;

	int	rs ;
	int	i, j ;
	int	pagesize, ppm ;
	int	ex = EX_INFO ;

	char	*progname ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	progname = strbasename(argv[0]) ;

	ex = EX_OK ;

/* some extra host information */

	pagesize = getpagesize() ;

	ppm = (1024 * 1024) / pagesize ;

	uc_sysconf(_SC_PHYS_PAGES,&lw) ;

	fprintf(stdout,"physical pages      total=%12ld (%8ld MiBytes)\n",
		lw,(lw / ppm)) ;

	uc_sysconf(_SC_AVPHYS_PAGES,&lw) ;

	fprintf(stdout,"physical pages  available=%12ld (%8ld MiBytes)\n",
		lw,(lw / ppm)) ;


	fclose(stdout) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


