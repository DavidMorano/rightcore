/* main */

/* test template */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocations */
#define	CF_MID		1		/* do new MID print-out */


/* revision history:

	= 2003-03-01, David A­D­ Morano
	This subroutine was originally written to be the front-end for the new
	MSGIDADM program.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ msgidadm <name(s)>


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<field.h>
#include	<getxusername.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"msgid.h"


/* local defines */

#define	SORTKEYLEN	10

#define	NCOLSRECIP	8
#define	NCOLSFROM	50
#define	NCOLSMSGID	78

#define	O_FLAGS		O_RDONLY

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	sncpylc(char *,int,cchar *) ;
extern int	sncpyuc(char *,int,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mkfnamesuf1(char *,cchar *,cchar *) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	nextfield(cchar *,int,cchar **) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	vecstr_adduniq(vecstr *,cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isFailOpen(int) ;

extern int	printhelp(bfile *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strcpylc(char *,cchar *) ;
extern char	*strcpyuc(char *,cchar *) ;
extern char	*strwcpylc(char *,cchar *,int) ;
extern char	*strwcpyuc(char *,cchar *,int) ;
extern char	*strwcpy(char *,cchar *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */

struct locinfo_flags {
	uint		all:1 ;
	uint		mid:1 ;
	uint		header:1 ;
} ;

struct locinfo {
	int		(*cmpfunc)() ;
	LOCINFO_FL	f ;
	int		nentries ;
	int		ski ;
	int		tdi ;
	char		username[USERNAMELEN + 1] ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procargs(PROGINFO *,ARGINFO *,BITS *,vecstr *,cchar *) ;
static int	procout(PROGINFO *,vecstr *,cchar *,cchar *) ;

static int	loadname(PROGINFO *,vecstr *,cchar *,int) ;
static int	loadnames(PROGINFO *,VECSTR *,cchar *,int) ;

static int	process(PROGINFO *,cchar *,bfile *,vecstr *) ;
static int	cmp_utime(), cmp_ctime(), cmp_mtime() ;

static int	cmpr_utime(), cmpr_ctime(), cmpr_mtime() ;


/* local variables */

static cchar *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"db",
	"td",
	"nh",
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
	argopt_db,
	argopt_td,
	argopt_nh,
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

static cchar *sortkeys[] = {
	"utime",
	"mtime",
	"ctime",
	"update",
	"msg",
	"create",
	"none",
	NULL
} ;

enum sortkeys {
	sortkey_utime,
	sortkey_mtime,
	sortkey_ctime,
	sortkey_utime2,
	sortkey_mtime2,
	sortkey_ctime2,
	sortkey_none,
	sortkey_overlast
} ;

static const uchar	aterms[] = {
	0x00, 0x2E, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		v, i  ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f_reverse = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	char		dbfname[MAXPATHLEN + 1] ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*sortkeyspec = NULL ;
	cchar		*tdspec = NULL ;
	cchar		*dbname = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;

	dbfname[0] = '\0' ;

	pip->lip = lip ;
	memset(lip,0,sizeof(LOCINFO)) ;

	lip->f.header = TRUE ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

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

/* program root */
	                case argopt_root:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pr = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
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
	                            rs = optvalue(avp,avl) ;
	                            pip->verboselevel = rs ;
	                        }
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* database name (path) */
	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            dbname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                dbname = argp ;
	                        } else
	                            rs = SR_INVALID ;
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

/* argument file name */
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

/* output file name */
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

	                case argopt_td:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            tdspec = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

	                case argopt_nh:
	                    lip->f.header = FALSE ;
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
	                                rs = optvalue(avp,avl) ;
	                                pip->debuglevel = rs ;
	                            }
	                        }
	                        break ;

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'a':
	                        lip->f.all = TRUE ;
	                        break ;

	                    case 'f':
	                        lip->f.mid = FALSE ;
	                        break ;

	                    case 'h':
	                        lip->f.header = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.header = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'm':
	                        lip->f.mid = TRUE ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                KEYOPT	*kop = &akopts ;
	                                rs = keyopt_loads(kop,argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

	                    case 'r':
	                        f_reverse = TRUE ;
	                        break ;

	                    case 's':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                sortkeyspec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* verbose mode */
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

	        } /* end if (digits as argument or not) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
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
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
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

/* get the program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* other initialization */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* apply some defaults */

	if ((dbname == NULL) || (dbname[0] == '\0'))
	    dbname = getenv(VARMSGIDDB) ;

	if ((dbname == NULL) || (dbname[0] == '\0'))
	    dbname = MSGIDDB ;

	mkpath3(dbfname,pip->pr,MSGIDDNAME,dbname) ;

	if ((cp = getenv(VARUSERNAME)) != NULL) {
	    strwcpy(lip->username,cp,USERNAMELEN) ;
	}

/* check arguments */

	lip->nentries = DEFENTS ;
	if (rs >= 0) {
	    if (! lip->f.all) {
	        if (argval != NULL) {
	            rs = cfdeci(argval,-1,&v) ;
	            if (v > 0) lip->nentries = v ;
	        }
	    } else
	        lip->nentries = INT_MAX ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: nentries=%u\n",
	        pip->progname,lip->nentries) ;
	}

/* sort key */

	lip->ski = sortkey_utime ;
	if (sortkeyspec != NULL) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: sortkeyspec=%s f_rev=%u\n",
	            sortkeyspec,f_reverse) ;
#endif
	    i = matostr(sortkeys,1,sortkeyspec,-1) ;
	    if (i >= 0) lip->ski = i ;
	}

	switch (lip->ski) {
	case sortkey_none:
	    break ;
	default:
	case sortkey_utime:
	case sortkey_utime2:
	    lip->ski = sortkey_utime ;
	    lip->cmpfunc = (f_reverse) ? cmpr_utime : cmp_utime ;
	    break ;
	case sortkey_mtime:
	case sortkey_mtime2:
	    lip->ski = sortkey_mtime ;
	    lip->cmpfunc = (f_reverse) ? cmpr_mtime : cmp_mtime ;
	    break ;
	case sortkey_ctime:
	case sortkey_ctime2:
	    lip->ski = sortkey_ctime ;
	    lip->cmpfunc = (f_reverse) ? cmpr_ctime : cmp_ctime ;
	    break ;
	} /* end switch */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: sortkey=%s(%u)\n",
	        sortkeys[lip->ski],lip->ski) ;
#endif

/* time display (TD) */

	lip->tdi = sortkey_mtime ;
	if (tdspec != NULL) {
	    if ((i = matostr(sortkeys,1,tdspec,-1)) >= 0) {
	        lip->tdi = i ;
	        switch (lip->tdi) {
	        case sortkey_utime2:
	            lip->tdi = sortkey_utime ;
	            break ;
	        case sortkey_mtime2:
	            lip->tdi = sortkey_mtime ;
	            break ;
	        case sortkey_ctime2:
	            lip->tdi = sortkey_ctime ;
	            break ;
	        } /* end switch */
	    } /* end if (matostr) */
	} /* end if */

/* go */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    vecstr	recips ;
	    if ((rs = vecstr_start(&recips,10,0)) >= 0) {
		ARGINFO	*aip = &ainfo ;
		BITS	*bop = &pargs ;
		if ((rs = procargs(pip,aip,bop,&recips,afname)) >= 0) {
		    rs = procout(pip,&recips,dbfname,ofname) ;
		}
	        rs1 = vecstr_finish(&recips) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (vecstr) */
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,"%s: invalid query (%d)\n",
	                pip->progname,rs) ;
	        }
	        break ;
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;
	    case SR_AGAIN:
	        ex = EX_TEMPFAIL ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} /* end if */

/* we are done */
retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    keyopt_finish(&akopts) ;
	    pip->open.akopts = FALSE ;
	}

	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
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

	fmt = "%s: USAGE> %s [<recipient(s)> ...] [-a] [-s <sortkey>] [-m]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-db <dbname>] [-td <displaykey>] [-r] -h[=<b>]] [-nh]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=n]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,
		vecstr *rlp,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		pan = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	cchar		*cp ;

	if (rs >= 0) {
	    int		ai ;
	    int		f ;
	    cchar	**argv = aip->argv ;
	    for (ai = 1 ; ai < aip->argc ; ai += 1) {

	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	    	    cp = argv[ai] ;
		    if (cp[0] != '\0') {
	    		pan += 1 ;
	    		rs = loadname(pip,rlp,cp,-1) ;
		    }
		}

	        if (rs < 0) break ;
	     } /* end for */
	} /* end if (ok) */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	    if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	          	    pan += 1 ;
			    rs = loadnames(pip,rlp,cp,cl) ;
			}
		    }

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = bclose(afp) ;
		if (rs >= 0) rs = rs1 ;
	    } else {
	        if (! pip->f.quiet) {
		    fmt = "%s: inaccessible argument-list (%d)\n" ;
	            bprintf(pip->efp,fmt,pn,rs) ;
	            bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (procargs) */


static int procout(PROGINFO *pip,vecstr *rlp,cchar *dfn,cchar *ofn)
{
	bfile		ofile, *ofp = &ofile ;
	int		rs = SR_OK ;
	int		rs1 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",0666)) >= 0) {

	    rs = process(pip,dfn,ofp,rlp) ;

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    bprintf(pip->efp,"%s: unavailable output (%d)\n",
	        pip->progname,rs) ;
	}

	return rs ;
}
/* end subroutine (procout) */


static int process(PROGINFO *pip,cchar *dbfname,bfile *ofp,vecstr *rlp)
{
	LOCINFO		*lip = pip->lip ;
	MSGID		db ;
	MSGID_CUR	cur ;
	MSGID_ENT	e, *ep ;
	vecobj		entries ;
	time_t		t ;
	int		rs, rs1 ;
	int		oflags ;
	int		policy ;
	int		n, tc ;
	int		nentries ;
	int		ninsert ;
	int		i, cl ;
	int		chend ;
	int		c = 0 ;
	int		f_done ;
	int		f_recip = FALSE ;
	int		f ;
	cchar		*cp ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		timebuf[TIMEBUFLEN + 1] ;


	nentries = lip->nentries ;
	ninsert = (lip->f.all) ? -1 : lip->nentries ;
	f_recip = FALSE ;
	if (! lip->f.all) {

	    rs1 = vecstr_count(rlp) ;
	    f_recip = (rs1 > 0) ? 1 : 0 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("msgidadm/process: recips=%d\n",rs1) ;
#endif

	} /* end if (not all specified) */

/* prepare for the entries */

	n = MIN(nentries,200) ;
	policy = VECOBJ_PORDERED ;
	if (lip->ski != sortkey_none)
	    policy = VECOBJ_PSORTED ;

	rs = vecobj_start(&entries,sizeof(MSGID_ENT),n,policy) ;
	if (rs < 0)
	    goto ret0 ;

/* open the database */

	oflags = O_FLAGS ;
	f = FALSE ;
	f = f || ((oflags & O_RDWR) == O_RDWR) ;
	rs = msgid_open(&db,dbfname,oflags,0666,0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: 1 msgid_open() rs=%d\n",rs) ;
#endif

	if ((rs == SR_ACCESS) && (! f))
	    rs = msgid_open(&db,dbfname,O_RDONLY,0666,4) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    struct ustat	sb ;
	    debugprintf("main: 2 msgid_open() rs=%d\n",rs) ;
	    mkfnamesuf1(tmpfname,dbfname,"msgid") ;
	    u_stat(tmpfname,&sb) ;
	    debugprintf("main: fsize=%lu\n",sb.st_size) ;
	}
#endif /* CF_DEBUG */

	if (rs < 0)
	    goto ret1 ;

/* list all entries */

	f_done = FALSE ;
	for (tc = 0 ; (! f_done) && (tc < 20) ; tc += 1) {

#ifdef	COMMENT
	    rs = msgid_txbegin(&db) ;
	    txid = rs ;
#endif /* COMMENT */

	    msgid_curbegin(&db,&cur) ;

	    while (msgid_enum(&db,&cur,&e) >= 0) {

	        if (e.recipient[0] != '\0') {
	            f = TRUE ;
	            if (f_recip) {
	                rs1 = vecstr_find(rlp,e.recipient) ;
	                f = (rs1 >= 0) ? 1 : 0 ;
	            }

	            if (f) {

	                if (lip->ski != sortkey_none) {

	                    rs1 = vecobj_inorder(&entries,&e,
	                        lip->cmpfunc,ninsert) ;

	                    if ((rs1 >= 0) && (! lip->f.all))
	                        vecobj_del(&entries,nentries) ;

	                } else
	                    rs1 = vecobj_add(&entries,&e) ;

	            } /* end if */

	        } /* end if */

	        if (rs < 0) break ;
	    } /* end while */

	    msgid_curend(&db,&cur) ;

#ifdef	COMMENT
	    rs = msgid_txcommit(&db,txid) ;
	    f_done = (rs >= 0) ;
#else
	    f_done = TRUE ;
#endif /* COMMENT */

	    if (! f_done) {
	        for (i = 0 ; vecobj_get(&entries,i,&ep) >= 0 ; i += 1) {
	            if (ep != NULL) {
	                vecobj_del(&entries,i--) ;
		    }
	        } /* end for */
	    } /* end if */

	} /* end for */

	msgid_close(&db) ;

/* print out the extracted entries */

	if ((rs >= 0) && lip->f.header) {

	    strwcpyuc(tmpfname,sortkeys[lip->tdi],SORTKEYLEN) ;

#if	CF_MID
	    bprintf(ofp,
	        "RECIP    %-14s COUNT %s\n",
	        tmpfname, "FROM") ;
#else /* CF_MID */
	    bprintf(ofp,
	        "RECIP    %-14s COUNT %s\n",
	        tmpfname,
	        ((lip->f.mid) ? "MESSAGE-ID" : "FROM")) ;
#endif /* CF_MID */

	} /* end if */

	for (i = 0 ; (i < nentries) && (vecobj_get(&entries,i,&ep) >= 0) ; 
	    i += 1) {

	    if (ep == NULL) continue ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        char	timebuf1[TIMEBUFLEN + 1] ;
	        char	timebuf2[TIMEBUFLEN + 1] ;
	        char	timebuf3[TIMEBUFLEN + 1] ;
	        debugprintf("main: tdi=%u ut=%s mt=%s ct=%s\n",
	            lip->tdi,
	            timestr_log(ep->utime,timebuf1),
	            timestr_log(ep->mtime,timebuf2),
	            timestr_log(ep->ctime,timebuf3)) ;
	    }
#endif /* CF_DEBUG */

	    switch (lip->tdi) {
	    case sortkey_utime:
	        t = ep->utime ;
	        break ;
	    case sortkey_ctime:
	        t = ep->ctime ;
	        break ;
	    case sortkey_mtime:
	        t = ep->mtime ;
	        break ;
	    } /* end switch */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: t=%s\n",
	            timestr_log(t,timebuf)) ;
#endif

	    n = NCOLSFROM ;
	    chend = ' ' ;
	    cp = ep->from ;
	    cl = strlen(cp) ;

	    if (cl == n) {
	        cl -= 1 ;
	        chend = cp[cl] ;
	    } else if (cl > n) {
	        cl = (n - 1) ;
	        chend = 0xAC ;
	    } /* end if */

	    c += 1 ;
	    {
	        char	recipbuf[NCOLSRECIP + 1] ;

	        strwcpy(recipbuf,ep->recipient,NCOLSRECIP) ;

	        rs = bprintf(ofp,"%-*s %-14s %5u %t%c\n",
	            NCOLSRECIP,recipbuf,
	            timestr_log(t,timebuf),
	            ep->count,
	            cp,cl,chend) ;

	    } /* end block */

	    if ((rs >= 0) && lip->f.mid) {

	        n = NCOLSMSGID ;
	        chend = ' ' ;
	        cp = ep->messageid ;
	        cl = strlen(cp) ;

	        if (cl == n) {
	            cl -= 1 ;
	            chend = cp[cl] ;
	        } else if (cl > n) {
	            cl = (n - 1) ;
	            chend = 0xAC ;
	        } /* end if */

	        rs = bprintf(ofp,"  %t%c\n",cp,cl,chend) ;

	    } /* end if */

	    if (rs < 0) break ;
	} /* end for (print-out loop) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: vecobj_count() rs=%d\n",
	        vecobj_count(&entries)) ;
#endif

ret1:
	vecobj_finish(&entries) ;

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (process) */


static int loadnames(PROGINFO *pip,VECSTR *nlp,cchar *sp,int sl)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;

	if (nlp == NULL) return SR_FAULT ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = loadname(pip,nlp,fp,fl) ;
	            if (rs > 0) c += rs ;
		}
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadnames) */


static int loadname(PROGINFO *pip,vecstr *nlp,cchar cp[],int cl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if (cp[0] == '-') {
	    cp = lip->username ;
	    cl = -1 ;
	    if (lip->username[0] == '\0') {
	        rs = getusername(lip->username,USERNAMELEN,-1) ;
	        cl = rs ;
	    }
	}

	if (rs >= 0) {
	    rs = vecstr_adduniq(nlp,cp,cl) ;
	}

	return rs ;
}
/* end subroutine (loadname) */


static int cmp_utime(void **e1pp,void **e2pp)
{
	MSGID_ENT	*e1p, *e2p ;

	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

	e1p = *e1pp ;
	e2p = *e2pp ;
	return (e2p->utime - e1p->utime) ;
}
/* end subroutine (cmp_utime) */


static int cmpr_utime(void **e1pp,void **e2pp)
{
	MSGID_ENT	*e1p, *e2p ;

	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e2pp == NULL)
	    return 1 ;

	if (*e1pp == NULL)
	    return -1 ;

	e1p = *e1pp ;
	e2p = *e2pp ;
	return (e1p->utime - e2p->utime) ;
}
/* end subroutine (cmpr_utime) */


static int cmp_ctime(void **e1pp,void **e2pp)
{
	MSGID_ENT	*e1p, *e2p ;

	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

	e1p = *e1pp ;
	e2p = *e2pp ;
	return (e2p->ctime - e1p->ctime) ;
}
/* end subroutine (cmp_ctime) */


static int cmpr_ctime(void **e1pp,void **e2pp)
{
	MSGID_ENT	*e1p, *e2p ;

	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e2pp == NULL)
	    return 1 ;

	if (*e1pp == NULL)
	    return -1 ;

	e1p = *e1pp ;
	e2p = *e2pp ;
	return (e1p->ctime - e2p->ctime) ;
}
/* end subroutine (cmpr_ctime) */


static int cmp_mtime(void **e1pp,void **e2pp)
{
	MSGID_ENT	*e1p, *e2p ;

	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

	e1p = *e1pp ;
	e2p = *e2pp ;
	return (e2p->mtime - e1p->mtime) ;
}
/* end subroutine (cmp_mtime) */


static int cmpr_mtime(void **e1pp,void **e2pp)
{
	MSGID_ENT	*e1p, *e2p ;

	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e2pp == NULL)
	    return 1 ;

	if (*e1pp == NULL)
	    return -1 ;

	e1p = *e1pp ;
	e2p = *e2pp ;
	return (e1p->mtime - e2p->mtime) ;
}
/* end subroutine (cmpr_mtime) */


