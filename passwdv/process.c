/* process */

/* process a name */


#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_GETPASSWORD	1		/* use 'getpassword(3dam)' */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This module processes a single password verification request.

	Synopsis:

	int process(pip,pp,pfp,ofp,name)
	PROGINFO	*pip ;
	PARAMOPT	*pp ;
	PWFILE		*pfp ;
	bfile		*ofp ;
	const char	name[] ;

	Arguments:

	gp		global data pointer
	username	username to check on
	paramoptp	** not used **

	Returns:

	0		the password verified OK
	<0		the password did not verify


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<pwfile.h>
#include	<getax.h>
#include	<getxusername.h>
#include	<paramopt.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	bufprintf(char *,int,const char *,...) ;

extern int	getpassword(const char *,char *,int) ;
extern int	checkpass(PROGINFO *,const char *,const char *,int) ;
extern int	passwdok(const char *,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	userexists(PROGINFO *,PWFILE *,const char *) ;


/* local variables */


/* exported subroutines */


int process(pip,pp,pfp,ofp,name)
PROGINFO	*pip ;
PARAMOPT	*pp ;
PWFILE		*pfp ;
bfile		*ofp ;
const char	name[] ;
{
	int	rs = SR_OK ;
	int	f_valid = FALSE ;

	char	ubuf[USERNAMELEN + 1] ;
	char	prompt[PROMPTLEN + 1] ;
	char	password[PASSWDLEN + 1] ;
	char	*p ;


	if (name == NULL)
	    return SR_FAULT ;

	if (name[0] == '\0')
	    return SR_INVALID ;

	if (name[0] == '-') {
	    if (pip->username == NULL) {
		rs = getusername(ubuf,USERNAMELEN,-1) ;
		if (rs >= 0)
		    rs = proginfo_setentry(pip,&pip->username,ubuf,rs) ;
	    }
	    name = pip->username ;
	}

	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: username=%s\n",name) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: username=%s\n",
		pip->progname,name) ;

	if (! userexists(pip,pfp,name)) {
	    bprintf(ofp,"username=%s does not exist\n",name) ;
	    goto ret0 ;
	}

/* we have a user entry */

	rs = bufprintf(prompt,PROMPTLEN,"password user=%s: ",
	    name) ;

#if	CF_GETPASSWORD

	if (rs >= 0)
	    rs = getpassword(prompt,password,PASSWDLEN) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: getpassword() rs=%d password=>%s<\n",
	        rs,password) ;
#endif

#else /* CF_GETPASSWORD */

	if (rs >= 0) {

	    p = getpass(prompt) ;

	    rs = sncpy1(password,PASSWDLEN,p) ;

	}

#endif /* CF_GETPASSWORD */

	if ((rs >= 0) && pip->f.sevenbit) {

	    for (p = password ; *p != '\0' ; p += 1)
	        *p = *p & 0x7F ;

	} /* end if (seven bit) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: password >%s<\n",password) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	if (pfp != NULL) {

	    PWFILE_ENT	pw ;

	    PWFILE_CUR	cur ;

	    char	pwbuf[PWFILE_ENTLEN + 1] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("process: using file\n") ;
#endif

	    pwfile_curbegin(pfp,&cur) ;

	    while (pwfile_fetchuser(pfp,name,&cur,
	        &pw,pwbuf,PWFILE_ENTLEN) >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("process: got one >%s<\n",pw.password) ;
#endif

	        if (passwdok(password,pw.password)) {
	            f_valid = TRUE ;
	            break ;
	        }

	    } /* end while */

	    pwfile_curend(pfp,&cur) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("process: done trying\n") ;
#endif

	} else {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("process: using system 'checkpass()'\n") ;
#endif

	    rs = checkpass(pip,name,password,TRUE) ;
	    f_valid = (rs > 0) ;

	} /* end if */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: ret rs=%d f_valid=%u\n",rs,f_valid) ;
#endif

	return (rs >= 0) ? f_valid : rs ;
}
/* end subroutine (process) */


/* local subroutines */


/* is the username present in the password database ? */
static int userexists(pip,pfp,name)
PROGINFO	*pip ;
PWFILE		*pfp ;
const char	name[] ;
{
	int	rs = SR_OK ;
	int	f = FALSE ;

	char	pwbuf[PWFILE_ENTLEN + 1] ;


	if (pfp != NULL) {
	    PWFILE_ENT	pfe ;

	    rs = pwfile_fetchuser(pfp,name,NULL,&pfe,pwbuf,PWFILE_ENTLEN) ;

	} else {
	    struct passwd	spw ;

	    rs = getpw_name(&spw,pwbuf,PWFILE_ENTLEN,name) ;

	}

	f = (rs >= 0) ;
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (userexists) */


