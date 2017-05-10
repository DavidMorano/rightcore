/* lastlogin */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1995-01-22, David A­D­ Morano
        This subroutine module was adopted for use from some previous code that
        performed the similar sorts of functions.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is used to get the last login information for the
	specified UID.  The last login information is retrieved from the caller
	specified 'lastlog' file, which is by default '/var/adm/lastlog'.

	Synopsis:

	int lastlogin(fname,uid,tp,hostname,line)
	const char	fname[] ;
	uid_t		uid ;
	time_t		*tp ;
	char		hostname[] ;
	char		line[] ;

	Arguments:

	fname		the 'lastlog' file
	uid		the UID to lookup
	tp		pointer to the 'lastlog' timestamp
	hostname	buffer to receive hostname
	line		buffer to receive the line

	Returns:

	>=0		OK
	<0		bad and this is the error value


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<lastlogfile.h>
#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int lastlogin(fname,uid,tp,hostname,line)
const char	fname[] ;
uid_t		uid ;
time_t		*tp ;
char		hostname[] ;
char		line[] ;
{
	LASTLOGFILE	lf ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("lastlogin: ent\n") ;
#endif

	if (uid < 0) uid = getuid() ;

	if ((rs = lastlogfile_open(&lf,fname,O_RDONLY)) >= 0) {
	    {
	        rs = lastlogfile_readinfo(&lf,uid,tp,line,hostname) ;
	        len = rs ;
	    }
	    rs1 = lastlogfile_close(&lf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (lastlogfile) */

#if	CF_DEBUGS
	debugprintf("lastlogin: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (lastlogin) */


