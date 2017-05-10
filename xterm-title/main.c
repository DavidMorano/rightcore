/* xterm-title */

/* place a title into the window manager title bar of an 'xterm' program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	CF_DEBUG	0


/* revision history:

	= 1991-10-01, David A­D­ Morano

	This program was originally written.


*/


/****************************************************************************

	* said above *

	Synopsis:

	$ xterm "arbitrary string"


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<signal.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>

#include	<bfile.h>
#include	<baops.h>
#include	<exitcodes.h>
#include	<mallocstuff.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		200
#endif

#define	MAXWORDS	200

#define	STRING_BEGIN	"\033]0;"
#define	STRING_END	"\007"

#define	XB_BOTH		"\033]0;"
#define	XE_BOTH		"\007"

#define	XB_ICON		"\033]1;"
#define	XE_ICON		"\007"

#define	XB_TITLE	"\033]2;"
#define	XE_TITLE	"\007"

#ifndef	VARTERM
#define	VARTERM		"TERM"
#endif



/* external functions */

extern char	*getlogin() ;
extern char	*strbasename(char *) ;


/* forward references */


/* local variables */






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct ustat		sb ;

	struct utsname		uts ;

	struct passwd		*pp ;

	bfile	errfile, *efp = &errfile ;

	uid_t	uid ;

	int	argl, aol ;
	int	npa, i, ai ;
	int	maxai ;
	int	rs ;
	int	len ;
	int	l, pn ;
	int	ol ;
	int	ofd = 1 ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_debug = FALSE, f_verbose = FALSE, f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_got = FALSE ;
	int	f_extra = FALSE ;
	int	f_both = FALSE ;

	char	*argp, *aop ;
	char	*progname ;
	char	*term = getenv(VARTERM) ;
	char	*logname ;
	char	buf[BUFLEN], *bp ;
	char	arg_word[(MAXWORDS/8) + 1] ;
	char	*uts_host, *uts_domain ;
	char	*username ;
	char	*both = NULL, *icon = NULL, *title = NULL ;
	char	*cp ;


	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"wca",0664) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;


/* initialize the argument tracking array */

	rs = SR_OK ;
	for (i = 0 ; i < ((MAXWORDS/8) + 1) ; i += 1) 
		arg_word[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argc -= 1 ;

	while ((rs >= 0) && (argc > 0)) {

	    argp = argv[++i] ;
	    argc -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            aop = argp ;
	            aol = argl ;
	            while (--aol) {

	                akp += 1 ;
	                switch ((int) *aop) {

	                case 'D':
	                    f_debug = TRUE ;
	                    break ;

	                case 'V':
	                    bprintf(efp,"%s: version %s\n",
	                        progname,VERSION) ;

	                    break ;

	                case 'v':
	                    f_verbose = TRUE ;
	                    break ;

	                case 'b':
	                    f_both = TRUE ;
	                    break ;

	                case 'i':
	                    if (argc <= 0) goto badargnum ;

	                    argp = argv[++i] ;
	                    argc -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl) {

	                        icon = argp ;
	                        f_got = TRUE ;
	                    }

	                    break ;

	                case 't':
	                    if (argc <= 0) goto badargnum ;

	                    argp = argv[++i] ;
	                    argc -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl) {

	                        title = argp ;
	                        f_got = TRUE ;
	                    }

	                    break ;

	                default:
	                    bprintf(efp,"%s: unknown option - %c\n",
	                        progname,*aop) ;

	                case '?':
	                    f_usage = TRUE ;

	                } /* end switch */

	            } /* end while */

	        } else {

	            if (i < MAXWORDS) {

	                BASET(arg_word,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXWORDS) {

	            BASET(arg_word,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                bprintf(efp,"%s: extra arguments ignored\n",
	                    progname) ;

	            }
	        }

	    } /* end if */

	} /* end while */

#if	CF_DEBUGS
	debugprintf("main: finished parsing arguments \n") ;
#endif

#if	CF_DEBUG
	if (f_debug)
	    bprintf(efp,"%s: finished parsing arguments\n",
	        progname) ;
#endif

	if (f_usage) 
		goto usage ;

	if (f_debug) {

	    if (f_both) bprintf(efp,"%s: the BOTH flag is set\n",
	        progname) ;

	}

	ex = EX_OK ;

/* check arguments */

	if (f_debug) 
		f_verbose = TRUE ;

/* punt if we are not on an XTERM */

	if ((term == NULL) ||
	    (strncmp(term,"xterm",5) != 0)) {

	    if (f_verbose)
	        bprintf(efp,"%s: terminal is not an 'xterm' ?\n",
	            progname) ;

	    goto badret ;
	}

/* punt if standard output is not a character device */

	if ((! isatty(ofd)) &&
		((cp = getenv("DISPLAY")) == NULL)) {

	    if (f_verbose)
	        bprintf(efp,"%s: output is not to an X terminal\n",
	            progname) ;

	    goto badret ;
	}


/* OK, we proceed to get information about who we are and where we are */

	u_uname(&uts) ;

	uts_host = uts.nodename ;
	uts_domain = "att.com" ;
	if ((cp = strchr(uts.nodename,'.')) != NULL) {

	    *cp++ = '\0' ;
	    uts_domain = cp ;
	}

/* initialize some basic stuff */

	if ((! f_got) && (npa <= 0)) {

	    uid = getuid() ;

/* try to get a passwd entry */

	    username = NULL ;
	    if ((cp = getenv("LOGNAME")) != NULL) {

	        if ((pp = getpwnam(cp)) != NULL) {

	            if (pp->pw_uid == uid) username = cp ;

	        }
	    }

	    if ((username == NULL) && ((cp = getlogin()) != NULL)) {

	        if ((pp = getpwnam(cp)) != NULL) {

	            if (pp->pw_uid == uid) username = mallocstr(cp) ;

	        }
	    }

	    if (username == NULL) {

	        if ((pp = getpwuid(uid)) != NULL)
	            username = mallocstr(pp->pw_name) ;

	        else
	            username = "*unknown*" ;

	    }


	    if (f_debug)
		 bprintf(efp,"%s: username found \"%s:%s\"\n",
	        progname,uts_host,username) ;

	    if (username != NULL) {

	        len = bufprintf(buf,BUFLEN,
			"%s%s:%s%s",
	            STRING_BEGIN,uts_host,username,STRING_END) ;

	    } else {

	        len = bufprintf(buf,BUFLEN,
			"%s%s%s",
	            STRING_BEGIN,uts_host,STRING_END) ;

	    }

	    write(ofd,buf,len) ;

	} /* end if (default title) */


/* miscellaneous calculation */

	ol = MAX((int) strlen(XE_BOTH),(int) strlen(XE_TITLE)) ;

	ol = MAX(ol,(int) strlen(XE_ICON)) ;


/* standard command line title ? */

	if (npa > 0) {

	    pn = 0 ;
	    bp = buf ;
	    len = 0 ;
	    for (ai = 1 ; ai <= maxai ; ai += 1) {

#if	CF_DEBUG
	if (f_debug) debugprintf(
	"main: top of argument process loop\n") ;
#endif

	        if (! BATST(arg_word,ai)) 
			continue ;

	        if ((argv[ai] == NULL) || (*argv[i] == '\0')) 
			continue ;

	        if (pn == 0) {

	            if (f_both) 
			len += bufprintf(buf,BUFLEN,
				"%s",XB_BOTH) ;

	            else 
			len += bufprintf(buf,BUFLEN,
				"%s",XB_TITLE) ;

	            bp = buf + len ;

	        } else {

	            *bp++ = ' ' ;
	            len += 1 ;

	        }

	        l = strlen(argv[ai]) ;

	        if ((len + l) > (BUFLEN - ol)) 
			l = (BUFLEN - ol) - len ;

	        strncpy(bp,argv[ai],l) ;

	        bp += l ;
	        len += l ;
	        pn += 1 ;

#if	CF_DEBUG
	if (f_debug) debugprintf(
	"main: bottom of argument process loop\n") ;
#endif

	    } /* end for */

	    if (pn > 0) {

	        len += bufprintf(bp,BUFLEN,
			"%s",(f_both) ? XE_BOTH : XE_TITLE) ;

	        write(ofd,buf,len) ;

	    }

	} /* end if */

/* icon name ? */

	if ((! f_both) && (icon != NULL)) {

	    len = bufprintf(buf,BUFLEN,
		"%s%W%s",XB_ICON,
	        icon,MIN((int) strlen(icon),BUFLEN - ol),
	        XE_ICON) ;

	    write(ofd,buf,len) ;

	}

/* done */
ret0:
	bclose(efp) ;

	return ex ;

/* bad stuff */
badret:
	ex = EX_DATAERR ;
	goto ret0 ;

usage:
	bprintf(efp,
	    "%s: USAGE> %s [title_string] [-i icon_name] [-b] [-?VD]\n",
	    progname,progname) ;

	bprintf(efp,
	    "\ttitle_string     name that appears in title bars\n") ;

	bprintf(efp,
	    "\t-i icon_name     name that appears in icons\n") ;

	bprintf(efp,
	    "\t-b               option makes string both icon and title\n") ;

	goto badret ;

badarg:
	bprintf(efp,"%s: bad argument given\n",progname) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",progname) ;

	goto badret ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



