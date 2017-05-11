/* findfilepath */

/* subroutine to try to find a file in the specified directory path */


#define	CF_DEBUGS	0	/* non-switchable debug print-outs */
#define	CF_PREPENDPWD	0	/* prepend PWD when encountered? */
#define	CF_FILEPATH	1	/* always return a file path */
#define	CF_FILEPATHLEN	0	/* always return a file path length */
#define	CF_SPERM	1	/* use 'sperm(3dam)' */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine was originally written.  It is based loosely on sinilar
	functions that I had before but not in as nice an interface.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine searches through all of the directories in the supplied
	directory path.  If the given file with the given mode is found in a
	directory then the path to this file is returned.

	If the directory path is specified as NULL, then the current execution
	path (given by environment variable 'PATH') is used.

	Synopsis:

	int findfilepath(path,fpath,fname,am)
	const char	path[] ;
	char		fpath[] ;
	const char	fname[] ;
	int		am ;

	Arguments:

	path		execution path or NULL to use default 'PATH'
	fpath		resulting path to the file
	fname		file to be searched for
	am		file mode like w/ u_open(2) and u_access(2)

	Returns:

	>0		program was found elsewhere and this is the path length
	0		program was found in present working directory (PWD)
	<0		program was not found or error

	fpath	resulting path if different than input


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
	int		pwdlen ;
	char		pwd[MAXPATHLEN + 1] ;
} ;


/* forward references */

static int	checkone(IDS *,struct ffp_data *,char *,cchar *,int,
			cchar *,int) ;
static int	checkit(IDS *,char *,const char *,int,const char *,int) ;
static int	fileperm(IDS *,const char *,int) ;


/* local variables */


/* exported subroutines */


int findfilepath(path,fpath,fname,am)
const char	path[] ;
char		fpath[] ;
const char	fname[] ;
int		am ;
{
	struct ffp_data	d ;
	IDS		ids ;
	int		rs ;
	int		len = 0 ;
	int		dirlen ;
	int		f_fpath = TRUE ;
	const char	*tp ;
	const char	*pp ;
	char		fpathbuf[MAXPATHLEN + 1] ;

	if ((rs = ids_load(&ids)) >= 0) {

	    if (fpath == NULL) {
	        f_fpath = FALSE ;
	        fpath = fpathbuf ;
	    }

	    fpath[0] = '\0' ;
	    if (fname[0] == '/') {

#if	CF_DEBUGS
	        debugprintf("findfilepath: rooted fname=>%s<\n",fname) ;
#endif

	        if ((rs = fileperm(&ids,fname,am)) >= 0) {
	            if (f_fpath) {
	                len = mkpath1(fpath,fname) ;
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
	                rs = checkone(&ids,&d,fpath,pp,dirlen,fname,am) ;
	                len = rs ;

	                pp = tp + 1 ;
	                if (rs >= 0) break ;
	            } /* end while */

	            if (rs < 0) {

	                dirlen = strlen(pp) ;

	                rs = checkone(&ids,&d,fpath,pp,dirlen,fname,am) ;
	                len = rs ;
	            }

#if	CF_FILEPATH
	            if ((rs <= 0) && f_fpath) {

	                if (rs == 0) {

#if	CF_FILEPATHLEN
	                    rs = mkpath1(fpath,fname) ;
	                    len = rs ;
#else
	                    mkpath1(fpath,fname) ;
#endif /* CF_FILEPATHLEN */

	                } else
	                    fpath[0] = '\0' ;

	            } /* end if */
#else
	            if ((rs <= 0) && f_fpath)
	                fpath[0] = '\0' ;
#endif /* CF_FILEPATH */

	        } /* end if (non-NULL path) */

	    } /* end if */

	    ids_release(&ids) ;
	} /* end if (ids) */

#if	CF_DEBUGS
	debugprintf("findfilepath: ret rs=%d len=%d\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (findfilepath) */


/* local subroutines */


static int checkone(idp,dp,fpath,dirpath,dirlen,fname,am)
IDS		*idp ;
struct ffp_data	*dp ;
char		fpath[] ;
const char	dirpath[] ;
int		dirlen ;
const char	fname[] ;
int		am ;
{
	int		rs = SR_OK ;

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
	        rs = checkit(idp,fpath,dp->pwd,dp->pwdlen,fname,am) ;
#else
	    rs = fileperm(idp,fname,am) ;
	    if (rs >= 0)
	        rs = 0 ;
#endif /* CF_PREPENDPWD */

	} else
	    rs = checkit(idp,fpath,dirpath,dirlen,fname,am) ;

#if	CF_DEBUGS
	debugprintf("findfilepath/checkone: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (checkone) */


static int checkit(idp,pbuf,dname,dnamelen,fname,am)
IDS		*idp ;
char		pbuf[] ;
const char	dname[] ;
int		dnamelen ;
const char	fname[] ;
int		am ;
{
	const int	plen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i = 0 ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("findfilepath/checkit: ent dir=>%t< fname=>%s<\n",
	    dname,dnamelen,fname) ;
#endif

	if (dnamelen != 0) {

#if	CF_DEBUGS
	    debugprintf("findfilepath/checkit: non-blank directory\n") ;
#endif

	    i = 0 ;
	    if (rs >= 0) {
	        rs = storebuf_strw(pbuf,plen,i,dname,dnamelen) ;
	        i += rs ;
	    }


	    if (rs >= 0) {
	        rs = storebuf_char(pbuf,plen,i,'/') ;
	        i += rs ;
	    }

	    if (rs >= 0) {
	        rs = storebuf_strw(pbuf,plen,i,fname,-1) ;
	        i += rs ;
	    }

	    if (rs >= 0) {
	        pbuf[i] = '\0' ;
	        len = i ;
	    }

#if	CF_DEBUGS
	    debugprintf("findfilepath/checkit: pathbuf=%s\n",pathbuf) ;
#endif

	    if (rs >= 0)
	        rs = fileperm(idp,pbuf,am) ;

	} else {

#if	CF_DEBUGS
	    debugprintf("findfilepath/checkit: BLANK fname=>%s<\n",fname) ;
#endif

	    rs = fileperm(idp,fname,am) ;
	    if (rs >= 0)
	        len = 0 ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("findfilepath/checkit: ret rs=%d len=%d\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (checkit) */


/* is it a file and are the permissions what we want? */
static int fileperm(idp,fname,am)
IDS		*idp ;
const char	fname[] ;
int		am ;
{
	struct ustat	sb ;
	int		rs ;

	if ((rs = u_stat(fname,&sb)) >= 0) {
	    rs = SR_NOTFOUND ;
	    if (S_ISREG(sb.st_mode))  {
#if	CF_SPERM
	        rs = sperm(idp,&sb,am) ;
#else
	        rs = perm(fname,-1,-1,NULL,am) ;
#endif
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (fileperm) */


