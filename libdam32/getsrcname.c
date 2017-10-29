/* getsrcname */

/* get the source address of a file-descriptor */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Find and return a name associated with a file-descriptor.

	Synopsis:

	int getsrcname(rbuf,rlen,s)
	char		rbuf[] ;
	int		rlen ;
	int		s ;

	Arguments:

	rbuf		buffer to receive result
	rlen		result length
	s		file descriptor

	Returns:

	>=0		OK
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<connection.h>
#include	<localmisc.h>


/* local defines */

#ifndef	DEBUGFNAME
#define	DEBUGFNAME	"getsrcname.deb"
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	getdomainname(char *,int) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;


/* forward references */


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int getsrcname(char *rbuf,int rlen,int s)
{
	const int	dlen = MAXHOSTNAMELEN ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
	char		dbuf[MAXHOSTNAMELEN+1] ;

#if	CF_DEBUGS
	debugprintf("getsrcname: ent s=%d\n",s) ;
#endif

	if (rbuf == NULL) return SR_FAULT ;

	if ((rs = getdomainname(dbuf,dlen)) >= 0) {
	    CONNECTION	conn ;
	    if ((rs = connection_start(&conn,dbuf)) >= 0) {

	        rs = connection_socksrcname(&conn,rbuf,s) ;
	        len = rs ;

	        rs1 = connection_finish(&conn) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (connection) */
	} /* end if (getdomainname) */

#if	CF_DEBUGS
	debugprintf("getsrcname: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getsrcname) */


/* local subroutines */


