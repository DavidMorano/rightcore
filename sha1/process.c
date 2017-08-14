/* process */

/* program to test out the the SHA-1 hash object */
/* last modified %G% version %I% */


#define	CF_DEBUGS	1
#define	CF_DEBUG	1


/* revision history:

	= 1999-02-01, David A­D­ Morano
        This subroutine (it's the whole program -- same as the FIFO test) was
        originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

*******/************************************************************************

	This subroutine tests the SHA-1 hash object.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<field.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"sha.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	LINELEN		2048		/* must be greater then 1024 */
#ifndef	BUFLEN
#define	BUFLEN		1024
#endif

#define	O_SRVFLAGS	(O_RDONLY | O_CREAT | O_NONBLOCK)

#define	COLUMNS		8


/* external subroutines */

extern int	cfdeci(char *,int,int *), cfhex() ;


/* external variables */


/* forward references */

static void		int_alarm() ;
static void		int_signal() ;


/* gloabal variables */

int		f_alarm ;
int		f_signal ;


int process(pip,ofp,pathname)
struct proginfo	*pip ;
bfile		*ofp ;
char		pathname[] ;
{
	SHA		e ;
	bfile		dfile ;
	uint		digest[5] ;
	int		len, rs ;
	int		i, j ;
	int		iw ;
	char		buf[BUFLEN + 1], *bp ;
	char		*cp ;

	if ((pathname == NULL) || (pathname[0] == '\0'))
		return SR_INVALID ;

	if ((rs = bopen(&dfile,pathname,"r",0666)) < 0)
		goto badin ;


		sha_init(&e) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("process: opened rs=%d\n",rs) ;
#endif

		while ((len = bread(&dfile,buf,BUFLEN)) > 0) {

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("process: bread() len=%d\n",len) ;
#endif

			sha_update(&e,buf,len) ;

		} /* end while (reading file) */

		(void) memset(buf,0,20) ;

		sha_digest(&e,buf) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	debugprintf("process: SHA>") ;
	for (i = 0 ; i < 20 ; i += 1) {
		debugprintf(" %02x",buf[i]) ;
	}
	debugprintf("\n") ;
	}
#endif

		for (i = 0 ; i < 5 ; i += 1) {
			netorder_readi(buf + (i * 4),digest + i) ;
		}

#if	CF_DEBUGS
	if (pip->debuglevel > 1) {

		if (memcmp(buf,digest,20) != 0)
		debugprintf("process: digest problem\n") ;

	}
#endif

		bprintf(ofp,"%08x %08x %08x %08x %08x\n",
			digest[0],
			digest[1],
			digest[2],
			digest[3],
			digest[4]) ;

		sha_free(&e) ;

	bclose(&dfile) ;

badret:
	return rs ;

badin:
	bprintf(pip->efp,"%s: could not open input, rs=%d\n",
	    pip->progname,rs) ;

	goto badret ;

}
/* end subroutine (process) */


