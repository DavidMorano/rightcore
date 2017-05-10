/* pcsngdir */

/* return a directory name when given a newsgroup name */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	CF_DEBUG	0


/* revision history:

	= 86/07/01, David A­D­ Morano

	This subroutine was originally written.


*/


/**************************************************************************

	This program will take its argument to be a newsgroup name.
	The program will write on standard outpur the corresponding
	directory name in the spool area.

	The 'PCS' variable must be set.



***************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<time.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<bfile.h>
#include	<exitcodes.h>
#include	<mallocstuff.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	LINELEN		256
#define	NPARG		2



/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;

extern char	*strbasename(char *) ;


/* external variables */


/* local structures */

static const char *cmdopts[] = {
	"VERSION",
	NULL
} ;

enum cmdopts {
	cmdopt_version,
	cmdopt_overlast
} ;








int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct ustat	statbuf ;

	struct proginfo	pi, *pip = &pi ;

	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;

	int	argr, argl, aol, avl, kwi ;
	int	pan ;
	int	i ;
	int	minusval ;
	int	len, line, cline ;
	int	rs ;
	int	ex = EX_USAGE ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_debug = FALSE ;
	int	f_usage= FALSE ;
	int	f_dash = FALSE ;
	int	f_ddash = FALSE ;
	int	f_bol, f_eol ;
	int	f_first ;

	char	*argp, *aop, *akp, *avp ;
	char	*progname ;
	char	*ofname ;
	char	*bp, buf[LINELEN] ;
	char	tmpfnamebuf[MAXPATHLEN + 1] ;
	char	*newsgroup = NULL ;
	char	*newsdir = NULL ;
	char	*cp, *cp2 ;


	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;

/* initial initialization */

	tmpfnamebuf[0] = '\0' ;
	ofname = BFILE_STDOUT ;

/* do the argument thing */

	rs = SR_OK ;
	pan = 0 ;			/* number of positional so far */
	i = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

#if	CF_DEBUGS
	    debugprintf("main: top of outer while, i=%d argr=%d \"%s\"\n",
		i,argr,argv[i]) ;
#endif

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

#if	CF_DEBUGS
	    debugprintf("main: processing, i=%d argr=%d \"%s\"\n",
		i,argr,argv[i]) ;
#endif

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

#if	CF_DEBUGS
	            debugprintf("main: got an option\n") ;
#endif

	            aop = argp + 1 ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                debugprintf("main: got an option key w/ a value\n") ;
#endif

	                aol = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                f_optequal = TRUE ;

	            } else
	                avl = 0 ;

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matstr(cmdopts,aop,aol)) >= 0) {

	                switch (kwi) {

	                case cmdopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal) 
				goto badargextra ;

	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
			    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        f_debug = TRUE ;
	                        break ;

/* alternate newsgroup spool area */
	                    case 'N':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            newsdir = NULL ;
	                            if (avl) 
					newsdir = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					newsdir = argp ;

	                        }

	                        break ;


	                    default:
				ex = EX_USAGE ;
				f_usage = TRUE ;
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            progname, *aop) ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    } /* end switch */

#if	CF_DEBUGS
	debugprintf("main: out of switch\n") ;
#endif

	                    akp += 1 ;

	                } /* end while (inner) */

#if	CF_DEBUGS
	debugprintf("main: out of bottom inner while, i=%d\n",i) ;
#endif

	            } /* end if (key word or letter option) */

	        } else {

	            f_dash = TRUE ;
	            pan += 1 ;		/* increment position count */

	        } /* end if (handling a non-normal positional) */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                if (argl > 0) 
				newsgroup = argp ;

	                break ;

	            } /* end switch */

	            pan += 1 ;

	        } else {

	            bprintf(efp,"%s: extra arguments ignored\n",progname) ;

	        }

	    } /* end if (option or positional) */

#if	CF_DEBUGS
	debugprintf("main: bottom of outer while, i=%d\n",i) ;
#endif

	} /* end while (outer) */

#if	CF_DEBUGS
	debugprintf("main: out of outer while, i=%d\n",i) ;
#endif

	if (f_version)
	    bprintf(efp,
		"%s: version %s\n",progname,VERSION) ;

	ex = EX_INFO ;
	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto retearly ;

/* open the output file */

	rs = bopen(ofp,ofname,"wct",0666) ;
	if (rs < 0)
		goto badout ;

/* our program root */

	if ((pip->pcs = getenv(VARPROGRAMROOT)) == NULL)
		pip->pcs = PCS ;

/* check on the validity of the News Spool directory */

	if (newsdir == NULL) {

	        mkpath2(tmpfnamebuf,pip->pcs,NEWSDNAME) ;

	        newsdir = mallocstr(tmpfnamebuf) ;

	} /* end if (make a NEWSDIR) */

	if ((u_stat(newsdir,&statbuf) < 0) ||
	    (! S_ISDIR(statbuf.st_mode))) 
		goto badnewsdir ;

#if	CF_DEBUG
	debugprintf("main: about to do it\n") ;
#endif

/* check for not newsgroup given on command line */

	rs = BAD ;
	if (newsgroup == NULL) 
		goto done ;

/* assume the newsgroup is "add directory path" for starters */

	cp2 = newsgroup ;
	while ((cp = strchr(cp2,'.')) != NULL) {

	    *cp = '/' ;
	    cp2 = cp + 1 ;

	} /* end while */

/* OK, start looking for the closest directory that matches */

#if	CF_DEBUG
	debugprintf("main: about to loop\n") ;
#endif

	f_first = TRUE ;
	rs = TRUE ;
	while (f_first || ((cp = strrchr(newsgroup,'/')) != NULL)) {

	    if (! f_first) 
			*cp = '.' ;

	    f_first = FALSE ;

	    mkpath2(tmpfnamebuf,newsdir,newsgroup) ;

	    if ((u_stat(tmpfnamebuf,&statbuf) >= 0) && 
		S_ISDIR(statbuf.st_mode) &&
	        (u_access(tmpfnamebuf,W_OK) >= 0)) {

	        rs = OK ;
	        break ;
	    }

	} /* end while */

/* if we do not have a directory yet, try the last name we are left with */

	if (rs != OK) {

	    mkpath2(tmpfnamebuf,newsdir,newsgroup) ;

	    if ((u_stat(tmpfnamebuf,&statbuf) >= 0) && 
		S_ISDIR(statbuf.st_mode) &&
	        (u_access(tmpfnamebuf,W_OK) >= 0))
	        rs = OK ;

	}

	ex = EX_NOUSER ;
	if (rs == OK) {

		ex = EX_OK ;
	    bprintf(ofp,"%s\n",newsgroup) ;

	}

/* done */
done:
ret2:
	bclose(ofp) ;

retearly:
ret1:
	bclose(efp) ;

ret0:
	return ex ;

usage:
	bprintf(efp,
	    "%s: USAGE> %s [-VD?] [-N newsspooldir] newsgroup\n",
	    progname,progname) ;

	goto retearly ;

badargnum:
	bprintf(efp,"%s: not enough arguments given\n",
		progname) ;

	goto badarg ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto retearly ;

badnewsdir:
	bprintf(efp,"%s: bad news spool directory \"%s\"\n",
	    progname,newsdir) ;

	goto badret ;

badout:
	bprintf(efp,"%s: can't open outfile (%d)\n",
		progname,rs) ;

	goto badret ;

badret:
	ex = EX_DATAERR ;
	goto retearly ;
}
/* end subroutine (main) */



