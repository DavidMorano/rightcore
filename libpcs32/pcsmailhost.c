/* pcsmailhost */

/* get the mailhost for the host that we are on */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-05-01, David A­D­ Morano
        This code module was completely rewritten to replace any original
        garbage that was here before.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is used to find the mailhost for a given user.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<char.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	MAILHOSTFNAME
#define	MAILHOSTFNAME	"etc/mailhost"
#endif

#ifndef	VARMAILHOST
#define	VARMAILHOST	"MAILHOST"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int pcsmailhost(pcsroot,setp,username,buf)
const char	pcsroot[] ;
vecstr		*setp ;
const char	username[] ;
char		buf[] ;
{
	struct ustat	sb ;

	bfile	nfile, *nfp = &nfile ;

	int	rs = SR_OK ;
	int	len ;
	int	cl = 0 ;

	char	mailhostfname[MAXPATHLEN + 1] ;
	char	linebuf[LINEBUFLEN + 1] ;
	char	*cp, *cp2 ;


	if (pcsroot == NULL)
	    return SR_FAULT ;

	buf[0] = '\0' ;

/* try the local environment variable */

	if (((cp = getenv(VARMAILHOST)) != NULL) && (cp[0] != '\0')) {
	    cl = strwcpy(buf,cp,MAXHOSTNAMELEN) - buf ;
	    goto ret0 ;
	}

/* check if the program root directory exists */

	rs = u_stat(pcsroot,&sb) ;

	if ((rs >= 0) && (! S_ISDIR(sb.st_mode))) rs = SR_NOTFOUND ;

	if (rs >= 0)
	    rs = mkpath2(mailhostfname, pcsroot ,MAILHOSTFNAME) ;

	if (rs < 0) goto ret0 ;

	if ((rs = bopen(nfp,mailhostfname,"r",0644)) >= 0) {

	    while ((rs = breadline(nfp,linebuf,LINEBUFLEN)) > 0) {
		len = rs ;

	        if (linebuf[len - 1] == '\n')
	            len -= 1 ;

	        linebuf[len] = '\0' ;
	        cp = linebuf ;
	        while (CHAR_ISWHITE(*cp))
	            cp += 1 ;

	        if (*cp == '#') continue ;

	        cp2 = cp ;
	        while (*cp && (! CHAR_ISWHITE(*cp)))
	            cp += 1 ;

	        *cp = '\0' ;
	        cl = (cp - cp2) ;
	        if (cl > 0)
	            break ;

	    } /* end while (reading lines) */

	    bclose(nfp) ;
	} /* end if (opened file) */

	if ((rs > 0) && (cl > 0)) {

	    if (cl > MAXHOSTNAMELEN)
	        cl = MAXHOSTNAMELEN ;

	    strwcpy(buf,cp2,cl) ;

	} /* end if */

ret0:
	return (rs >= 0) ? cl : rs ;
}
/* end subroutine (pcsmailhost) */



