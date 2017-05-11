/* snshellunder */

/* make the shell-under information string */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates the shell-under string for a child process.
	The string looks like:

		*<ppid>*<execfname>

	where:

	ppid		parent process ID
	execfname	program exec-file-name

	Synopsis:

	int snshellunder(dbuf,dlen,pid,execfname)
	char		dbuf[] ;
	int		dlen ;
	pid_t		pid ;
	const char	*execfname ;

	Arguments:

	dbuf		destination buffer
	dlen		length of destrination buffer
	pid		parent process ID
	execdname	exec-file-name

	Returns:

	>=0		length of created string
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	ctdeci(char *,int,int) ;
extern int	ctdecui(char *,int,uint) ;


/* exported subroutines */


int snshellunder(char *dbuf,int dlen,pid_t pid,cchar *execfname)
{
	int		rs = SR_OK ;
	int		i = 0 ;
	if (dbuf == NULL) return SR_FAULT ;
	if (execfname == NULL) return SR_FAULT ;
	if (pid >= 0) {
	    const int	mch = '*' ;
	    if (rs >= 0) {
	        rs = storebuf_char(dbuf,dlen,i,mch) ;
	        i += rs ;
	    }
	    if (rs >= 0) {
	        uint	v = (int) pid ;
	        rs = storebuf_decui(dbuf,dlen,i,v) ;
	        i += rs ;
	    }
	    if (rs >= 0) {
	        rs = storebuf_char(dbuf,dlen,i,mch) ;
	        i += rs ;
	    }
	} /* end if (PID included) */
	if (rs >= 0) {
	    rs = storebuf_strw(dbuf,dlen,i,execfname,-1) ;
	    i += rs ;
	}
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (snshellunder) */


