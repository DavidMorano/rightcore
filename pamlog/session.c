/* session */

/* PAM module */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 1989-03-01, David A­D­ Morano

	This subroutine was originally written.  This whole program, LOGDIR, is
	needed for use on the Sun CAD machines because Sun doesn't support
	LOGDIR or LOGNAME at this time.  There was a previous program but it is
	lost and not as good as this one anyway.  This one handles NIS+ also.
	(The previous one didn't.)


	= 1998-06-01, David A­D­ Morano

	I enhanced the program a little to print out some other user
	information besides the user's name and login home directory.


	= 1999-03-01, David A­D­ Morano

	I enhanced the program to also print out effective UID and effective
	GID.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a PAM module, it is only used for session management.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/utsname.h>
#include	<security/pam_appl.h>
#include	<security/pam_modules.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<vecstr.h>
#include	<logfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(mip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */

struct modinfo_flags {
	uint		nowarn:1 ;
	uint		debug:1 ;
	uint		tryfirstpass:1 ;
	uint		usefirstpass:1 ;
	uint		quiet:1 ;
} ;

struct modinfo_data {
	char		*service ;
	char		*user ;
	char		*tty ;
	char		*rhost ;
	char		*ruser ;
} ;

struct modinfo {
	struct modinfo_data	d ;
	struct modinfo_flags	f ;
	struct utsname		ui ;
	LOGFILE			lh ;
	const char	*pr ;
	pid_t		pid ;
	uid_t		uid ;
	time_t		daytime ;
	int		debuglevel ;
	int		verboselevel ;
	char		nodename[NODENAMELEN + 1] ;
	char		tty[NODENAMELEN + 1] ;
} ;


/* forward references */


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"DEBUG",
	"ROOT",
	"of",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_debug,
	argopt_root,
	argopt_of,
	argopt_overlast
} ;

/* define the configuration keywords */
static const char *qopts[] = {
	"debug",
	"nowarn"
	"try_first_pass",
	"use_first_pass",
	NULL
} ;

enum qopts {
	qopt_debug,
	qopt_nowarn,
	qopt_tryfirstpass,
	qopt_usefirstpass,
	qopt_overlast
} ;


/* exported subroutines */


int pam_sm_open_session(pamh,flags,argc,argv)
pam_handle_t	*pamh ;
int		flags ;
int		argc ;
const char	**argv ;
{
	struct modinfo	mi, *mip = &mi ;
	struct ustat	sb ;

	int	argr, argl, aol, akl, avl ;
	int	maxai, pan, npa, kwi, ai ;
	int	rs = 0 ;
	int	i, j, k ;
	int	v ;
	int	sl, cl ;
	int	ex = PAM_SUCCESS ;
	int	f_optplus, f_optminus, f_optequal ;
	int	f_version = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;

	char	argpresent[MAXARGGROUPS] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	logid[LOGIDLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*outfname = NULL ;
	const char	*sp, *cp ;


	memset(mip,0,sizeof(struct modinfo)) ;

/* start parsing the arguments */

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	argr = argc ;
	for (i = 0 ; (rs == 0) && (i < argc) ; i += 1) {

	    argr -= 1 ;
	    argp = argv[i] ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                argval = (argl - 1) ;

	            } else {

#if	CF_DEBUGS
	                debugprintf("main: got an option\n") ;
#endif

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                    debugprintf("main: option key w/ a value\n") ;
#endif

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

#if	CF_DEBUGS
	                    debugprintf("main: aol=%d avp=\"%s\" avl=%d\n",
	                        aol,avp,avl) ;

	                    debugprintf("main: akl=%d akp=\"%s\"\n",
	                        akl,akp) ;
#endif

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

	                }

/* keyword match or only key letters? */

	                if ((kwi = matostr(argopts,3,akp,akl)) >= 0) {

	                    switch (kwi) {

/* version */
	                    case argopt_version:
	                        f_version = TRUE ;
	                        if (f_optequal) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        break ;

/* verbose mode */
	                    case argopt_verbose:
	                        mip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdeci(avp,avl,&v) ;
	                                mip->verboselevel = v ;
	                            }
	                        }
	                        break ;

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                pr = avp ;
	                        } else {
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
	                                break ;
	                            }
	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        }
	                        break ;

/* output file name */
	                    case argopt_of:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                outfname = avp ;
	                        } else {
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
	                                break ;
	                            }
	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                outfname = argp ;
	                        }
	                        break ;

/* handle all keyword defaults */
	                    default:
	                        rs = SR_INVALID ;
	                        ex = EX_USAGE ;
	                        break ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {
	                        int	kc = (*akp & 0xff) ;

	                        switch (kc) {

/* debug */
	                        case 'D':
	                            mip->debuglevel = 1 ;
	                            if (f_optequal) {
	                                f_optequal = FALSE ;
	                                if (avl) {
	                                    rs = cfdeci(avp,avl,&v) ;
	                                    mip->debuglevel = v ;
	                                }
	                            }
	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* quiet mode */
	                        case 'q':
	                            mip->f.quiet = TRUE ;
	                            break ;

/* verbose mode */
	                        case 'v':
	                            mip->verboselevel = 1 ;
	                            if (f_optequal) {
	                                f_optequal = FALSE ;
	                                if (avl) {
	                                    rs = cfdeci(avp,avl,&v) ;
	                                    mip->verboselevel = v ;
	                                }
	                            }
	                            break ;

	                        default:
	                            rs = SR_INVALID ;
	                            ex = EX_USAGE ;
	                            break ;

	                        } /* end switch */

	                        akp += 1 ;
	                        if (rs < 0)
	                            break ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minux sign character alone on the command line */

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

#if	CF_DEBUGS
	            debugprintf("main: pos arg=>%s<\n",argv[i]) ;
#endif

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            rs = SR_INVALID ;
	            ex = EX_USAGE ;
	            break ;

	        }

	    } /* end if (key letter/word or positional) */

	} /* end for (all command line argument processing) */

	if (rs >= 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

		if (BATST(argpresent,i) && (argv[i][0] != '\0')) {
		    int	qi ;

		    qi = matstr(qopts,argv[i],-1) ;

		    if (qi >= 0) {
			switch (qi) {

			case qopt_debug:
				mip->f.debug = TRUE ;
				break ;

			case qopt_nowarn:
				mip->f.nowarn = TRUE ;
				break ;

			case qopt_tryfirstpass:
				mip->f.tryfirstpass = TRUE ;
				break ;

			case qopt_usefirstpass:
				mip->f.usefirstpass = TRUE ;
				break ;

			default:
				rs = SR_INVALID ;
				break ;

			} /* end switch */
		    } /* end if */

		} /* end if */

	    } /* end for */

	} /* end if */

	if (rs < 0)
	    goto badusage ;

/* what about a program root? */

	if (pr == NULL)
	    pr = PROGRAMROOT ;

	rs = u_stat(pr,&sb) ;

	if ((rs < 0) || (! S_ISDIR(sb.st_mode))) {
	    rs = SR_NOTDIR ;
	    goto badpr ;
	}

	mip->pr = pr ;
	u_uname(&mip->ui) ;

	mip->uid = getuid() ;

	mip->pid = getpid() ;

	mip->daytime = time(NULL) ;

	cl = NODENAMELEN ;
	if ((cp = strchr(mip->ui.nodename,'.')) != NULL)
	    cl = MIN((cp - mip->ui.nodename),NODENAMELEN) ;

	strwcpy(mip->nodename,mip->ui.nodename,cl) ;

/* make a log-ID (cheap) */

	cp = strwcpy(logid,mip->nodename,4) ;

	cl = ctdeci(tmpfname,20,mip->pid) ;

	cl = MIN((LOGIDLEN - (cp - logid)),cl) ;
	strwcpy(cp,tmpfname,cl) ;

/* do we have a log file? */

	mkpath2(tmpfname,mip->pr,LOGFNAME) ;

	if ((rs = logfile_open(&mip->lh,tmpfname,0,0664,logid)) >= 0) {

#ifdef	LOGSIZE
	    logfile_checksize(&mip->lh,LOGSIZE) ;
#endif

	    logfile_printf(&mip->lh,"%s session starting",
	        timestr_logz(mip->daytime,timebuf)) ;

	    if (f_version)
	        logfile_printf(&mip->lh,"version=%s",VERSION) ;

	    if (mip->verboselevel > 0) {
	        logfile_printf(&mip->lh,"uid=%u pid=%u",
	            mip->uid,mip->pid) ;
	    }

/* get the data that the application prepared for us */

	    ex = pam_get_item(pamh, PAM_SERVICE, (void **)&mi.d.service) ;

	    if ((ex == PAM_SUCCESS) && (mip->d.service != NULL))
	        logfile_printf(&mip->lh,"service=%s",mip->d.service) ;

	    ex = pam_get_item(pamh, PAM_USER, (void **)&mi.d.user) ;

	    if ((ex == PAM_SUCCESS) && (mip->d.user != NULL))
	        logfile_printf(&mip->lh,"user=%s",mip->d.user) ;

	    ex = pam_get_item(pamh, PAM_RHOST, (void **)&mi.d.rhost) ;

	    if ((ex == PAM_SUCCESS) && (mip->d.rhost != NULL) &&
		(mip->d.rhost[0] != '\0'))
	        logfile_printf(&mip->lh,"rhost=%s",mip->d.rhost) ;

	    ex = pam_get_item(pamh, PAM_RUSER, (void **)&mi.d.ruser) ;

	    if ((ex == PAM_SUCCESS) && (mip->d.ruser != NULL) &&
		(mip->d.ruser[0] != '\0'))
	        logfile_printf(&mip->lh,"ruser=%s",mip->d.ruser) ;

	    ex = pam_get_item(pamh, PAM_TTY, (void **)&mi.d.tty) ;

	    if ((ex == PAM_SUCCESS) && (mip->d.tty != NULL)) {

		cp = mip->d.tty ;
		if (strncmp(mip->d.tty,DEVPREFIX,DEVPREFIXLEN) == 0)
			cp = mip->d.tty + DEVPREFIXLEN ;

		strwcpy(mip->tty,cp,NODENAMELEN) ;

	        logfile_printf(&mip->lh,"line=%s",mip->tty) ;

	    }

	    ex = PAM_SUCCESS ;
	    logfile_close(&mip->lh) ;
	} /* end if */

ret0:
	return ex ;

/* bad stuff */
badusage:
	ex = PAM_IGNORE ;
	goto ret0 ;

badpr:
	ex = PAM_SUCCESS ;
	goto ret0 ;
}
/* end subroutine (pam_sm_open_session) */


int pam_sm_close_session(pamh,flags,argc,argv)
pam_handle_t	*pamh ;
int		flags ;
int		argc ;
const char	**argv ;
{


	return PAM_SUCCESS ;
}
/* end subroutine (pam_sm_close_session) */


