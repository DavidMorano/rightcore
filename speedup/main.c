/* main */

/* front-end for whatever */


#define	CF_DEBUGS	0		/* non-switchables */
#define	CF_DEBUG	0		/* debug print-outs */
#define	CF_LOCSETENT	0		/* |locinfo_setentry()| */


/* revision history:

	= 1996-02-01, David A­D­ Morano

	The program was written from scratch to do what the previous
	program by the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a fairly generic front-end subroutine for a program.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<keyopt.h>
#include	<vecobj.h>
#include	<baops.h>
#include	<field.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	NDEFS
#define	NDEFS		10
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern double	fam(double *,int) ;
extern double	fhm(double *,int) ;
extern double	fsum(double *,int) ;

extern int	sfshrink(cchar *,int,cchar **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecf(const char *,int, double *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strshrink(char *) ;
extern char	*strnchr(const char *,int,int) ;


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		sum:1 ;
	uint		amean:1 ;
	uint		hmean:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	struct proginfo	*pip ;
	const char	*pr_pcs ;
	const char	*homedname ;
	uint		sum ;
} ;

struct processor {
	VECOBJ		numbers ;
	double		*fa ;
	uint		open:1 ;
} ;


/* forward references */

static int	usage(struct proginfo *) ;

static int	locinfo_start(struct locinfo *,struct proginfo *) ;
static int	locinfo_finish(struct locinfo *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(struct locinfo *,const char **,
			const char *,int) ;
#endif /* CF_LOCSETENT */

static int	processor_start(struct processor *,int) ;
static int	processor_finish(struct processor *) ;
static int	processor_add(struct processor *,const char *,int) ;
static int	processor_result(struct processor *,int,double *) ;


/* external variables */


/* local variables */

static const char	*progmodes[] = {
	"asum",
	"amean",
	"hmean",
	NULL
} ;

enum progmodes {
	progmode_asum,
	progmode_amean,
	progmode_hmean,
	progmode_overlast
} ;

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"option",
	"pm",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_option,
	argopt_pm,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
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
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static const char	*progopts[] = {
	"type",
	"sum",
	"asum",
	"amean",
	"hmean",
	NULL
} ;

enum progopts {
	progopt_type,
	progopt_sum,
	progopt_asum,
	progopt_amean,
	progopt_hmean,
	progopt_overlast
} ;

static const char	*whiches[] = {
	"sum",
	"amean",
	"hmean",
	NULL
} ;

enum whiches {
	which_sum,
	which_amean,
	which_hmean,
	which_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;
	struct locinfo	li, *lip = &li ;
	struct processor	nproc ;
	KEYOPT		akopts ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan = 0 ;
	int	rs, rs1 ;
	int	ki, wi ;
	int	cl ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS] ;
	const char	*pmspec = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFD1)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_rest: starting DFD=%d\n",rs) ;
	}
#endif

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

	pip->lip = &li ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* early things to initialize */

	pip->verboselevel = 1 ;

/* process program arguments */

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

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

	        if (isdigit(argp[1])) {

	                argval = (argp + 1) ;

	        } else if (argp[1] == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	                f_optequal = TRUE ;

	            } else {

	                avl = 0 ;
	                akl = aol ;

	            }

	            if ((kwi = matostr(argopts,2,aop,aol)) >= 0) {

	                switch (kwi) {

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

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pr = argp ;

	                    }

	                    break ;

/* program mode */
	                case argopt_pm:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            pmspec = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pmspec = argp ;

	                    }

	                    break ;

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;

	                    break ;

/* verbose */
	                case argopt_verbose:
	                    pip->verboselevel = 2 ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl > 0) {

	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

	                        }
	                    }

	                    break ;

/* temporary directory */
	                case argopt_tmpdir:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->tmpdname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pip->tmpdname = argp ;

	                    }

	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* the user specified some progopts */
	                case argopt_option:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }

	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl)
	                        rs = keyopt_loads(&akopts,argp,argl) ;

	                    break ;

/* search name */
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

/* error file */
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

/* output file */
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

/* intput file */
	                case argopt_if:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ifname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            ifname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                default:
	                    rs = SR_INVALID ;
			    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *aop) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
				    if (avl) {
	                                rs = optvalue(avp,avl) ;
				        pip->debuglevel = rs ;
				    }
	                        }
	                        break ;

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'o':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            rs = keyopt_loads(&akopts,argp,argl) ;
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* verbose output */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
				        pip->verboselevel = rs ;
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

	        } /* end if (digits or progopts) */

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

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
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

	if (f_help) {
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
	}

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* get our program mode */

	if (pmspec == NULL)
	    pmspec = pip->progname ;

	pip->progmode = matstr(progmodes,pmspec,-1) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0)
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	        else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif

	switch (pip->progmode) {
	case progmode_asum:
	    proginfo_setbanner(pip,BANNER_ASUM) ;
	    break ;
	case progmode_amean:
	    proginfo_setbanner(pip,BANNER_AMEAN) ;
	    break ;
	case progmode_hmean:
	    proginfo_setbanner(pip,BANNER_HMEAN) ;
	    break ;
	default:
	    pip->progmode = progmode_asum ;
	    break ;
	} /* end switch */

/* check a few more things */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* get some progopts */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    KEYOPT_CUR	cur ;
	    debugprintf("main: progopts specified:\n") ;
	    keyopt_curbegin(&akopts,&cur) ;
	    while ((rs = keyopt_enumkeys(&akopts,&cur,&cp)) >= 0) {
	        if (cp == NULL) continue ;
	        debugprintf("main: | optkey=%s\n",cp) ;
	    }
	    keyopt_curend(&akopts,&cur) ;
	}
#endif /* CF_DEBUG */

	for (ki = 0 ; progopts[ki] != NULL ; ki += 1) {
	    KEYOPT_CUR	cur ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: optkey=%s\n",progopts[ki]) ;
#endif

	    keyopt_curbegin(&akopts,&cur) ;

	    while (rs >= 0) {

	        rs1 = keyopt_enumvalues(&akopts,progopts[ki],&cur,&cp) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            debugprintf("main: keyopt_enumvalues() rs=%d\n",rs1) ;
	            debugprintf("main: cp=%s\n",cp) ;
	        }
#endif

	        cl = rs1 ;
	        if (rs1 < 0)
	            break ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("main: value=%s\n",cp) ;
#endif

	        switch (ki) {
	        case progopt_type:
	            if (cl > 0) {
	                wi = matostr(whiches,2,cp,cl) ;
	                switch (wi) {
	                case which_sum:
	                    lip->f.sum = TRUE ;
	                    break ;
	                case which_amean:
	                    lip->f.amean = TRUE ;
	                    break ;
	                case which_hmean:
	                    lip->f.hmean = TRUE ;
	                    break ;
	                } /* end switch */
	            } /* end if (non-zero value) */
	            break ;
	        case progopt_sum:
	        case progopt_asum:
	            lip->f.sum = TRUE ;
	            break ;
	        case progopt_amean:
	            lip->f.amean = TRUE ;
	            break ;
	        case progopt_hmean:
	            lip->f.hmean = TRUE ;
	            break ;
	        } /* end switch */

	    } /* end while (enumerating) */

	    keyopt_curend(&akopts,&cur) ;

	} /* end for (progopts) */

/* if we don't have a request for something yet, use our progmode */

	f = lip->f.sum || lip->f.amean || lip->f.hmean ;
	if (! f) {
	    switch (pip->progmode) {
	    case progmode_amean:
	        lip->f.amean = TRUE ;
	        break ;
	    case progmode_hmean:
	        lip->f.hmean = TRUE ;
	        break ;
	    case progmode_asum:
	    default:
	        lip->f.sum = TRUE ;
	        break ;
	    } /* end switch */
	} /* end if (didn't get anuthing yet) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: requests sum=%u amean=%u, hmean=%u\n",
	        lip->f.sum,lip->f.amean,lip->f.hmean) ;
#endif

	if (pip->debuglevel > 0) {

	    if (lip->f.sum)
	        bprintf(pip->efp,"%s: request> %s\n",
	            pip->progname,whiches[which_sum]) ;

	    if (lip->f.amean)
	        bprintf(pip->efp,"%s: request> %s\n",
	            pip->progname,whiches[which_amean]) ;

	    if (lip->f.hmean)
	        bprintf(pip->efp,"%s: request> %s\n",
	            pip->progname,whiches[which_hmean]) ;

	}

/* open the output */

	if ((ofname != NULL) && 
	    (ofname[0] != '\0') && (strcmp(ofname,STDOUTFNAME) != 0)) {
	    rs = bopen(ofp,ofname,"wct",0644) ;

	} else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0644) ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: could not open output (%d)\n",
	        pip->progname,rs) ;
	    goto badoutopen ;
	}

	pip->ofp = ofp ;

/* OK, we do it */

	if ((rs = processor_start(&nproc,NDEFS)) >= 0) {

	    for (ai = 1 ; ai < argc ; ai += 1) {

	        f = (ai <= ai_max) && BATST(argpresent,ai) ;
	        f = f || (ai > ai_pos) ;
	        if (! f) continue ;

		cp = argv[ai] ;
	        pan += 1 ;
	        rs = processor_add(&nproc,cp,-1) ;

	        if (rs < 0) {
	            bprintf(pip->efp,
	                "%s: processing error (%d) with value=>%s<\n",
	                pip->progname,rs,argv[ai]) ;
	            break ;
	        }

	    } /* end for (looping through requested circuits) */

/* process any files in the argument filename list file */

	    if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	        bfile	argfile ;

	        if (strcmp(afname,"-") != 0) {
	            rs = bopen(&argfile,afname,"r",0666) ;
	        } else
	            rs = bopen(&argfile,BFILE_STDIN,"dr",0666) ;

	        if (rs >= 0) {
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = breadline(&argfile,lbuf,LINEBUFLEN)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                cl = sfshrink(lbuf,len,&cp) ;

	                if ((cl <= 0) || (cp[0] == '#'))
	                    continue ;

	                pan += 1 ;
	                rs = processor_add(&nproc,cp,cl) ;

	                if (rs < 0) {
	                    bprintf(pip->efp,
	                        "%s: processing error (%d) with value=>%t<\n",
	                        pip->progname,rs,cp,cl) ;
	                    break ;
	                }

	            } /* end while (reading lines) */

	            bclose(&argfile) ;
	        } else {
	            if (! pip->f.quiet) {
	                bprintf(pip->efp,
	                    "%s: could not open the argument list file (%d)\n",
	                    pip->progname,rs) ;
	                bprintf(pip->efp,"%s: \targfile=%s\n",
	                    pip->progname,afname) ;
	            }
	        }

	    } /* end if (processing file argument file list) */

	    if ((rs >= 0) && (pan == 0)) {
	        bfile	infile ;

	        if ((ifname != NULL) && (ifname[0] != '\0') &&
	            (strcmp(ifname,STDINFNAME) != 0)) {
	            rs = bopen(&infile,ifname,"r",0666) ;
	        } else
	            rs = bopen(&infile,BFILE_STDIN,"dr",0666) ;

	        if (rs >= 0) {
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = breadline(&infile,lbuf,LINEBUFLEN)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                cl = sfshrink(lbuf,len,&cp) ;

	                if ((cl <= 0) || (cp[0] == '#'))
	                    continue ;

	                pan += 1 ;
	                rs = processor_add(&nproc,cp,cl) ;

	                if (rs < 0) {

	                    bprintf(pip->efp,
	                        "%s: processing error (%d) with value=>%t<\n",
	                        pip->progname,rs,cp,cl) ;

	                    break ;
	                }

	            } /* end while (reading lines) */

	            bclose(&infile) ;
	        } else {
	            if (! pip->f.quiet) {
	                bprintf(pip->efp,
	                    "%s: could not open the input file (%d)\n",
	                    pip->progname,rs) ;
	                bprintf(pip->efp,"%s: \targfile=%s\n",
	                    pip->progname,afname) ;
	            }
	        }

	    } /* end if (processing STDIN) */

/* print out the results */

	    if (rs >= 0) {
	        double	fnum ;
	        int	wi ;

	        if (lip->f.sum) {
	            wi = which_sum ;
	            if ((rs = processor_result(&nproc,wi,&fnum)) >= 0) {
	                bprintf(ofp,"%14.4f\n",fnum) ;
		    }
	        }

	        if (lip->f.amean) {
	            wi = which_amean ;
	            if ((rs = processor_result(&nproc,wi,&fnum)) >= 0) {
	                bprintf(ofp,"%14.4f\n",fnum) ;
		    }
	        }

	        if (lip->f.hmean) {
	            wi = which_hmean ;
	            if ((rs = processor_result(&nproc,wi,&fnum)) >= 0) {
	                bprintf(ofp,"%14.4f\n",fnum) ;
		    }
	        }

	    } /* end if (outputting results) */

	    processor_finish(&nproc) ;
	} /* end if (processor) */

	pip->ofp = NULL ;
	bclose(ofp) ;

#ifdef	COMMENT
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: files=%u processed=%u\n",
	        pip->progname,pip->c_files,pip->c_processed) ;
#endif

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

/* we are out of here */
badoutopen:
done:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

retearly:
	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

badpargs:
	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* program usage */
usage:
	usage(pip) ;
	goto retearly ;

badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
struct proginfo	*pip ;
{
	int		rs ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<value(s)> ...] [-o <calculation>] [-Vv]\n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-af {<afile>|-}]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int processor_start(op,n)
struct processor	*op ;
int			n ;
{
	int		rs ;
	int		size ;
	int		opts ;

	if (n < 3) n = 3 ;

	memset(op,0,sizeof(struct processor)) ;

	size = sizeof(double) ;
	opts = VECOBJ_OCOMPACT ;
	rs = vecobj_start(&op->numbers,size,n,opts) ;

#if	CF_DEBUGS
	debugprintf("main/processor_start: vecobj_start() rs=%d\n",rs) ;
#endif

	op->open = (rs >= 0) ;
	return rs ;
}

static int processor_finish(op)
struct processor	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (! op->open)
	    return SR_NOTOPEN ;

	if (op->fa != NULL) {
	    rs1 = uc_free(op->fa) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fa = NULL ;
	}

	rs1 = vecobj_finish(&op->numbers) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}

static int processor_add(op,s,slen)
struct processor	*op ;
const char		s[] ;
int			slen ;
{
	double		fnum ;
	int		rs = SR_OK ;
	int		sl, cl ;
	const char	*sp ;
	const char	*tp, *cp ;

	if (! op->open)
	    return SR_NOTOPEN ;

	sp = s ;
	sl = (slen >= 0) ? slen : strlen(s) ;

	if ((tp = strnchr(sp,sl,'#')) != NULL)
		sl = (tp - sp) ;

#if	CF_DEBUGS
	debugprintf("main/processor_add: line=>%t<\n",sp,sl) ;
#endif

	while ((cl = nextfield(sp,sl,&cp)) > 0) {

#if	CF_DEBUGS
	    debugprintf("main/processor_add: str=>%t<\n",cp,cl) ;
#endif

	    rs = cfdecf(cp,cl,&fnum) ;

#if	CF_DEBUGS
	    debugprintf("main/processor_add: cfdecf() rs=%d\n",rs) ;
#endif

	    if (rs >= 0)
	        rs = vecobj_add(&op->numbers,&fnum) ;

	    if (rs < 0)
	        break ;

	    sl -= (cp + cl - sp) ;
	    sp = (cp + cl) ;

	} /* end while */

	return rs ;
}

static int processor_result(op,which,rp)
struct processor	*op ;
int			which ;
double			*rp ;
{
	int		rs = SR_OK ;
	int		n ;

	if (! op->open)
	    return SR_NOTOPEN ;

	if (rp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	{
	    int	i ;
	    double	*fnp ;
	    rs = vecobj_count(&op->numbers) ;
	    n = rs ;
	    debugprintf("main/processor_result: n=%u\n",n) ;
	    for (i = 0 ; vecobj_get(&op->numbers,i,&fnp) >= 0 ; i += 1) {
	        debugprintf("main/processor_result: num[%u]=%14.4f\n",
	            i,*fnp) ;
	    }
	}
#endif /* CF_DEBUG */

	if (op->fa == NULL) {
	    double	*fnp ;
	    int		size, i, j ;

	    rs = vecobj_count(&op->numbers) ;
	    n = rs ;

	    size = (n + 1) * sizeof(double) ;
	    if (rs >= 0)
	        rs = uc_malloc(size,&op->fa) ;

	    if (rs >= 0) {

	        j = 0 ;
	        for (i = 0 ; vecobj_get(&op->numbers,i,&fnp) >= 0 ; i += 1) {

	            if (fnp == NULL) continue ;

	            op->fa[j++] = *fnp ;

	        } /* end for */

	        n = j ;

	    } /* end if (populating) */

	} /* end if (creating array) */

	if (rs >= 0) {
	    switch (which) {
	    case which_sum:
	        *rp = fsum(op->fa,n) ;
	        break ;
	    case which_amean:
	        *rp = fam(op->fa,n) ;
	        break ;
	    case which_hmean:
	        *rp = fhm(op->fa,n) ;
	        break ;
	    } /* end switch */
	} /* end if */

	return rs ;
}
/* end subroutine (processor_result) */


static int locinfo_start(lip,pip)
struct locinfo	*lip ;
struct proginfo	*pip ;
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(struct locinfo)) ;
	lip->pip = pip ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(lip)
struct locinfo	*lip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


#if	CF_LOCSETENT
int locinfo_setentry(lip,epp,vp,vl)
struct locinfo	*lip ;
const char	**epp ;
const char	vp[] ;
int		vl ;
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (! lip->open.stores) {
	    rs = vecstr_start(&lip->stores,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;

	    if (*epp != NULL) oi = vecstr_findaddr(&lip->stores,*epp) ;

	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(&lip->stores,vp,len,epp) ;
	    } else
	        *epp = NULL ;

	    if ((rs >= 0) && (oi >= 0))
	        vecstr_del(&lip->stores,oi) ;

	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */
#endif /* CF_LOCSETENT */


