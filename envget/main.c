/* main */

/* front-end subroutine */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */
#define	CF_DEBUG	0		/* debug print-outs switchable */
#define	CF_UNDERSCORE	1		/* allow the underscore key */
#define	CF_ALLOK	0		/* all variables are OK */
#define	CF_ENVSORT	0		/* sort the environment */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This is the front-end for retrieving environment variables
	and outputting them in a packaged-up format for SHELL
	interpretation.

	Synopsis:

	$ envget [-tee <teefile>]


**************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<paramopt.h>
#include	<userinfo.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	6000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfkey(const char *,int,const char **) ;
extern int	vstrcmp(const char **, const char **) ;
extern int	vstrkeycmp(const char **, const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecull(const char *,int,ULONG *) ;
extern int	cfnumull(const char *,int,ULONG *) ;
extern int	optbool(const char *,int) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;
static int	isnoquote(int) ;
static int	isenvok(const char *) ;
static int	isenvan(const char *) ;


/* local variables */

static const char	*argopts[] = {
	"VERSION",
	"DEBUG",
	"HELP",
	"export",
	"env",
	"of",
	"tee",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_debug,
	argopt_help,
	argopt_export,
	argopt_env,
	argopt_of,
	argopt_tee,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ 0, 0 }
} ;

static const char	*progopts[] = {
	"export",
	NULL
} ;

enum progopts {
	progopt_export,
	progopt_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	PROGINFO	pi, *pip = &pi ;
	struct ustat	sb ;
	USERINFO	u ;
	PARAMOPT	aparams ;
	bfile		errfile, teefile ;
	bfile		outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs ;
	int 	i, sl, cl ;
	int	ex = EX_INFO ;
	int	f_optplus, f_optminus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_export = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1] ;
	char	userinfobuf[USERINFO_LEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*envfname = NULL ;
	const char	*ofname = NULL ;
	const char	*teefname = NULL ;
	const char	*sp, *cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* early things to initialize */

	pip->verboselevel = 1 ;

/* gather the arguments */

	rs = paramopt_start(&aparams) ;
	pip->open.aparams = (rs >= 0) ;

	for (ai = 0 ; ai < MAXARGGROUPS	; ai += 1)
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

	   	    rs = cfdecti((argp + 1),(argl - 1),&argvalue) ;

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

	            kwi = matostr(argopts,2,akp,akl) ;

	            if (kwi >= 0)  {

	                    switch (kwi) {

/* version */
	                    case argopt_version:
	                        f_version = TRUE ;
	                        if (f_optequal)
	                            rs = SR_INVALID ;

	                        break ;

	                    case argopt_debug:
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl, &pip->debuglevel) ;

	                        }
	                        break ;

	                    case argopt_help:
	                        f_help = TRUE ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
					f_help = (rs > 0) ;
				    }
	                        }
	                        break ;

	                    case argopt_export:
	                        f_export = TRUE ;
	                        if (f_optequal)
				    rs = SR_INVALID ;

	                        break ;

	                    case argopt_env:
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
				    break ;
				}

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            envfname = argp ;

	                        break ;

/* output file */
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

/* tee file */
	                case argopt_tee:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            teefname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            teefname = argp ;

	                    }

	                    break ;

/* handle all keyword defaults */
	                    default:
	                        rs = SR_INVALID ;
	                            bprintf(pip->efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

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
	                                if (avl)
	                                    rs = cfdeci(avp,avl,
	                                        &pip->debuglevel) ;

	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

	                        case 'e':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                envfname = argp ;

	                            break ;

/* options */
	                        case 'o':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				     }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                rs = paramopt_loads(&aparams,PO_OPTION,
	                                    argp,argl) ;

	                            break ;

				case 'p':
					f_export = TRUE ;
					break ;

/* verbosity level */
	                        case 'v':
	                            pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    rs = cfdeci(avp,avl,
	                                        &pip->verboselevel) ;

	                            }

	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
	                            break ;

	                        default:
	                            rs = SR_INVALID ;
	                                bprintf(pip->efp,
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                            break ;

	                        } /* end switch */

	                    akp += 1 ;
	                    if (rs < 0)
	                        break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        }

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (rs < 0)
		goto badarg ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u verboselevel=%d\n",
	        pip->progname, pip->debuglevel, pip->verboselevel) ;

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

/* program search name */

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,NULL) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    bprintf(pip->efp,"%s: sn=%s\n",
	        pip->progname,pip->searchname) ;

	} /* end if */

/* help file */

	if (f_usage)
	    usage(pip) ;

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_usage || f_help)
	    goto retearly ;


	ex = EX_OK ;

/* option parsing */

	{
	    PARAMOPT_CUR	c ;

	    int	spc = 0 ;


	    paramopt_curbegin(&aparams,&c) ;

	    while ((cl = paramopt_enumvalues(&aparams,PO_OPTION,&c,&cp)) >= 0) {

		if (cp == NULL) continue ;

	        i = matostr(progopts,2,cp,-1) ;

	        if (rs < 0)
	            continue ;

	        switch (i) {

	        case progopt_export:
	            f_export = TRUE ;
	            spc += 1 ;
	            break ;

	        } /* end switch */

	    } /* end while (looping through options) */

	    paramopt_curend(&aparams,&c) ;

	} /* end block (options processing) */

	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto badarg ;
	}

/* output */

	if (ofname == NULL) {
		if ((cp = getenv(VARSTDOUTFNAME)) != NULL)
			ofname = cp ;
	}

	if ((ofname == NULL) || (ofname[0] == '\0'))
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	else
	    rs = bopen(ofp,ofname,"wct",0666) ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    goto badoutopen ;
	}

/* optional "tee" file */

	if ((teefname != NULL) && (teefname[0] != '\0')) {
		rs = bopen(&teefile,teefname,"wct",0666) ;
		pip->open.teefile = (rs >= 0) ;
	}

/* do the printing */

	for (i = 0 ; envv[i] != NULL ; i += 1) {

	    if (isenvok(envv[i])) {
	        rs = bprintf(ofp,"%s\n",envv[i]) ;
		if ((rs >= 0) && pip->open.teefile) {
		   rs = bprintf(&teefile,"%s\n",envv[i]) ;
		}
	    }

	    if (rs < 0)
		break ;

	} /* end for */

	if (pip->open.teefile)
	    bclose(&teefile) ;

	bclose(ofp) ;

badoutopen:
done:
	ex = EX_OK ;
	if (rs == SR_NOENT)
	    ex = EX_NOINPUT ;

	else if (rs < 0)
	    ex = EX_DATAERR ;

retearly:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)d\n",
		pip->progname,ex,rs) ;

ret3:
	if (pip->open.aparams)
	    paramopt_finish(&aparams) ;

ret2:
	if (pip->efp != NULL)
	    bclose(pip->efp) ;

ret1:
	proginfo_finish(pip) ;

ret0:

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


/* print out (standard error) some short usage */
static int usage(pip)
PROGINFO	*pip ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [-e <envfile>] [-p] [-export] [-tee <tee>]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int isenvok(vp)
const char	*vp ;
{


#if	(! CF_UNDERSCORE)
	if (*vp == '_')
	    return FALSE ;
#endif

	while (*vp) {

	    if ((! isprint(*vp)) || (*vp == '\"'))
	        return FALSE ;

	    vp += 1 ;

	} /* end while */

	return TRUE ;
}
/* end subroutine (isenvok) */


static int isenvan(vp)
const char	*vp ;
{
	int	f ;


	while (*vp) {

	    f = isalnum(*vp) || isnoquote(*vp) ;
	    if (! f)
	        return FALSE ;

	    vp += 1 ;

	} /* end while */

	return TRUE ;
}
/* end subroutine (isenvan) */


static int isnoquote(ch)
int	ch ;
{
	int	rs = FALSE ;


	switch (ch) {

	case ':':
	case ';':
	case '/':
	case '%':
	case '?':
	case '_':
	case '+':
	case '-':
	case ',':
	case '.':
	case '=':
	case '~':
	    rs = TRUE ;
	    break ;

	} /* end switch */

	return rs ;
}
/* end subroutine (isnoquote) */



