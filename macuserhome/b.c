/* b_userhome (for Mac) */

/* SHELL built-in: return various user information */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	1		/* switchable at invocation */
#define	CF_OUTPUT	1		/* allowing output */


/* revision history:

	= 1989-03-01, David A.D. Morano

	This subroutine was originally written.


*/

/* Copyright © 1989 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ userhome [[<username>|-] 


*****************************************************************************/


#include	<envstandards.h>	/* must be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<vecstr.h>
#include	<ffile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_userhome.h"


/* local defines */

#ifndef	LOGNAMELEN
#ifdef	LOGNAME_MAX
#define	LOGNAMELEN	LOGNAME_MAX
#else
#define	LOGNAMELEN	32
#endif
#endif

#ifndef	USERNAMELEN
#ifdef	LOGNAME_MAX
#define	USERNAMELEN	LOGNAME_MAX
#else
#define	USERNAMELEN	32
#endif
#endif

#ifndef	ENVBUFLEN
#define	ENVBUFLEN	(MAXPATHLEN + MAXNAMELEN)
#endif

#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"

#define	LOCINFO		struct locinfo

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(lip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,int) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	nextfield(const char *,int,char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	ctdeci(const char *,int,int) ;
extern int	ctdecl(const char *,int,long) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	gecosname(const char *,char *,int) ;
extern int	fbwrite(FILE *,const char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct locinfo_flags {
	uint		quiet:1 ;
} ;

struct locinfo {
	FILE		*efp ;
	const char	*progname ;
	const char	*pr ;
	const char	*sn ;
	struct locinfo_flags	f ;
	vecstr		stores ;
	int		progmode ;
	int		debuglevel ;
	int		verboselevel ;
} ;


/* forward references */

static int	usage(struct locinfo *) ;
static int	printhelp(struct locinfo *,const char *) ;

static int	locinfo_start(LOCINFO *,const char **) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(struct locinfo *,const char **,
			const char *,int) ;

static int	procname(struct locinfo *,FILE *,const char *) ;
static int	getusername(char *,int) ;


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"HELP",
	"pm",
	"sn",
	"af",
	"ef",
	"of",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_pm,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_overlast
} ;

static const char	*progmodes[] = {
	"userhome",
	"username",
	"userdir",
	"logdir",
	NULL
} ;

enum progmodes {
	progmode_userhome,
	progmode_username,
	progmode_userdir,
	progmode_logdir,
	progmode_overlast
} ;


/* exported subroutines */


int p_userhome(argc,argv,envv,contextp)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
void		*contextp ;
{
	struct locinfo	li, *lip = &li ;

	BITS	pargs ;

	FILE	*ofp = stdout ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan = 0 ;
	int	rs = SR_OK ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	usernamebuf[USERNAMELEN + 1] ;
	const char	*pr = NULL ;
	const char	*pm = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = locinfo_start(lip,argv) ;
	if (rs < 0) goto badlocstart ;

	lip->efp = stderr ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc ;
	for (ai = 0 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = argv[ai] ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {

	        if (isdigit(argp[1])) {

	            argval = (argp+1) ;

	        } else if (argp[1] == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	            aop = argp + 1 ;
	            aol = argl - 1 ;
	            akp = aop ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {
	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + aol - avp ;
	                f_optequal = TRUE ;
	            } else {
	                akl = aol ;
	                avl = 0 ;
	            }

/* keyword match or only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* verbose mode */
	                case argopt_verbose:
	                    lip->verboselevel = 1 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optvalue(avp,avl) ;
	                            lip->verboselevel = rs ;
	                        }
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* program mode */
	                case argopt_pm:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pm = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pm = argp ;
	                    }
	                    break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sn = argp ;
	                    }
	                    break ;

/* argument file */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            afname = argp ;
	                    }
	                    break ;

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            efname = argp ;
	                    }
	                    break ;

/* output file name */
	                case argopt_of:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            ofname = argp ;
	                    }
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch */

	            } else {

	                while (akl--) {
			    int	kc = (*akp & 0xff) ;

	                    switch (kc) {

/* debug */
	                    case 'D':
	                        lip->debuglevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                lip->debuglevel = rs ;
	                            }
	                        }
	                        break ;

/* quiet mode */
	                    case 'Q':
	                        lip->f.quiet = TRUE ;
	                        break ;

/* program-root */
	                    case 'R':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* verbose mode */
	                    case 'v':
	                        lip->verboselevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                lip->verboselevel = rs ;
	                            }
	                        }
	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits as argument or not) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if ((cp = getenv(VAREFNAME)) != NULL)
	    lip->efp = fopen(cp,"w") ;

	if (rs < 0)
	    goto badarg ;

	if (lip->debuglevel > 0)
	    fprintf(lip->efp,"%s: debuglevel=%u\n",
	        lip->progname,lip->debuglevel) ;

/* figure out a program mode */

	if (pm == NULL) pm = lip->progname ;

	{
	    int	progmode = matostr(progmodes,1,pm,-1) ;
	    if (progmode < 0) progmode = progmode_username ;
	    if (sn == NULL) sn = progmodes[progmode] ;
	    lip->progmode = progmode ;
	    if (lip->debuglevel > 0)
	        fprintf(lip->efp,"%s: pm=%s(%u)\n",
		    lip->progname,progmodes[progmode],progmode) ;
	}

/* try to find a program root */

	if (lip->pr == NULL) lip->pr = pr ;

	if (lip->pr == NULL) {
	    if ((cp = getenv(VARPRNAME)) != NULL) 
		lip->pr = cp ;
	}

	if (lip->pr == NULL) {
	    if ((sfdirname(lip->progname,-1,&cp)) > 0)
		lip->pr = cp ;
	}

	if (lip->pr == NULL) lip->pr = PROGRAMROOT ;

	if (rs < 0) goto retearly ;

/* set program search-name */

	lip->sn = sn ;
	if (lip->sn == NULL) lip->sn = getenv(VARSEARCHNAME) ;
	if (lip->sn == NULL) {
	    lip->sn = lip->progname ;
	}

/* continue */

	if (f_version)
	    fprintf(lip->efp,"%s: version %s\n",
	        lip->progname,VERSION) ;

	if (lip->debuglevel > 0) {
	    fprintf(lip->efp,"%s: pr=%s\n",lip->progname,lip->pr) ;
	    fprintf(lip->efp,"%s: sn=%s\n",lip->progname,lip->sn) ;
	} /* end if */

	if (f_usage)
	    usage(lip) ;

/* help file */

	if (f_help)
	    printhelp(lip,ofname) ;

	if (f_usage || f_help || f_usage)
	    goto retearly ;

	ex = EX_OK ;

/* some preliminary initialization */

	usernamebuf[0] = '\0' ;

/* OK, we finally do our thing */

#if	CF_OUTPUT
	if ((ofname != NULL) && (ofname[0] != '\0'))
	    ofp = fopen(ofname,"w") ;
	if (ofp == NULL) {
	    rs = SR_NOTOPEN ;
	    goto badoutopen ;
	}
#endif /* CF_OUTPUT */

/* go through the loops */

	for (ai = 1 ; ai <= ai_max ; ai += 1) {

	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = procname(lip,ofp,cp) ;

	    if (rs < 0) break ;
	} /* end for (handling positional arguments) */

	if ((rs >= 0) && (pan == 0)) {

	    cp = "-" ;
	    pan += 1 ;
	    rs = procname(lip,ofp,cp) ;

	} /* end if */

#if	CF_OUTPUT
	fflush(ofp) ;
#endif

/* done */
badoutopen:
done:
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        break ;
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;
	    default:
	        ex = EX_DATAERR ;
	        break ;
	    } /* end switch */
	} /* end if */

/* finish */
retearly:
	if (lip->debuglevel > 0)
	    fprintf(lip->efp,"%s: exiting ex=%u (%d)\n",
	        lip->progname,ex,rs) ;

	fflush(lip->efp) ;

	bits_finish(&pargs) ;

badpargs:
	locinfo_finish(lip) ;

badprogstart:
badlocstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	fprintf(lip->efp,"%s: invalid argument specified (%d)\n",
	    lip->progname,rs) ;
	usage(lip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(lip)
struct locinfo	*lip ;
{
	int	rs ;
	int	wlen = 0 ;

	const char	*pn = lip->progname ;
	const char	*fmt ;


	fmt = "%s: USAGE> %s [<username>|-] \n" ;
	rs = fprintf(lip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = fprintf(lip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int printhelp(lip,ofn)
struct locinfo	*lip ;
const char	*ofn;
{
	FFILE	ofile, *ofp = &ofile ;

	const int	llen = LINEBUFLEN ;

	int	rs ;
	int	wlen = 0 ;

	char	hfname[MAXPATHLEN+1] ;


	rs = mkpath3(hfname,lip->pr,HELPDNAME,lip->sn) ;
	if (rs < 0) goto ret0 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-')) 
	    ofn = STDOUTFNAME ;

	if ((rs = ffopen(ofp,ofn,"w")) >= 0) {
	    FFILE	hfile, *hfp = &hfile ;

	    if ((rs = ffopen(hfp,hfname,"r")) >= 0) {
	        int	len ;

	        while ((rs = ffread(hfp,hfname,llen)) > 0) {
		    len = rs ;
		    rs = ffwrite(ofp,hfname,len) ;
	            if (rs < 0) break ;
	        } /* end while */

	        ffclose(hfp) ;
	    } else if (isNotPresent(rs))
	        rs = SR_OK ;

	} /* end if */

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (printhelp) */


static int locinfo_start(lip,argv)
struct locinfo	*lip ;
const char	**argv ;
{
	int	rs ;


	if (lip == NULL)
	    return SR_FAULT ;

	memset(lip,0,sizeof(struct locinfo)) ;
	lip->verboselevel = 1 ;

	rs = vecstr_start(&lip->stores,0,0) ;
	if (rs < 0) goto bad0 ;

	if ((argv != NULL) && (argv[0] != NULL)) {
	    int		cl ;
	    const char	*tp, *cp ;
	    if ((cl = sfbasename(argv[0],-1,&cp)) > 0) {
		if ((tp = strnrchr(cp,cl,'.')) != NULL) cl = (tp-cp) ;
		if (cl > 0)
		    rs = locinfo_setentry(lip,&lip->progname,cp,cl) ;
	    }
	}

	if (lip->progname == NULL) lip->progname = SEARCHNAME ;

ret0:
	return rs ;

/* bad stuff */
bad1:
	vecstr_finish(&lip->stores) ;

bad0:
	goto ret0 ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(lip)
struct locinfo	*lip ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (lip == NULL)
	    return SR_FAULT ;

	rs1 = vecstr_finish(&lip->stores) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_finish) */


int locinfo_setentry(lip,epp,vp,vl)
struct locinfo	*lip ;
const char	**epp ;
const char	vp[] ;
int		vl ;
{
	int	rs = SR_OK ;
	int	oi = -1 ;
	int	len = 0 ;


	if (lip == NULL)
	    return SR_FAULT ;

	if (epp == NULL)
	    return SR_INVALID ;

/* find existing entry for later deletion */

	if (*epp != NULL)
	    oi = vecstr_findaddr(&lip->stores,*epp) ;

/* add the new entry */

	if (vp != NULL) {
	    len = strnlen(vp,vl) ;
	    rs = vecstr_store(&lip->stores,vp,len,epp) ;
	} else if (epp != NULL)
	    *epp = NULL ;

/* delete the old entry if we had one */

	if ((rs >= 0) && (oi >= 0))
	    vecstr_del(&lip->stores,oi) ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */


static int procname(lip,ofp,name)
struct locinfo	*lip ;
FILE		*ofp ;
const char	name[] ;
{
	struct passwd	*pwp ;

	const int	dlen = USERNAMELEN ;

	int	rs = SR_OK ;
	int	wlen = 0 ;

	const char	*pp ;

	char	dbuf[USERNAMELEN+ 1] ;


	if (name == NULL) return SR_FAULT ;

	if (name[0] == '-') {
	    name = dbuf ;
	    rs = getusername(dbuf,dlen) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("procname: name=%s rs=%d \n",name,rs) ;
#endif

	if (rs >= 0)
	    pwp = getpwnam(name) ;

	if ((rs >= 0) && (pwp != NULL) && (pwp->pw_dir != NULL)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("procname: pm=%s(%u)\n",
		progmodes[lip->progmode],lip->progmode) ;
#endif

	        switch (lip->progmode) {

	        case progmode_username:
	        default:
		    pp = pwp->pw_name ;
		    break ;

	        case progmode_userhome:
	        case progmode_userdir:
	        case progmode_logdir:
		    pp = pwp->pw_dir ;
		    break ;

	        } /* end switch */

	} /* end if (have name) */

	if ((rs >= 0) && (lip->verboselevel > 0)) {
	    rs = fprintf(ofp,"%s\n",((pp != NULL) ? pp : "")) ;
	    wlen += rs ;
	} /* end if (printing) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("procname: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procname) */


/* try to get the current username (of the current process) */
static int getusername(dbuf,dlen)
char	dbuf[] ;
int	dlen ;
{
	struct passwd	*pwp ;

	uid_t	uid = getuid() ;

	int	rs = SR_NOENT ;

	const char	*usernamep, *lognamep ;


	if (dbuf == NULL) return SR_FAULT ;

	dbuf[0] = '\0' ;

	if (dlen < 0) dlen = USERNAMELEN ;

/* check the 'USERNAME' environment variable */

	if (rs == SR_NOENT) {

	    usernamep = getenv(VARUSERNAME) ;

	    if (usernamep != NULL) {

	        pwp = getpwnam(usernamep) ;

	        if (pwp != NULL) {

	            rs = SR_NOENT ;
	            if (pwp->pw_uid == uid)
	                rs = sncpy1(dbuf,dlen,usernamep) ;

	        }

	    } /* end if (had a value) */

	} /* end if (USERNAME environment) */

/* check the 'LOGNAME' environment variable */

	if (rs == SR_NOENT) {

	    lognamep = getenv(VARLOGNAME) ;

	    if (lognamep != NULL) {

	        if ((usernamep == NULL) || (strcmp(usernamep,lognamep) != 0)) {

	            pwp = getpwnam(lognamep) ;

	            if (pwp != NULL) {

	                rs = SR_NOENT ;
	                if (pwp->pw_uid == uid)
	                    rs = sncpy1(dbuf,dlen,lognamep) ;

	            }

	        } /* end if (different names) */

	    } /* end if (had a value) */

	} /* end if (LOGNAME environment) */

/* check the UTMP database */

	if (rs == SR_NOENT) {

	    dbuf[dlen] = '\0' ;
	    lognamep = getlogin() ;

	    if (lognamep != NULL) {

		sncpy1(dbuf,dlen,lognamep) ;

	        pwp = getpwnam(dbuf) ;

	        if (pwp != NULL) {

	            rs = SR_NOENT ;
	            if ((pwp->pw_uid == uid) && (dbuf[0] != '\0'))
	                rs = strlen(dbuf) ;

	        }

	    } /* end if */

	} /* end if (UTMPX database) */

/* use the PASSWD database */

	if (rs == SR_NOENT) {

	    pwp = getpwuid(uid) ;

	    if (pwp != NULL) {

	        rs = SR_NOENT ;
	        if (pwp->pw_name != NULL)
	            rs = sncpy1(dbuf,dlen,pwp->pw_name) ;

	    } else {
		int	v = uid ;
		rs = snsd(dbuf,dlen,"U",v) ;
	    }

	} /* end if (PASSWD database) */

/* we should be done! */

	return rs ;
}
/* end subroutine (getusername) */



