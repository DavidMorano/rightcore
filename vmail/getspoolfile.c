/* getspoolfile */

/* get the system mail spool directory */
/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This code module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int getspoolfile(pcsroot,prog_getmail,mailuser,spoolfile)
	char	pcsroot[] ;
	char	prog_getmail[] ;
	char	mailuser[] ;
	char	spoolfile[] ;

	Arguments:

	pcsroot		PCS system root directory
	prog_getmail	PCS 'getmail' program (or look-alike)
	mailuser	the mailuser to use
	spoolfile	user specified output buffer for result

	Returns:

	- spoolfile	system mail spool file for this user
	>=0		operation completed successfully
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tzfile.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<userinfo.h>
#include	<pcsconf.h>
#include	<localmisc.h>


/* local defines */

#ifndef	PROG_GETMAIL
#define	PROG_GETMAIL	"getmail"
#endif

#ifndef	PCS
#define	PCS		"/usr/add-on/pcs"
#endif

#ifndef	BUFLEN
#define	BUFLEN		MAXPATHLEN
#endif

#ifndef	CMDBUFLEN
#define	CMDBUFLEN	((2 * MAXPATHLEN) + 10)
#endif


/* external subroutines */

extern int	sfshrink(const char *,int,char **) ;
extern int	pcsgetprog(const char *,char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* exported subroutines */


int getspoolfile(pcsroot,prog_getmail,mailuser,spoolfile)
char	pcsroot[] ;
char	prog_getmail[] ;
char	mailuser[] ;
char	spoolfile[] ;
{
	bfile	tmpfile, *tfp = &tmpfile ;
	bfile	*fpa[3] ;

	int	rs ;
	int	len, cl ;

	char	cmdbuf[CMDBUFLEN + 1] ;
	char	progbuf[MAXPATHLEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	*cp ;


#if	CF_DEBUG
	debugprintf("getspoolfile: getting mail user\n") ;
#endif

	if (spoolfile != NULL)
		spoolfile[0] = '\0' ;

	if (pcsroot == NULL) {

		if ((pcsroot = getenv("PCS")) == NULL)
			pcsroot = PCS ;

	}

	if (prog_getmail == NULL)
		prog_getmail = PROG_GETMAIL ;

	rs = pcsgetprog(pcsroot,prog_getmail,progbuf) ;

	if (rs >= 0) {

		char	*progfname ;


		progfname = prog_getmail ;
		if (rs > 0)
			progfname = progbuf ;

	    if (mailuser != NULL)
	        bufprintf(cmdbuf,CMDBUFLEN,"%s -p -u %s",
	            progfname,mailuser) ;

	    else
	        bufprintf(cmdbuf,CMDBUFLEN,"%s -p",
	            progfname) ;

	    fpa[0] = NULL ;
	    fpa[1] = tfp ;
	    fpa[2] = NULL ;
	    if ((rs = bopencmd(fpa,cmdbuf)) >= 0) {

	        if ((rs = breadline(fpa[1],buf,BUFLEN)) > 0) {

		    len = rs ;

#if	CF_DEBUG
	            debugprintf("getspoolfile: buf> %W",buf,len) ;
#endif

		    if (buf[len - 1] == '\n')
			len -= 1 ;

	            buf[len] = '\0' ;

	            cl = sfshrink(buf,len,&cp) ;

	            if ((cl > 0) && (spoolfile != NULL))
	                strwcpy(spoolfile,cp,cl) ;

	        } /* end if */

	        bclose(fpa[1]) ;

	    } /* end if */

	} else {

	    prog_getmail = NULL ;
	    if ((spoolfile != NULL) && ((cp = getenv("MAIL")) != NULL))
	        strcpy(spoolfile,cp) ;

	} /* end if */

#if	CF_DEBUG
	debugprintf("getspoolfile: ret rs=%d spoolfile=%s\n",rs,spoolfile) ;
#endif

	return rs ;
}
/* end subroutine (getspoolfile) */



