/* prograw */

/* subroutine to process socket interface type addresses */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 1999-08-17, David A­D­ Morano
        This subroutine was taken from the LOGDIR/LOGNAME program (fist written
        for the SunOS 4.xx environment in 1989). This whole program is replacing
        serveral that did similar things in the past.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine tries to make a socket-style TLI address out of the
        given arguments.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<sockaddress.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	HOSTBUFLEN
#define	HOSTBUFLEN		(8 * MAXHOSTNAMELEN)
#endif

#ifndef	HEXBUFLEN
#define	HEXBUFLEN		(4 * MAXHOSTNAMELEN)
#endif


/* external subroutines */


/* external variables */


/* forward references */

int		progout_open(PROGINFO *) ;
int		progout_close(PROGINFO *) ;

static int	makehex(char *,const char *,int) ;


/* local structures */


/* local variables */


/* exported subroutines */


int prograw(PROGINFO *pip,cchar *familyname,cchar *netaddr1,cchar *netaddr2)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if ((netaddr1 != NULL) && (netaddr1[0] != '\0')) {
	    if ((rs = progout_open(pip)) >= 0) {
	        char	hexbuf[HEXBUFLEN + 1] ;
	        if ((rs = makehex(hexbuf,netaddr1,-1)) >= 0) {
		    rs = progout_printf(pip,"%t\n",hexbuf,rs) ;
		}
	        rs1 = progout_close(pip) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (progout) */
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid address specified >%s<\n" ;
	    bprintf(pip->efp,fmt,pn,netaddr1) ;
	}

	return rs ;
}
/* end subroutine (prograw) */


/* local subroutines */


static int makehex(char *hexbuf,cchar *addr,int alen)
{
	int		v ;
	int		i ;
	int		j = 0 ;

	if (alen < 0)
	    alen = strlen(addr) ;

	hexbuf[j++] = '\\' ;
	hexbuf[j++] = 'x' ;
	for (i = 0 ; i < alen ; i += 1) {
	    v = MKCHAR(addr[i]) ;
	    hexbuf[j++] = getdig((v >> 4) & 15) ;
	    hexbuf[j++] = getdig((v >> 0) & 15) ;
	}

	hexbuf[j] = '\0' ;
	return j ;
}
/* end subroutine (makehex) */


