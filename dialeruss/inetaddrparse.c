/* inetaddrparse */

/* this little thing parses an INET address into parts */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_SAFE		1		/* extra safe? */


/* revision history:

	= 1998-03-24, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine (object?) parses an INET host-address-port
        specification.

	Synopsis:

	int inetaddrparse_load(ap,s,slen)
	INETADDRPARSE	*ap ;
	const char	s[] ;
	int		slen ;

	Arguments:

	- ap		pointer to object (to hold results)
	- s		string to parse
	- slen		length of string to parse

	Rerurns:

	<0		bad
	>=0		OK; number of items in specification

	Notes:

	The string is parsed as follows:

		[[<af>:]<host>:]<port>


*******************************************************************************/


#define	INETADDRPARSE_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<stropts.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"inetaddrparse.h"


/* local defines */


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	getaf(const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int inetaddrparse_load(INETADDRPARSE *ap,cchar *sp,int sl)
{
	int		cl ;
	int		n = 0 ;
	const char	*tp ;
	const char	*cp ;

	if (ap == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	memset(ap,0,sizeof(INETADDRPARSE)) ;

	n += 1 ;
	if ((tp = strnchr(sp,sl,':')) != NULL) {
	    n += 1 ;
	    ap->af.sp = sp ;
	    ap->af.sl = (tp - sp) ;
	    ap->host.sp = sp ;
	    ap->host.sl = (tp - sp) ;
	    cp = (tp + 1) ;
	    cl = ((sp + sl) - cp) ;
	    if ((tp = strnchr(cp,cl,':')) != NULL) {
	        n += 1 ;
	        ap->host.sp = cp ;
	        ap->host.sl = (tp - cp) ;
	        ap->port.sp = (tp + 1) ;
	        ap->port.sl = ((sp + sl) - (tp + 1)) ;
	    } else {
	        ap->af.sp = NULL ;
	        ap->af.sl = 0 ;
	        ap->port.sp = cp ;
	        ap->port.sl = cl ;
	    } /* end if */
	} else {
	    ap->port.sp = sp ;
	    ap->port.sl = sl ;
	} /* end if */

	return n ;
}
/* end subroutine (inetaddrparse_load) */


