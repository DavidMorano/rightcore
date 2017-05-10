/* findfilepath */

/* subroutine to try to find a file in the specified directory path */


#define	CF_DEBUGS	0	/* non-switchable debug print-outs */
#define	F_PREPENDPWD	0	/* prepend PWD when encountered ? */
#define	F_FILEPATH	1	/* always return a file path */
#define	F_FILEPATHLEN	0	/* always return a file path length */


/* revision history:

	= 95/05/01, David A­D­ Morano

	This subroutine was originally written.  It is based
	loosely on sinilar functions that I had before but not
	in as nice an interface.


	= 98/05/20, David A­D­ Morano

	I modified this subroutine to treat an empty path string to be
	the current directory.


*/


/******************************************************************************

	This subroutine searches through all of the directories in the
	supplied directory path.  If the given file with the given mode
	is found in a directory then the path to this file is
	returned.

	If the directory path is specified as NULL, then the current
	execution path (given by environment variable 'PATH') is used.


	Synopsis:

	int findfilepath(path,fname,mode,filepath)
	char	path[] ;
	char	fname[] ;
	int	mode ;
	char	filepath[] ;


	Arguments:

	path		execution path or NULL to use default 'PATH'
	fname		file to be searched for
	mode		file mode like w/ open(2) & access(2)
	filepath	resulting path to the file


	Returns:

	<0		program was not found or error
	0		program was found in present working directory (PWD)
	>0		program was found elsewhere and this is the path length

	filepath	resulting path if different than input


******************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>

#include	"localmisc.h"



/* local defines */



/* external subroutines */

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

static int	checkone(struct ffp_data *,char *,int,char *,int,char *) ;
static int	checkit(char *,int,char *,int,char *) ;
static int	fileandperm(const char *,int) ;






int findfilepath(path,fname,mode,filepath)
char	path[] ;
char	fname[] ;
int	mode ;
char	filepath[] ;
{
	struct ffp_data	d ;

	int	rs, len ;
	int	dirlen ;
	int	f_filepath = TRUE ;

	char	filepathbuf[MAXPATHLEN + 1] ;
	char	*pp, *cp ;


#if	CF_DEBUGS
	{
	    char	modestr[10] ;

	    rs = 0 ;
	    if (mode & R_OK)
	        modestr[rs++] = 'r' ;

	    if (mode & W_OK)
	        modestr[rs++] = 'w' ;

	    if (mode & X_OK)
	        modestr[rs++] = 'x' ;

	    modestr[rs] = '\0' ;
	    debugprintf("findfilepath: mode=%s\n",modestr) ;
	}
#endif /* CF_DEBUGS */

	if (filepath == NULL) {

	    f_filepath = FALSE ;
	    filepath = filepathbuf ;

	}

	filepath[0] = '\0' ;
	if (fname[0] == '/') {

#if	CF_DEBUGS
	    debugprintf("findfilepath: rooted fname=>%s<\n",fname) ;
#endif

	    if ((rs = fileandperm(fname,mode)) < 0) {

#if	CF_DEBUGS
	        debugprintf("findfilepath: fileandperm() rs=%d\n",rs) ;
#endif

	        return rs ;
	    }

	    if (f_filepath)
	        len = strwcpy(filepath,fname,(MAXPATHLEN - 1)) - filepath ;

	    else
	        len = strlen(fname) ;

	    return len ;

	} /* end if */

	d.pwd[0] = '\0' ;
	d.pwdlen = -1 ;

	if (path == NULL)
	    path = getenv("PATH") ;

	rs = SR_NOTFOUND ;
	if (path != NULL) {

	    pp = path ;
	    while ((cp = strchr(pp,':')) != NULL) {

	        dirlen = cp - pp ;
	        rs = checkone(&d,pp,dirlen,fname,mode,filepath) ;

#if	CF_DEBUGS
	        debugprintf("findfilepath: loop rs=%d\n",rs) ;
#endif

	        if (rs >= 0)
	            return rs ;

	        pp = cp + 1 ;

	    } /* end while */

	    dirlen = strlen(pp) ;

	    rs = checkone(&d,pp,dirlen,fname,mode,filepath) ;

#if	F_FILEPATH
	    if ((rs <= 0) && f_filepath) {

	        if (rs == 0) {

#if	F_FILEPATHLEN
	            rs = strwcpy(filepath,fname,(MAXPATHLEN - 1)) - filepath ;
#else
	            (void) strwcpy(filepath,fname,(MAXPATHLEN - 1)) ;
#endif /* F_FILEPATHLEN */

	        } else
	            filepath[0] = '\0' ;

	    }
#else
	    if ((rs <= 0) && f_filepath)
	        filepath[0] = '\0' ;
#endif /* F_FILEPATH */

	} /* end if (non-NULL path) */

#if	CF_DEBUGS
	debugprintf("findfilepath: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (findfilepath) */



/* LOCAL SUBROUTINES */



static int checkone(dp,dirpath,dirlen,fname,mode,filepath)
struct ffp_data	*dp ;
int	dirlen ;
char	dirpath[], fname[] ;
int	mode ;
char	filepath[] ;
{
	int	rs ;


	if (dirlen == 0) {

#if	CF_DEBUGS
	    debugprintf("findfilepath/checkone: got a null directory\n") ;
#endif

#if	F_PREPENDPWD
	    if (dp->pwdlen < 0) {

	        dp->pwdlen = getpwd(dp->pwd,(MAXPATHLEN - 1)) ;

	        rs = dp->pwdlen ;

	    } /* end if */

	    if (dp->pwdlen >= 0)
	        rs = checkit(dp->pwd,dp->pwdlen,fname,mode,filepath) ;
#else
	    if ((rs = fileandperm(fname,mode)) >= 0)
	        rs = 0 ;
#endif /* F_PREPENDPWD */

	} else
	    rs = checkit(dirpath,dirlen,fname,mode,filepath) ;

#if	CF_DEBUGS
	debugprintf("findfilepath/checkone: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (checkone) */


static int checkit(dir,dirlen,fname,mode,pathbuf)
char	dir[] ;
int	dirlen ;
char	fname[] ;
int	mode ;
char	pathbuf[] ;
{
	int	rs, i, len ;


#if	CF_DEBUGS
	debugprintf("findfilepath/checkit: entered dir=>%t< fname=>%s<\n",
	    dir,dirlen,fname) ;
#endif

	if (dirlen != 0) {

#if	CF_DEBUGS
	    debugprintf("findfilepath/checkit: non-blank directory\n") ;
#endif

	    i = 0 ;
	    rs = storebuf_strw(pathbuf,(MAXPATHLEN - 1),i,dir,dirlen) ;

	    if (rs < 0)
	        goto ret0 ;

	    i += rs ;

#if	CF_DEBUGS
	    debugprintf("findfilepath/checkit: 1 rs=%d i=%d\n",rs,i) ;
#endif

	    rs = storebuf_char(pathbuf,(MAXPATHLEN - 1),i,'/') ;

	    if (rs < 0)
	        goto ret0 ;

	    i += rs ;
	    rs = storebuf_strw(pathbuf,(MAXPATHLEN - 1),i,fname,-1) ;

	    if (rs < 0)
	        goto ret0 ;

	    i += rs ;
	    pathbuf[i] = '\0' ;
	    len = i ;

#if	CF_DEBUGS
	    debugprintf("findfilepath/checkit: pathbuf=%s\n",pathbuf) ;
#endif

	    rs = fileandperm(pathbuf,mode) ;

	} else {

#if	CF_DEBUGS
	    debugprintf("findfilepath/checkit: fname=>%s<\n",fname) ;
#endif

	    if ((rs = fileandperm(fname,mode)) >= 0)
	        len = 0 ;

	} /* end if */

ret0:

#if	CF_DEBUGS
	debugprintf("findfilepath/checkit: rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (checkit) */


/* is it a file and are the permissions what we want ? */
static int fileandperm(fname,mode)
const char	fname[] ;
int		mode ;
{
	struct ustat	sb ;

	int	rs ;


#if	CF_DEBUGS
	debugprintf("findfilepath/fileandperm: fname=>%s<\n",fname) ;
#endif

	rs = u_stat(fname,&sb) ;

#if	CF_DEBUGS
	debugprintf("findfilepath/fileandperm: u_stat() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    rs = SR_NOTFOUND ;
	    if (S_ISREG(sb.st_mode)) 
		rs = perm(fname,-1,-1,NULL,mode) ;

#if	CF_DEBUGS
	debugprintf("findfilepath/fileandperm: perm() rs=%d\n",rs) ;
#endif

	}

	return rs ;
}
/* end subroutine (fileandperm) */



