/* getloginterm */

/* get the name of the controlling terminal for the current session */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1999-01-10, David A­D­ Morano
        This subroutine was originally written. It was prompted by the failure
        of other terminal message programs from finding the proper controlling
        terminal.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will find and return the name of the controlling
	terminal for the given session ID.

	Arguments :
		sid	session ID to find controlling terminal for
		buf	user buffer to receive name of controlling terminal
		len	length of user supplied buffer

	Returns :
		<0	error
		>=	length of name of controlling terminal


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<tmpx.h>
#include	<localmisc.h>


/* local defines */

#define	DEVDIR		"/dev/"


/* external subroutines */

char	*strwcpy(char *,char *,int) ;


/* forward references */


/* exported subroutines */


int getloginterm(sid,buf,buflen)
int	sid ;
char	buf[] ;
int	buflen ;
{
	TMPX		utmp ;
	TMPX_ENT	ue ;
	int		rs ;
	int		i, len ;

	if (buf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	eprintf("getloginterm: entered, sid=%d\n",sid) ;
#endif

	if (sid <= 0)
	    sid = u_getsid((pid_t) 0) ;

#if	CF_DEBUGS
	eprintf("getloginterm: 2 sid=%d\n",sid) ;
#endif

	if ((rs = tmpx_open(&utmp,NULL,O_RDONLY)) >= 0) {

#if	CF_DEBUGS
	eprintf("getloginterm: tmpx_open() rs=%d\n",rs) ;
#endif

	    if ((rs = tmpx_fetchpid(&utmp,&ue,sid)) >= 0) {

	        if (buflen >= 0) {

	            if (buflen >= 6) {

	                strcpy(buf,DEVDIR) ;

	                for (i = 0 ; i < MIN(32,buflen) ; i += 1) {

			    if (ue.ut_line[i] == '\0')
				break ;

	                    buf[5 + i] = ue.ut_line[i] ;

			} /* end for */

#if	CF_DEBUGS
	eprintf("getloginterm: i=%d\n",i) ;
#endif

	                if ((i < 32) && (ue.ut_line[i] != '\0'))
	                    rs = SR_TOOBIG ;

	                else
	                    buf[5 + i] = '\0' ;

	                len = 5 + i ;

	            } else
	                rs = SR_TOOBIG ;

	        } else {

	            strcpy(buf,DEVDIR) ;

	            len = strwcpy(buf + 5,ue.ut_line,32) - buf ;

	        }

	    } /* end if (got it) */

	    tmpx_close(&utmp) ;

	} /* end if (UTMPX open) */

#if	CF_DEBUGS
	eprintf("getloginterm: exiting, rs=%d len=%d\n",rs,len) ;
	if (rs >= 0)
	eprintf("getloginterm: term=>%W<\n",buf,len) ;
#endif

	return ((rs >= 0) ? len : rs) ;
}
/* end subroutine (getloginterm) */



