/* mailspooldir */

/* check for a valid mail spool directory */
/* last modified %G% version %I% */


#define	CF_DEBUG	0


/* revision history:

	= 1997-01-10, David A­D­ Morano
	This code module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

        This subroutine rates the value of a supplied mailspool directory name.
        A higher rated directory name returns a higher value on return.

        NOTE: The big thing to note in this subroutine, and any other code that
        "pretends" to check for the existence of the mail spool directory, is
        that the directory may also be an AUTOMOUNT mount point. This means that
        it might not be accessible when we go to check it alone. Instead, we
        have to check something inside the directory to make sure that the
        AUTOMOUNTer first mounts the directory.

	Synopsis:

	int mailspooldir(pip,spooldname,fl)
	char	spooldname[] ;
	int	fl ;

	Arguments:

	pip		- program information pointer
	spooldname	- a candidate spool file
	fl		- optional length of directory name

	Returns:

	0		- spool file is minimally accessible
	1		- spool file is more accessible
	2		- et cetera
	<0		- not a good spool file


************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<time.h>
#include	<signal.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external routines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	perm(const char *,uid_t, gid_t,gid_t *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mailspooldir(pip,spooldname,fl)
struct proginfo	*pip ;
char		spooldname[] ;
int		fl ;
{
	struct ustat	sb ;

	int	rs, srs ;
	int	cl, glen ;

	char	tmpdname[MAXPATHLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	*cp ;


	srs = -1 ;
	if ((spooldname == NULL) || (spooldname[0] == '\0'))
	    return srs ;

	strwcpy(tmpdname,spooldname,MIN(fl,(MAXPATHLEN - 1))) ;

/* try something inside the directory (because of automounting) */

	mkpath2(tmpfname,tmpdname,":saved") ;

	if ((u_stat(tmpfname,&sb) < 0) || (! S_ISDIR(sb.st_mode))) {

		mkpath2(tmpfname,tmpdname,"root") ;

	if (u_stat(tmpfname,&sb) < 0)
	    return srs ;

	}

/* OK, let's try to continue */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("mailspooldir: maildname=%s\n",tmpdname) ;
#endif

	rs = -1 ;

/* handle the common case first (it doesn't really matter) */

	if (pip->f.setgid)
	    rs = perm(tmpdname,-1,pip->egid,NULL,W_OK) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("mailspooldir: 1 rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    rs = perm(tmpdname,-1,-1,NULL,W_OK) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("mailspooldir: 2 rs=%d\n",rs) ;
#endif

	if ((rs < 0) && pip->f.setuid)
	    rs = perm(tmpdname,pip->euid,-1,NULL,W_OK) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("mailspooldir: 3 rs=%d\n",rs) ;
#endif

	if (rs < 0) {

/* try harder */

#ifdef	COMMENT
	    if ((pip->euid == 0) &&
	        (sb.st_gid != pip->egid) && (sb.st_gid != pip->gid)) {

	    }
#endif /* COMMENT */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("mailspooldir: bad perm rs=%d\n",rs) ;
#endif

	    if ((pip->uid != 0) && (pip->euid != 0))
	        return srs ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("mailspooldir: 4 rs=%d\n",rs) ;
#endif

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("mailspooldir: spool directory is writable !\n") ;
#endif

	srs = 0 ;

/* is the last component of the MAILDIR path named "mail" ? */

	cl = sfbasename(tmpdname,-1,&cp) ;

	glen = strlen(MAILGNAME) ;

	if ((cl == 4) && (strncmp(cp,MAILGNAME,glen) == 0) )
		srs +=1 ;

/* is there a spool file there that the REAL user can read ? */

	if (u_access(tmpdname,W_OK) >= 0) 
		srs += 1 ;

	return srs ;
}
/* end subroutine (mailspooldir) */



