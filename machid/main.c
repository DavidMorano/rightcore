/* main */

/* get the various IDs from the system */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1988-01-10, David A­D­ Morano

	This subroutine was written (originally) as a test of
	the Sun Solaris UNIX 'kstat' facility.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	Get the IDs from the system.


***************************************************************************/


#include	<envstadnards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/systeminfo.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	TO		11


/* external subroutines */

extern int	cfdecl(char *,int,long) ;


/* external variables */


/* forward references */


/* gloabal variables */


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	FILE	*fp = stdout ;
	long	v ;
	uint	hostid ;

	int	rs ;
	int	rs1 ;
	int	len ;
	int	i, j ;
	int	bl, sl ;
	int	s ;
	int	fd = 0 ;

	char	buf[BUFLEN + 1], *bp ;
	char	*pr = NULL ;
	char	*progname ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	progname = argv[0] ;

/* print out some common accessible machine parameters */

	sl = u_sysinfo(SI_HW_PROVIDER,buf,BUFLEN) ;

	buf[sl] = '\0' ;
	fprintf(fp,"provider> %s\n",buf) ;


	sl = u_sysinfo(SI_HW_SERIAL,buf,BUFLEN) ;

	buf[sl] = '\0' ;
	fprintf(fp,"serial> %s\n",buf) ;


	rs1 = uc_sysconf(_SC_TZNAME_MAX,&v) ;

#if	CF_DEBUGS
	debugprintf("main: TZNAME_MAX uc_sysconf() rs=%d\n",rs1) ;
#endif

	bl = 0 ;
	if (rs1 >= 0)
	bl = ctdecl(buf,BUFLEN,v) ;

	buf[bl] = '\0' ;
	fprintf(fp,"tznamelen> %s\n",buf) ;

	hostid = (uint) gethostid() ;

	fprintf(fp,"hostid> %u (%08x)\n",
	    (uint) hostid,
	    (uint) hostid) ;

	fclose(fp) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return EX_OK ;
}
/* end subroutine (main) */


