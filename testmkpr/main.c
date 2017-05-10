/* main */

/* front-end for the TESTMKPR program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	1		/* run-time debug print-outs */


/* revision history:

	= 2008-01-16, David A­D­ Morano

	Of course this was taken from previous programs.  I deleted the list
	that was here and replaced it with this simple admission. :-)


*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ testmkpr.x [<prname> [<dname>]]


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<netdb.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<dater.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern int	sfshrink(const char *,int,char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	getrootdname(char *,int,const char *,const char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;


/* define command option words */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	bfile		errfile ;

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		pan = 0 ;
	int		rs ;
	int		rs1 ;
	int		len ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;
	int		f_usage = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*prname = NULL ;
	const char	*dname = NULL ;
	const char	*cp ;
	char		argpresent[MAXARGGROUPS] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;

/* start parsing the arguments */

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1)
	    argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
		const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

		    argval = (argp + 1) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {
	                f_optequal = TRUE ;
	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	            } else {
			avp = NULL ;
	                avl = 0 ;
	                akl = aol ;
	            }

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

			case argopt_root:
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;

	                    break ;

/* verbose mode */
	                case argopt_verbose:
	                    pip->verboselevel = 2 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;
				}
	                    }
	                    break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                sn = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* argument-list */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                afname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                efname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* output-file */
	                case argopt_of:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ofname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch */

	            } else {

	                while (akl--) {
			    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

/* debug */
	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdeci(avp,avl,
	                                    &pip->debuglevel) ;
				    }
	                        }
	                        break ;

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

			case 'R':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* output file name */
	                    case 'o':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            ofname = argp ;
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;
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

	        } /* end if (digits or options) */

	    } else {

	        if (ai >= MAXARGINDEX) break ;
	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

/* help file */

	if (f_usage)
	    usage(pip) ;

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_usage || f_help)
	    goto retearly ;


	ex = EX_OK ;

/* get the first positional argument as the mailbox name */

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (f) {
	        cp = argv[ai] ;
	        switch (pan) {
	        case 0:
		    prname = cp ;
		    break ;
	        case 1:
		    dname = cp ;
		    break ;
	        } /* end switch */
	    }

	    pan += 1 ;
	} /* end for */

/* verify some parameters and arguments */

	if (prname == NULL) prname = VARPRLOCAL ;
	if (dname == NULL) dname = pip->domainname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("main: prname=%s dname=%s\n",prname,dname) ;
#endif

	if (ofname == NULL) ofname = BFILE_STDOUT ;

	if (rs >= 0) {
	    bfile	ofile, *ofp = &ofile ;
	    if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0) {
	        const int	plen = MAXPATHLEN ;
	        char		pbuf[MAXPATHLEN+1] ;

	        rs1 = mkpr(pbuf,plen,prname,dname) ;
	        len = rs1 ;

	        bprintf(ofp,"rs=%d pr=%t\n",rs1,pbuf,len) ;

	        bclose(ofp) ;
	    } else {
	        ex = EX_CANTCREAT ;
	    }
	} /* end if (ok) */

/* done */
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

/* early return thing */
retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

	if (pip->efp != NULL) {
	    pip->open.errfile = TRUE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [mailbox] \n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-?V] [-Dv]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


