/* process */

/* process a name */


#define	CF_DEBUG	1
#define	F_GETPASSWORD	1


/* revision history:

	= 96/03/01, David A­D­ Morano

	The program was written from scratch to do what
	the previous program by the same name did.


*/


/******************************************************************************

	This module processes a single password verification request.

	Arguments:

	gp		global data pointer
	username	username to check on
	paramoptp	** not used **


	Returns:

	0		the password verified OK
	<0		the password did not verify



******************************************************************************/


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

#include	<bfile.h>
#include	<baops.h>
#include	<pwfile.h>
#include	<getax.h>

#include	"localmisc.h"
#include	"paramopt.h"
#include	"config.h"
#include	"defs.h"



/* local defines */



/* external subroutines */

extern int	checkpass() ;
extern int	passwdok(const char *,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	userexists(struct proginfo *,PWFILE *,char *) ;


/* global variables */


/* local variables */






int process(gp,pfp,name,pp)
struct proginfo	*gp ;
PWFILE		*pfp ;
char		name[] ;
PARAMOPT	*pp ;
{
	int	rs = OK ;

	char	*p, password[PASSLEN + 1] ;
	char	prompt[PROMPTLEN + 1] ;


	if ((name == NULL) || (name[0] == '\0'))
	    return BAD ;


	if (! userexists(gp,pfp,name)) {

	    bprintf(gp->ofp,"user \"%s\" does not exist\n",name) ;

	    return BAD ;
	}


/* we have a user entry */

	bufprintf(prompt,PROMPTLEN,"password for '%s': ",
	    name) ;

#if	F_GETPASSWORD
	rs = getpassword(prompt,password,PASSLEN) ;
#else
	p = getpass(prompt) ;

	strcpy(password,p) ;
#endif

	if (gp->f.sevenbit) {

	    for (p = password ; *p != '\0' ; p += 1)
	        *p = *p & 0x7F ;

	} /* end if (seven bit) */

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("process: password >%s<\n",password) ;
#endif

	if (rs < 0)
	    return rs ;

	if (pfp != NULL) {

	    PWFILE_ENT	pe ;

	    PWFILE_CUR	cur ;

	char	pebuf[PWFILE_ENTLEN + 1] ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("process: using file\n") ;
#endif

	    pwfile_curbegin(pfp,&cur) ;

	    rs = SR_ACCESS ;
	    while (pwfile_fetchuser(pfp,name,&cur,
	        &pe,pebuf,PWFILE_ENTLEN) >= 0) {

#if	CF_DEBUG
	        if (gp->debuglevel > 1)
	            debugprintf("process: got one >%s<\n",pe.password) ;
#endif

	        if (passwdok(password,pe.password)) {

			rs = SR_OK ;
	            break ;
		}

	    } /* end while */

	    pwfile_curend(pfp,&cur) ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("process: done trying\n") ;
#endif

	} else {

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("process: using system\n") ;
#endif

	    rs = checkpass(gp,name,password,TRUE) ;

	}

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("process: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */



/* LOCAL SUBROUTINES */



/* is the username present in the password database ? */
static int userexists(gp,pfp,name)
struct proginfo	*gp ;
PWFILE		*pfp ;
char		name[] ;
{
	int	rs ;

	char	pebuf[PWFILE_ENTLEN + 1] ;


	if (pfp != NULL) {

	    PWFILE_ENT	pfe ;


#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("process/userexists: using file\n") ;
#endif

	    rs = pwfile_fetchuser(pfp,name,NULL,&pfe,pebuf,PWFILE_ENTLEN) ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("process/userexists: done trying\n") ;
#endif

	} else {

	    struct passwd	spe ;


#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("process/userexists: using system\n") ;
#endif

	    rs = getpw_name(&spe,pebuf,PWFILE_ENTLEN,name) ;

	}

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("process/userexists: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ;
}
/* end subroutine (userexists) */



