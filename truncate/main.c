/* main */

/* generic (?) front-end */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_LOCSETENT	0		/* |locinfo_setentry()| */


/* revision history:

	= 1999-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ truncate -<size> file(s)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<bits.h>
#include	<field.h>
#include	<keyopt.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"kshlib.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	mkpath1w(char *,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecmfll(const char *,int,LONG *) ;
extern int	cfdecull(const char *,int,ULONG *) ;
extern int	cfdecmfui(const char *,int,int *) ;
extern int	cfdecmfull(const char *,int,ULONG *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	fperm(int,uid_t,gid_t,gid_t *,int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		trunclen:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	struct proginfo	*pip ;
	LONG		trunclen ;
} ;


/* forward references */

static int	usage(struct proginfo *pip) ;

static int	procargs(struct proginfo *,struct arginfo *,BITS *,
			const char *,const char *) ;
static int	procspecs(PROGINFO *,void *,const char *,int) ;
static int	procspec(struct proginfo *,bfile *,const char *,int) ;

static int	locinfo_start(struct locinfo *,struct proginfo *) ;
static int	locinfo_finish(struct locinfo *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(struct locinfo *,const char **,
			const char *,int) ;
#endif /* CF_LOCSETENT */


/* local variables */

static const char	*argopts[] = {
	"VERSION",
	"VERBOSE",
	"HELP",
	"ROOT",
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
	argopt_root,
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
	{ 0, 0 }
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


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;
	struct locinfo	li, *lip = &li ;
	struct arginfo	ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	bfile		errfile ;
	LONG		trunclen = -1 ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	rs, rs1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_help = FALSE ;
	int	f_usage = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*sn = NULL ;
	const char	*pr = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUG */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* pre-initialization */

	pip->verboselevel = 1 ;

	pip->lip = &li ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

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
		int ch = (argp[1] & 0xff) ;

	        if (isdigit(ch)) {

	            argval = (argp+1) ;

	        } else if (ch == '-') {

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

/* do we have a keyword match or should we assume only key letters? */

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                    switch (kwi) {

	                    case argopt_help:
	                        f_help = TRUE ;
	                        break ;

			    case argopt_root:
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					pr = argp ;
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

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            sn = avp ;
				}
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            sn = argp ;
				}
	                    }
	                    break ;

/* argument-list file */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            afname = avp ;
				}
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            afname = argp ;
				}
	                    }
	                    break ;

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            efname = avp ;
				}
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            efname = argp ;
				}
	                    }
	                    break ;

/* output file */
	                case argopt_of:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            ofname = avp ;
				}
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            ofname = argp ;
				}
	                    }
	                    break ;

/* handle all keyword defaults */
	                    default:
	                        rs = SR_INVALID ;
	                        f_usage = TRUE ;
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

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* output file name */
	                        case 'l':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					LONG	vv ;
	                                pip->f.trunclen = TRUE ;
	                                rs = cfdecmfll(argp,argl,&vv) ;
					trunclen = vv ;
	                            }
	                            break ;

/* quiet mode */
	                        case 'q':
	                            pip->f.quiet = TRUE ;
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
	                            f_usage = TRUE ;
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

	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	}

	if (rs < 0) goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: debuglevel=%u verboselevel=%d\n",
	        pip->progname,
		pip->debuglevel, pip->verboselevel) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: f_usage=%u f_help=%u\n",f_usage,f_help) ;
#endif

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* other initialization */

	if ((rs >= 0) && (argval != NULL) && (trunclen < 0)) {
	    pip->f.trunclen = TRUE ;
	    rs = cfdecmfll(argval,-1,&trunclen) ;
	}
	lip->trunclen = trunclen ;

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    bprintf(pip->efp,"%s: default length=%lld\n",
		pip->progname,lip->trunclen) ;
	}

	memset(&ainfo,0,sizeof(struct arginfo)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    const char	*afn = afname ;
	    const char	*ofn = ofname ;
	    rs = procargs(pip,&ainfo,&pargs,ofn,afn) ;
	}

done:
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        break ;
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;
	    case SR_INTR:
		ex = EX_INTR ;
		break ;
	    default:
	        ex = mapex(mapexs,rs) ;
		break ;
	    } /* end switch */
	} else if (rs >= 0) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

/* early return thing */
retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",
	        ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument(s) given\n",
	    pip->progname) ;
	usage(pip) ;
	goto retearly ;
}
/* end subroutine (main) */


/* local subroutines */


static int usage(struct proginfo *pip) 
{
	int		rs ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s <file>[=<len>] [...] [-l <length>]\n",
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procargs(pip,aip,bop,ofname,afname)
struct proginfo	*pip ;
struct arginfo	*aip ;
BITS		*bop ;
const char	*ofname ;
const char	*afname ;
{
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = STDOUTFNAME ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0) {
	    int		pan = 0 ;
	    int		cl ;
	    const char	*cp ;

	    if (rs >= 0) {
	        int	ai ;
	        int	f ;
	        for (ai = 1 ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	            if (f) {
	                cp = aip->argv[ai] ;
			if (cp[0] != '\0') {
	                    pan += 1 ;
	                    rs = procspec(pip,ofp,cp,-1) ;
			    c += 1 ;
			}
		    }

		    if (rs >= 0) rs = lib_sigterm() ;
		    if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if */

	    if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	        bfile	afile, *afp = &afile ;

	        if (strcmp(afname,"-") == 0)
	            afname = STDINFNAME ;

	        if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
		    const int	llen = LINEBUFLEN ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = breadline(afp,lbuf,llen)) > 0) {
	                int	len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

			    if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
			        if (cp[0] != '#') {
	                	    pan += 1 ;
	                	    rs = procspecs(pip,ofp,cp,cl) ;
			    	    c += rs ;
			        }
			    }

			    if (rs >= 0) rs = lib_sigterm() ;
			    if (rs >= 0) rs = lib_sigintr() ;
	   	        if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = bclose(afp) ;
	    	    if (rs >= 0) rs = rs1 ;
	        } else {
	                bprintf(pip->efp,
	                    "%s: inaccessible argument-list (%d)\n",
	                        pip->progname,rs) ;
	                bprintf(pip->efp,"%s: afile=%s\n",
	                    pip->progname,afname) ;
	        } /* end if */

	    } /* end if (processing file argument file list) */

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    bprintf(pip->efp,"%s: inaccessible output (%d)\n",
		pip->progname,rs) ;
	} /* end if (open-file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_rest/procargs: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procargs) */


static int procspecs(PROGINFO *pip,void *ofp,const char *lbuf,int len)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;
	if ((rs = field_start(&fsb,lbuf,len)) >= 0) {
	    int		fl ;
	    const char	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
		if (fl > 0) {
		    rs = procspec(pip,ofp,fp,fl) ;
		    c += rs ;
		}
		if (fsb.term == '#') break ;
		if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procspecs) */


/* ARGSUSED */
static int procspec(pip,ofp,np,nl)
struct proginfo	*pip ;
bfile		*ofp ;
const char	np[] ;
int		nl ;
{
	LOCINFO		*lip = pip->lip ;
	LONG		flen ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	int		f_open = FALSE ;
	const char	*pn = pip->progname ;
	const char	*fname ;
	char		tbuf[MAXPATHLEN + 1] ;

	if (np == NULL) return SR_FAULT ;
	if (np[0] == '\0') return SR_OK ;

	if (nl < 0) nl = strlen(np) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: np=>%t<\n",np,nl) ;
#endif

	flen = lip->trunclen ;

	if (! pip->f.literal) {
	    const char	*tp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: not literal\n") ;
#endif

	    if ((tp = strnchr(np,nl,'=')) != NULL) {
		const int	tl = ((np+nl)-(tp+1)) ;
	        fname = tbuf ;
	        if ((rs = mkpath1w(tbuf,np,(tp-np))) >= 0) {
		    if (tl > 0) {
	                rs = cfdecmfll((tp+1),tl,&flen) ;
		    }
		}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: flen=%lld\n",flen) ;
#endif

	    } else {
	    fname = tbuf ;
	    rs = mkpath1w(tbuf,np,nl) ;
	    }

	} else {
	    fname = tbuf ;
	    rs = mkpath1w(tbuf,np,nl) ;
	}

/* do it */

	if (rs >= 0) {
	    int	fd = FD_STDIN ;

	if (strcmp(fname,"-") != 0) {
	    f_open = TRUE ;
	    rs = u_open(fname,O_WRONLY,0666) ;
	    fd = rs ;
	} else
	    rs = fperm(fd,-1,-1,NULL,W_OK) ;

	if (rs >= 0) {
	    struct ustat	sb ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
		if (S_ISREG(sb.st_mode)) {
		    if (pip->debuglevel > 0) {
			bprintf(pip->efp,"%s: len=%lld file=%s\n",
				pip->progname,flen,fname) ;
		    }
		    if (sb.st_size > flen) {
			offset_t	foff = flen ;
			if ((rs = uc_ftruncate(fd,foff)) >= 0) {
	    		    f = TRUE ;
			}
		     }
		} else {
		     printf(pip->efp,"%s: not regular file (%d)\n",pn,rs) ;
		     printf(pip->efp,"%s: file=%s\n",pn,tbuf) ;
		}
	    } /* end if (stat) */
	    if (f_open) u_close(fd) ;
	} /* end if (file) */

	} /* end if (ok) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procspec) */


static int locinfo_start(lip,pip)
struct locinfo	*lip ;
struct proginfo	*pip ;
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(struct locinfo)) ;
	lip->pip = pip ;
	lip->trunclen = -1 ;

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


