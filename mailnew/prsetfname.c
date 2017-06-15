/* prsetfname */

/* set program-root (oriented) file-name */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We create a file name according to some rules.

	Synopsis:

	int prsetfname(pr,fname,sp,sl,f_def,dname,name,suf)
	const char	*pr ;
	char		fname[] ;
	const char	sp[] ;
	const char	dname[], name[], suf[] ;
	int		sl ;
	int		f_def ;

	Arguments:

	pr		program-root
	fname		result buffer
	sp		source pointer
	sl		source length
	f_def		use default file-name if none (empty) is given
	dname		directory-name
	name		base-name
	suf		suffix

	Returns:

	<0		error
	>=0		length of result


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	TMPVARDNAME
#define	TMPVARDNAME	"/var/tmp"
#endif

#ifndef	VCNAME
#define	VCNAME		"var"
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sibreak(const char *,int,const char *) ;
extern int	siskipwhite(const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_log(time_t,char *) ;


/* local structures */


/* exported subroutines */


int prsetfname(pr,fname,ebuf,el,f_def,dname,name,suf)
const char	*pr ;
char		fname[] ;
const char	ebuf[] ;
const char	dname[], name[], suf[] ;
int		el ;
int		f_def ;
{
	int		rs = SR_OK ;
	int		ml ;
	const char	*np ;
	char		tmpname[MAXNAMELEN + 1] ;

	if ((f_def && (ebuf[0] == '\0')) || (strcmp(ebuf,"+") == 0)) {
	    np = name ;
	    if ((suf != NULL) && (suf[0] != '\0')) {
	        np = tmpname ;
	        mkfnamesuf1(tmpname,name,suf) ;
	    }
	    if (np[0] != '/') {
	        if ((dname != NULL) && (dname[0] != '\0')) {
	            rs = mkpath3(fname,pr,dname,np) ;
	        } else {
	            rs = mkpath2(fname,pr,np) ;
		}
	    } else {
	        rs = mkpath1(fname, np) ;
	    }
	} else if (strcmp(ebuf,"-") == 0) {
	    fname[0] = '\0' ;
	} else if (ebuf[0] != '\0') {
	    np = ebuf ;
	    if (el >= 0) {
	        np = tmpname ;
	        ml = MIN(MAXPATHLEN,el) ;
	        strwcpy(tmpname,ebuf,ml) ;
	    }
	    if (ebuf[0] != '/') {
	        if (strchr(np,'/') != NULL) {
	            rs = mkpath2(fname,pr,np) ;
	        } else {
	            if ((dname != NULL) && (dname[0] != '\0')) {
	                rs = mkpath3(fname,pr,dname,np) ;
	            } else {
	                rs = mkpath2(fname,pr,np) ;
		    }
	        } /* end if */
	    } else {
	        rs = mkpath1(fname,np) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (prsetfname) */


