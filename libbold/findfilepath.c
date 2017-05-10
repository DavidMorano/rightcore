/* findfilepath */

/* subroutine to try to find a file in the specified directory path */


#define	CF_DEBUGS	0	/* non-switchable debug print-outs */
#define	CF_PREPENDPWD	0	/* prepend PWD when encountered? */
#define	CF_FILEPATH	1	/* always return a file path */
#define	CF_FILEPATHLEN	0	/* always return a file path length */
#define	CF_SPERM	1	/* use 'sperm(3dam)' */


/* revision history:

	= 1998-05-01, David A­D­ Morano

	This subroutine was originally written.  It is based loosely
	on sinilar functions that I had before but not in as nice an
	interface.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine searches through all of the directories in the
	supplied directory path.  If the given file with the given mode
	is found in a directory then the path to this file is
	returned.

	If the directory path is specified as NULL, then the current
	execution path (given by environment variable 'PATH') is used.

	Synopsis:

	int findfilepath(path,filepath,mode,fname)
	const char	path[] ;
	char		filepath[] ;
	int		mode ;
	const char	fname[] ;

	Arguments:

	path		execution path or NULL to use default 'PATH'
	filepath	resulting path to the file
	mode		file mode like w/ u_open(2) and u_access(2)
	fname		file to be searched for

	Returns:

	>0		program was found elsewhere and this is the path length
	0		program was found in present working directory (PWD)
	<0		program was not found or error

	filepath	resulting path if different than input


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<ids.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getpwd(char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct ffp_data {
	int	pwdlen ;
	char	pwd[MAXPATHLEN + 1] ;
} ;


/* forward references */

static int	checkone(IDS *,struct ffp_data *,const char *,int,
			const char *, int,char *) ;
static int	checkit(IDS *,const char *,int,const char *,int,char *) ;
static int	fileperm(IDS *,const char *,int) ;


/* local variables */


/* exported subroutines */


int findfilepath(path,filepath,mode,fname)
const char	path[] ;
char		filepath[] ;
int		mode ;
const char	fname[] ;
{
	struct ffp_data	d ;

	IDS	ids ;

	int	rs ;
	int	len = 0 ;
	int	dirlen ;
	int	f_filepath = TRUE ;

	const char	*tp ;
	const char	*pp ;

	char	filepathbuf[MAXPATHLEN + 1] ;


	rs = ids_load(&ids) ;
	if (rs < 0)
	    goto ret0 ;

	if (filepath == NULL) {
	    f_filepath = FALSE ;
	    filepath = filepathbuf ;
	}

	filepath[0] = '\0' ;
	if (fname[0] == '/') {

#if	CF_DEBUGS
	    debugprintf("findfilepath: rooted fname=>%s<\n",fname) ;
#endif

	    rs = fileperm(&ids,fname,mode) ;
	    if (rs >= 0) {
	        if (f_filepath) {
		    len = mkpath1(filepath,fname) ;
	        } else
	            len = strlen(fname) ;
	    }

	} else {

	    d.pwd[0] = '\0' ;
	    d.pwdlen = -1 ;

	    if (path == NULL)
	        path = getenv(VARPATH) ;

	    rs = SR_NOTFOUND ;
	    if (path != NULL) {

	        pp = path ;
	        while ((tp = strchr(pp,':')) != NULL) {

	            dirlen = tp - pp ;
	            rs = checkone(&ids,&d,pp,dirlen,fname,mode,filepath) ;
	            len = rs ;
	            if (rs >= 0)
	                break ;

	            pp = tp + 1 ;

	        } /* end while */

	        if (rs < 0) {

	            dirlen = strlen(pp) ;

	            rs = checkone(&ids,&d,pp,dirlen,fname,mode,filepath) ;
	            len = rs ;
	        }

#if	CF_FILEPATH
	        if ((rs <= 0) && f_filepath) {

	            if (rs == 0) {

#if	CF_FILEPATHLEN
	                rs = mkpath1(filepath,fname) ;
			len = rs ;
#else
	                mkpath1(filepath,fname) ;
#endif /* CF_FILEPATHLEN */

	            } else
	                filepath[0] = '\0' ;

	        } /* end if */
#else
	        if ((rs <= 0) && f_filepath)
	            filepath[0] = '\0' ;
#endif /* CF_FILEPATH */

	    } /* end if (non-NULL path) */

	} /* end if */

	ids_release(&ids) ;

ret0:

#if	CF_DEBUGS
	debugprintf("findfilepath: ret rs=%d len=%d\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (findfilepath) */


/* local subroutines */


static int checkone(idp,dp,dirpath,dirlen,fname,mode,filepath)
IDS		*idp ;
struct ffp_data	*dp ;
const char	dirpath[], fname[] ;
int		dirlen ;
int		mode ;
char		filepath[] ;
{
	int	rs = SR_OK ;


	if (dirlen == 0) {

#if	CF_DEBUGS
	    debugprintf("findfilepath/checkone: got a null directory\n") ;
#endif

#if	CF_PREPENDPWD
	    if (dp->pwdlen < 0) {

	        rs = getpwd(dp->pwd,MAXPATHLEN) ;
	        dp->pwdlen = rs ;

	    } /* end if */

	    if (dp->pwdlen >= 0)
	        rs = checkit(idp,dp->pwd,dp->pwdlen,fname,mode,filepath) ;
#else
	    rs = fileperm(idp,fname,mode) ;
	    if (rs >= 0)
	        rs = 0 ;
#endif /* CF_PREPENDPWD */

	} else
	    rs = checkit(idp,dirpath,dirlen,fname,mode,filepath) ;

#if	CF_DEBUGS
	debugprintf("findfilepath/checkone: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (checkone) */


static int checkit(idp,dname,dnamelen,fname,mode,pathbuf)
IDS		*idp ;
const char	dname[] ;
int		dnamelen ;
const char	fname[] ;
int		mode ;
char		pathbuf[] ;
{
	int	rs = SR_OK ;
	int	i = 0 ;
	int	len = 0 ;


#if	CF_DEBUGS
	debugprintf("findfilepath/checkit: entered dir=>%t< fname=>%s<\n",
	    dname,dnamelen,fname) ;
#endif

	if (dnamelen != 0) {

#if	CF_DEBUGS
	    debugprintf("findfilepath/checkit: non-blank directory\n") ;
#endif

	    i = 0 ;
	    if (rs >= 0) {
	        rs = storebuf_strw(pathbuf,MAXPATHLEN,i,dname,dnamelen) ;
	        i += rs ;
	    }


	    if (rs >= 0) {
	        rs = storebuf_char(pathbuf,MAXPATHLEN,i,'/') ;
	        i += rs ;
	    }

	    if (rs >= 0) {
	        rs = storebuf_strw(pathbuf,MAXPATHLEN,i,fname,-1) ;
	        i += rs ;
	    }

	    if (rs >= 0) {
	        pathbuf[i] = '\0' ;
	        len = i ;
	    }

#if	CF_DEBUGS
	    debugprintf("findfilepath/checkit: pathbuf=%s\n",pathbuf) ;
#endif

	    if (rs >= 0)
	        rs = fileperm(idp,pathbuf,mode) ;

	} else {

#if	CF_DEBUGS
	    debugprintf("findfilepath/checkit: BLANK fname=>%s<\n",fname) ;
#endif

	    rs = fileperm(idp,fname,mode) ;
	    if (rs >= 0)
	        len = 0 ;

	} /* end if */

ret0:

#if	CF_DEBUGS
	debugprintf("findfilepath/checkit: ret rs=%d len=%d\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (checkit) */


/* is it a file and are the permissions what we want? */
static int fileperm(idp,fname,mode)
IDS		*idp ;
const char	fname[] ;
int		mode ;
{
	struct ustat	sb ;

	int	rs ;


	if ((rs = u_stat(fname,&sb)) >= 0) {
	    rs = SR_NOTFOUND ;
	    if (S_ISREG(sb.st_mode))  {
#if	CF_SPERM
	        rs = sperm(idp,&sb,mode) ;
#else
	        rs = perm(fname,-1,-1,NULL,mode) ;
#endif
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (fileperm) */



