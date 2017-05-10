/* logout */

/* log out of the system (out of the UTMP database) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1999-03-01, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine logs the caller out of the system. What this really does
        is to log the caller out of the UTMP database.

	Synopsis:

	int logout(pid_t sid)

	Arguments:

	sid		session ID to log-out

	Returns:

	0		OK
	SR_PERM		no permission to perform function
	SR_NOENT	session was not logged in


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<tmpx.h>
#include	<localmisc.h>


/* local defines */

#ifndef	WTMPFNAME
#define	WTMPFNAME	"/var/adm/wtmpx"
#endif

#ifndef	UTMPFNAME
#define	UTMPFNAME	"/var/adm/utmpx"
#endif


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	mkutmpid(char *,int,const char *,int) ;
extern int	idcpy(char *,const char *,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int logout(pid_t sid)
{
	struct utmpx	uc ;
	struct utmpx	*up ;
	int		rs = SR_OK ;
	int		si = 0 ;

	if (sid <= 0) sid = getsid(0) ;	/* not supposed to fail! */

#if	CF_DEBUGS
	    debugprintf("logout: sid=%d\n",sid) ;
#endif

	setutxent() ;

	while ((up = getutxent()) != NULL) {

	    if ((up->ut_pid == sid) && (up->ut_type == TMPX_TUSERPROC))
	        break ;

	    si += 1 ;

	} /* end while (positioning within the UTMPX file) */

	if (up != NULL) {

	    uc = *up ;		/* copy the record found */

	    uc.ut_exit.e_termination = 0 ;
	    uc.ut_exit.e_exit = 0 ;
	    uc.ut_type = TMPX_TDEADPROC ;

	    up = pututxline(&uc) ;
	    if (up == NULL) rs = SR_PERM ;

	} else
	    rs = SR_NOENT ;

	endutxent() ;

	return (rs >= 0) ? si : rs ;
}
/* end subroutine (logout) */


