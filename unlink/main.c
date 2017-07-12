/* main (unlink) */

/* front-end subroutine for the UNLINK program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_FOLLOWFILES	0		/* follow sybolic links of files */
#define	CF_FTCASE	1		/* try a C-lang 'switch' */


/* revision history:

	= 1998-02-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a fairly generic front-end subroutine for a program.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<baops.h>
#include	<bfile.h>
#include	<hdb.h>
#include	<fsdirtree.h>
#include	<sigblock.h>
#include	<bwops.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif

#define	LINKINFO	struct linkinfo

#define	DMODE		0775


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	removes(const char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	mkuserpath(char *,const char *,const char *,int) ;
extern int	fileobject(const char *) ;
extern int	strwcmp(const char *,const char *,int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* local structures */

struct fileinfo_flags {
	uint		dangle:1 ;
} ;

struct fileinfo {
	struct fileinfo_flags	f ;
	uint		fts ;
} ;

struct linkinfo {
	ino64_t		ino ;
	const char	*fname ;
} ;


/* forward references */

static uint	linkhash(const void *,int) ;

static int	usage(struct proginfo *) ;

static int	procopts(struct proginfo *) ;
static int	procfts(struct proginfo *) ;
static int	procname(struct proginfo *,const char *) ;
static int	procdir(struct proginfo *,const char *,FSDIRTREE_STAT *) ;
static int	procother(struct proginfo *,const char *,FSDIRTREE_STAT *) ;

static int	procsufbegin(struct proginfo *) ;
static int	procsufhave(struct proginfo *,const char *,int) ;
static int	procsufend(struct proginfo *) ;

static int	procloadsuf(struct proginfo *,int,const char *,int) ;

static int	procprintfts(struct proginfo *,const char *) ;
static int	procprintsufs(struct proginfo *,const char *) ;

static int	proclinkbegin(struct proginfo *) ;
static int	proclinkadd(struct proginfo *,ino64_t,const char *) ;
static int	proclinkhave(struct proginfo *,ino64_t,LINKINFO **) ;
static int	proclinkend(struct proginfo *) ;

static int	procsize(struct proginfo *,const char *, 
			FSDIRTREE_STAT *,struct fileinfo *) ;
static int	proclink(struct proginfo *,const char *, 
			FSDIRTREE_STAT *,struct fileinfo *) ;
static int	procsync(struct proginfo *,const char *, 
			FSDIRTREE_STAT *,struct fileinfo *) ;
static int	procrm(struct proginfo *,const char *, 
			FSDIRTREE_STAT *,struct fileinfo *) ;

static int	procsynclink(struct proginfo *,const char *,
			FSDIRTREESTAT *,LINKINFO *) ;
static int	procsyncer(struct proginfo *,const char *,
			FSDIRTREESTAT *) ;

static int	procsyncer_reg(struct proginfo *,const char *,
			FSDIRTREESTAT *) ;
static int	procsyncer_dir(struct proginfo *,const char *,
			FSDIRTREESTAT *) ;
static int	procsyncer_lnk(struct proginfo *,const char *,
			FSDIRTREESTAT *) ;
static int	procsyncer_fifo(struct proginfo *,const char *,
			FSDIRTREESTAT *) ;
static int	procsyncer_sock(struct proginfo *,const char *,
			FSDIRTREESTAT *) ;

static int	linkinfo_start(LINKINFO *,ino64_t,const char *) ;
static int	linkinfo_finish(LINKINFO *) ;

static int	mkpdirs(const char *,mode_t) ;
static int	istermrs(int) ;

static int	linkcmp(struct linkinfo *,struct linkinfo *,int) ;


/* external variables */


/* local structures */

enum fts {
	ft_r,
	ft_d,
	ft_b,
	ft_c,
	ft_p,
	ft_n,
	ft_l,
	ft_s,
	ft_D,
	ft_e,
	ft_overlast
} ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"sn",
	"pm",
	"af",
	"ef",
	"of",
	"sa",
	"sr",
	"yf",
	"yi",
	"iacc",
	"nice",
	"delay",
	"option",
	"set",
	"follow",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_sn,
	argopt_pm,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_sa,
	argopt_sr,
	argopt_yf,
	argopt_yi,
	argopt_iacc,
	argopt_nice,
	argopt_delay,
	argopt_option,
	argopt_set,
	argopt_follow,
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
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static const char	*progmodes[] = {
	"filesize",
	"filefind",
	"filelinker",
	"filesyncer",
	"filerm",
	NULL
} ;

enum progmodes {
	progmode_filesize,
	progmode_filefind,
	progmode_filelinker,
	progmode_filesyncer,
	progmode_filerm,
	progmode_overlast
} ;

static const char	*progopts[] = {
	"uniq",
	"name",
	"noprog",
	"nosock",
	"nopipe",
	"nofifo",
	"nodev",
	"noname",
	"nolink",
	"noextra",
	"cores",
	"s",
	"sa",
	"sr",
	"nosuf",
	"follow",
	"younger",
	"yi",
	"iacc",
	"quiet",
	"nice",
	NULL
} ;

enum progopts {
	progopt_uniq,
	progopt_name,
	progopt_noprog,
	progopt_nosock,
	progopt_nopipe,
	progopt_nofifo,
	progopt_nodev,
	progopt_noname,
	progopt_nolink,
	progopt_noextra,
	progopt_cores,
	progopt_s,
	progopt_sa,
	progopt_sr,
	progopt_nosuf,
	progopt_follow,
	progopt_younger,
	progopt_yi,
	progopt_iacc,
	progopt_quiet,
	progopt_nice,
	progopt_overlast
} ;

static const char	*ftstrs[] = {
	"file",
	"directory",
	"block",
	"character",
	"pipe",
	"fifo",
	"name",
	"socket",
	"link",
	"door",
	"exists",
	"regular",
	NULL
} ;

enum ftstrs {
	ftstr_file,
	ftstr_directory,
	ftstr_block,
	ftstr_character,
	ftstr_pipe,
	ftstr_fifo,
	ftstr_name,
	ftstr_socket,
	ftstr_link,
	ftstr_door,
	ftstr_exists,
	ftstr_regular,
	ftstr_overlast
} ;

#ifdef	COMMENT

static const char	*whiches[] = {
	"uniq",
	"name",
	"noprog",
	"nosock",
	"nopipe",
	"nodev",
	"nolink",
	NULL
} ;

enum whiches {
	which_uniq,
	which_name,
	which_noprog,
	which_nosock,
	which_nopipe,
	which_nodev,
	which_nolink,
	which_overlast
} ;

#endif /* COMMENT */

static const char	*po_fts = PO_FTS ;
static const char	*po_sufreq = PO_SUFREQ ;
static const char	*po_sufacc = PO_SUFACC ;
static const char	*po_sufrej = PO_SUFREJ ;

static const char	*sufs[] = {
	PO_SUFREQ,
	PO_SUFACC,
	PO_SUFREJ,
	NULL
} ;

enum sufs {
	suf_req,
	suf_acc,
	suf_rej,
	suf_overlast
} ;

static const int	termrs[] = {
	SR_FAULT,
	SR_INVALID,
	SR_NOMEM,
	SR_NOANODE,
	SR_BADFMT,
	SR_NOSPC,
	SR_NOSR,
	SR_NOBUFS,
	SR_BADF,
	SR_OVERFLOW,
	SR_RANGE,
	0
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	BITS	pargs ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;

	uint	mo_start = 0 ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan = 0 ;
	int	rs, rs1 ;
	int	cl ;
	int	v ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*pmspec = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*efname = NULL ;
	const char	*yfname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFNAME)) == NULL) {
	    if ((cp = getenv(VARDEBUGFD1)) == NULL)
	        cp = getenv(VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
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

/* early things to initialize */

	pip->ofp = ofp ;
	pip->namelen = MAXNAMELEN ;
	pip->verboselevel = 1 ;

	pip->bytes = 0 ;
	pip->megabytes = 0 ;
	pip->progmode = -1 ;

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	if (rs >= 0) {
	    rs = keyopt_start(&pip->akopts) ;
	    pip->open.akopts = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = paramopt_start(&pip->aparams) ;
	    pip->open.aparams = (rs >= 0) ;
	}

	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc ;
	for (ai = 0 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = argv[ai] ;
	    argl = strlen(argp) ;

#if	CF_DEBUGS
	    debugprintf("main: arg consider=>%t<\n",argp,argl) ;
#endif

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {

#if	CF_DEBUGS
	    debugprintf("main: arg=%u option\n",ai) ;
#endif

	        if (isdigit(argp[1])) {

	            argval = (argp+1) ;

	        } else if (argp[1] == '-') {

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

	            kwi = matostr(argopts,2,akp,akl) ;

	            if (kwi >= 0) {

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
	                        rs = keyopt_loads(&pip->akopts,argp,argl) ;
	                    break ;

/* search name */
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
	                        if (avl) {
	                            rs = optvalue(avp,avl) ;
	                            pip->verboselevel = rs ;
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
	                case argopt_set:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl)
	                        rs = paramopt_loadu(&pip->aparams,argp,argl) ;
	                    break ;

/* argument files */
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

/* error file */
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

/* follow symbolic links */
	                case argopt_follow:
	                    pip->final.follow = TRUE ;
	                    pip->have.follow = TRUE ;
	                    pip->f.follow = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.follow = (rs > 0) ;
	                        }
	                    }
	                    break ;

/* ignore inaccessible files */
	                case argopt_iacc:
	                    pip->final.iacc = TRUE ;
	                    pip->have.iacc = TRUE ;
	                    pip->f.iacc = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.iacc = (rs > 0) ;
	                        }
	                    }
	                    break ;

/* nice value */
	                case argopt_nice:
	                    cp = NULL ;
	                    cl = -1 ;
	                    pip->final.nice = TRUE ;
	                    pip->have.nice = TRUE ;
	                    pip->f.nice = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
	                            cl = avl ;
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
	                            cp = argp ;
	                            cl = argl ;
	                        }
	                    }
	                    if (cp != NULL) {
	                        rs = optvalue(cp,cl) ;
	                        pip->nice = rs ;
	                    }
	                    break ;

/* delay value */
	                case argopt_delay:
	                    cp = NULL ;
	                    cl = -1 ;
	                    pip->final.nice = TRUE ;
	                    pip->have.nice = TRUE ;
	                    pip->f.nice = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
	                            cl = avl ;
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
	                            cp = argp ;
	                            cl = argl ;
	                        }
	                    }
	                    if (cp != NULL) {
	                        rs = cfdecti(cp,cl,&v) ;
	                        pip->delay = v ;
	                    }
	                    break ;

/* suffix-accept */
	                case argopt_sa:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl) {
	                        pip->final.sufacc = TRUE ;
	                        rs = procloadsuf(pip,suf_acc,
	                            argp,argl) ;
	                    }
	                    break ;

/* suffix-reject */
	                case argopt_sr:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl) {
	                        pip->final.sufrej = TRUE ;
	                        rs = procloadsuf(pip,suf_rej,
	                            argp,argl) ;
	                    }
	                    break ;

/* younger-file */
	                case argopt_yf:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl)
	                        yfname = argp ;
	                    break ;

/* younger-interval */
	                case argopt_yi:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl) {
	                        pip->have.younger = TRUE ;
	                        pip->final.younger = TRUE ;
	                        rs = cfdecti(argp,argl,&v) ;
	                        pip->younger = v ;
	                    }
	                    break ;

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
			    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

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

	                    case 'V':
	                        f_version = TRUE ;
	                        if (f_optequal)
	                            rs = SR_INVALID ;
	                        break ;

/* continue on error */
	                    case 'c':
	                        pip->final.nostop = TRUE ;
	                        pip->have.nostop = TRUE ;
	                        pip->f.nostop = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.nostop = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* target directory */
	                    case 'd':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            pip->final.tardname = TRUE ;
	                            pip->have.tardname = TRUE ;
	                            pip->tardname = argp ;
	                        }
	                        break ;

/* follow symbolic links */
	                    case 'f':
	                        pip->final.follow = TRUE ;
	                        pip->have.follow = TRUE ;
	                        pip->f.follow = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.follow = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* file name length restriction */
	                    case 'l':
				cp = NULL ;
				cl = -1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
					cp = avp ;
					cl = avl ;
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
	                                cp = argp ;
					cl = argl ;
	                            }
	                        }
				if (cp != NULL) {
	                                rs = optvalue(cp,cl) ;
	                                pip->namelen = rs ;
				}
	                        break ;

/* no-change */
	                    case 'n':
	                        pip->f.nochange = TRUE ;
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
	                            rs = keyopt_loads(&pip->akopts,argp,argl) ;
	                        break ;

/* quiet */
	                    case 'q':
	                        pip->final.quiet = TRUE ;
	                        pip->have.quiet = TRUE ;
	                        pip->f.quiet = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.quiet = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'r':
				break ;

/* require a suffix for file names */
	                    case 's':
	                        cp = NULL ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                cp = avp ;
	                                cl = avl ;
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
	                                cp = argp ;
	                                cl = argl ;
	                            }
	                        }
	                        if (cp != NULL) {
	                            pip->final.sufreq = TRUE ;
	                            rs = procloadsuf(pip,suf_req,cp,cl) ;
	                        }
	                        break ;

/* file types */
	                    case 't':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            rs = paramopt_loads(&pip->aparams,
	                                po_fts,argp,argl) ;
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

	                    case 'y':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            pip->final.younger = TRUE ;
	                            pip->have.younger = TRUE ;
	                            rs = cfdecti(argp,argl,&v) ;
	                            pip->younger = v ;
	                        }
	                        break ;

/* allow zero number of arguments */
	                    case 'z':
	                        pip->final.zargs = TRUE ;
	                        pip->final.zargs = TRUE ;
	                        pip->f.zargs = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.zargs = (rs > 0) ;
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
	}

	if (rs < 0) {
	    ex = EX_USAGE ;
	    bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	        pip->progname,rs) ;
	    usage(pip) ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;
	    bcontrol(pip->efp,BC_LINEBUF,0) ;
	    bflush(pip->efp) ;
	}

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

/* get our program mode */

	if (pmspec == NULL) pmspec = pip->progname ;

	pip->progmode = matstr(progmodes,pmspec,-1) ;

	if (pip->progmode < 0)
	    pip->progmode = progmode_filerm ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0) {
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    } else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif /* CF_DEBUG */

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: progmode=%s(%u)\n",
	        pip->progname,progmodes[pip->progmode],pip->progmode) ;

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* initialization */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	pip->daytime = time(NULL) ;
	pip->euid = geteuid() ;
	pip->uid = getuid() ;

/* younger file? */

	if ((rs >= 0) && (pip->younger == 0)) {
	    if (argval != NULL) {
	        pip->have.younger = TRUE ;
	        rs = cfdecti(argval,-1,&v) ;
	        pip->younger = v ;
	    }
	}

	if ((rs >= 0) && (pip->younger == 0)) {
	    if ((yfname != NULL) && (yfname[0] != '\0')) {
	        struct ustat	sb ;
	        int rs1 = uc_stat(yfname,&sb) ;
	        if (rs1 >= 0) {
	            pip->have.younger = TRUE ;
	            pip->younger = (pip->daytime - sb.st_mtime) ;
	        }
	    }
	}

/* get more program options */

	if (rs >= 0)
	    rs = procopts(pip) ;

	if (rs >= 0)
	    rs = procfts(pip) ;

	if ((rs >= 0) && pip->f.cores)
	    pip->fts |= (1 << ft_r) ;

	if ((rs >= 0) && (pip->debuglevel > 0))
	    rs = procprintfts(pip,po_fts) ;

	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

	if (pip->f.f_noextra) {
	    pip->f.f_nodev = TRUE ;
	    pip->f.f_nopipe = TRUE ;
	    pip->f.f_nosock = TRUE ;
	    pip->f.f_noname = TRUE ;
	    pip->f.f_nodoor = TRUE ;
	}

/* create the 'fnos' value */

	if (pip->f.f_nodev) {
	    bwset(pip->fnos,ft_c) ;
	    bwset(pip->fnos,ft_b) ;
	}
	if (pip->f.f_noname) bwset(pip->fnos,ft_n) ;
	if (pip->f.f_nopipe) bwset(pip->fnos,ft_p) ;
	if (pip->f.f_nolink) bwset(pip->fnos,ft_l) ;
	if (pip->f.f_nosock) bwset(pip->fnos,ft_s) ;
	if (pip->f.f_nodoor) bwset(pip->fnos,ft_d) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: f_follow=%u\n",pip->f.follow) ;
	    debugprintf("main: f_continue=%u\n",pip->f.nostop) ;
	    debugprintf("main: f_iacc=%u\n",pip->f.iacc) ;
	}
#endif /* CF_DEBUG */

	if (pip->debuglevel > 0) {
	    const char	*pn = pip->progname ;
	    bprintf(pip->efp,"%s: follow=%u\n",pn,pip->f.follow) ;
	    bprintf(pip->efp,"%s: continue=%u\n",pn,pip->f.nostop) ;
	    bprintf(pip->efp,"%s: iacc=%u\n",pn,pip->f.iacc) ;
	}

/* check a few more things */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	{
	    int	f = FALSE ;

	    f = f || (pip->progmode == progmode_filelinker) ;
	    f = f || (pip->progmode == progmode_filesyncer) ;
	    if (f) {
	        const char	*fmt ;

	        if (pip->tardname == NULL) {
	            if ((cp = getenv(VARTARDNAME)) != NULL) {
	                pip->have.tardname = TRUE ;
	                pip->tardname = cp ;
	            }
	        }

	        if ((pip->tardname == NULL) || (pip->tardname[0] == '\0')) {
	            fmt ="%s: no target directory specified\n" ;
	            rs = SR_INVALID ;
	            bprintf(pip->efp,fmt, pip->progname) ;
	        }
	        if (rs >= 0) {
	            rs = fsdirtreestat(pip->tardname,0,&pip->tarstat) ;
	            if ((rs < 0) || (! S_ISDIR(pip->tarstat.st_mode))) {
	                fmt = "%s: target directory inaccessible (%d)\n" ;
	                rs = SR_NOTDIR ;
	                bprintf(pip->efp,fmt, pip->progname,rs) ;
	            }
	        }
	        if (rs < 0) {
	            ex = EX_USAGE ;
	            goto retearly ;
	        }

	    }
	} /* end block */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: tardname=%s\n",pip->tardname) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: tardname=%s\n",pip->progname,pip->tardname) ;

#ifdef	COMMENT
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: follow=%u\n",pip->progname,pip->f.follow) ;
#endif

	if ((rs >= 0) && (pip->nice > 0)) {
	    int	n = MIN(pip->nice,19) ;
	    rs = u_nice(n) ;
	}

/* get ready */

	if (rs >= 0) {
	    if ((rs = paramopt_havekey(&pip->aparams,po_sufreq)) > 0) {
	        pip->have.sufreq = TRUE ;
	        if (pip->debuglevel > 0)
	            rs = procprintsufs(pip,po_sufreq) ;
	    } /* end if */
	} /* end if */

	if (rs >= 0)
	    rs = procsufbegin(pip) ;

	if (rs >= 0) {
	    int	f = FALSE ;
	    f = f || (pip->progmode == progmode_filesyncer) ;
	    if (f)
	        rs = proclinkbegin(pip) ;
	}

	if (rs < 0)
	    goto done ;

/* start processing */

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = BFILE_STDOUT ;
	rs = bopen(ofp,ofname,"wct",0644) ;
	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: output unavailable (%d)\n",
	        pip->progname,rs) ;
	    goto badoutopen ;
	}

/* OK, we do it */

#if	CF_DEBUGS
	    debugprintf("main: ai_max=%u ai_pos=%u\n",ai_max,ai_pos) ;
#endif

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (! f) continue ;
	    cp = argv[ai] ;

	    pan += 1 ;
	    rs = procname(pip,cp) ;

	    if (rs < 0) {
	        bprintf(pip->efp,
	            "%s: error name=%s (%d)\n",
	            pip->progname,cp,rs) ;
	        break ;
	    }

	} /* end for (looping through requested circuits) */

/* process any files in the argument filename list file */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(afname,"-") == 0) afname = BFILE_STDIN ;

	    if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
		const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            cp = lbuf ;
	            cl = len ;
	            if (cl == 0) continue ;

	            pan += 1 ;
	            rs = procname(pip,cp) ;

	            if (rs < 0) {
	                bprintf(pip->efp,
	                    "%s: error name=%s (%d)\n",
	                    pip->progname,cp,rs) ;
	                break ;
	            }

	        } /* end while (reading lines) */

	        bclose(afp) ;
	    } else {
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: could not open the argument list file (%d)\n",
	                pip->progname,rs) ;
	            bprintf(pip->efp,"%s: \targfile=%s\n",
	                pip->progname,afname) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (pip->progmode == progmode_filesize)) {
	    long	blocks, blockbytes ;

	    bprintf(ofp,"%lu megabyte%s and %lu bytes\n",
	        pip->megabytes,
	        ((pip->megabytes == 1) ? "" : "s"),
	        pip->bytes) ;

/* calculate UNIX blocks */

	    blocks = pip->megabytes * 1024 * 2 ;
	    blocks += (pip->bytes / UNIXBLOCK) ;
	    blockbytes = (pip->bytes % UNIXBLOCK) ;

	    bprintf(ofp,"%lu UNIX blocks and %lu bytes\n",
	        blocks,blockbytes) ;

	} /* end if (program mode=filesize) */

	bclose(ofp) ;

badoutopen:
	{
	    int	f = FALSE ;
	    f = f || (pip->progmode == progmode_filesyncer) ;
	    if (f) {
	        rs1 = proclinkend(pip) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	}

	procsufend(pip) ;

	if ((pip->debuglevel > 0) && (pan > 0))
	    bprintf(pip->efp,"%s: files=%u processed=%u\n",
	        pip->progname,pip->c_files,pip->c_processed) ;

	if ((rs >= 0) && (pan == 0) && (! pip->f.zargs) && (! pip->f.quiet)) {
	    ex = EX_USAGE ;
	    bprintf(pip->efp,"%s: no files or directories were specified\n",
	        pip->progname) ;
	}

done:
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet)
	            bprintf(pip->efp,"%s: invalid usage (%d)\n",
	                pip->progname,rs) ;
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

retearly:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    paramopt_finish(&pip->aparams) ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&pip->akopts) ;
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
}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen = 0 ;

	const char	*pn = pip->progname ;
	const char	*fmt ;


	fmt = "%s: USAGE> %s [<file(s)|dir(s)> ...] [-af {<argfile>|-}] \n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-f <file(s)|dir(s)>] [-delay <delay>]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procopts(pip)
struct proginfo	*pip ;
{
	KEYOPT		*kop = &pip->akopts ;
	KEYOPT_CUR	kcur ;

	int	rs = SR_OK ;
	int	oi ;
	int	kl, vl ;
	int	v ;
	int	c = 0 ;

	const char	*kp, *vp ;
	const char	*cp ;


	if ((cp = getenv(VAROPTS)) != NULL)
	    rs = keyopt_loads(kop,cp,-1) ;

	if (rs < 0)
	    goto ret0 ;

/* process program options */

	if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {

	    while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	        vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	        if ((oi = matostr(progopts,1,kp,kl)) >= 0) {

	            c += 1 ;
	            switch (oi) {

	            case progopt_uniq:
	                if (! pip->final.f_uniq) {
	                    pip->final.f_uniq = TRUE ;
	                    pip->f.f_uniq = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.f_uniq = (rs > 0) ;
	                    }
	                }
	                break ;

/* [what is this?] */
	            case progopt_name:
	                if (! pip->final.f_name) {
	                    pip->final.f_name = TRUE ;
	                    pip->f.f_name = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.f_name = (rs > 0) ;
	                    }
	                }
	                break ;

	            case progopt_noprog:
	                if (! pip->final.f_noprog) {
	                    pip->final.f_noprog = TRUE ;
	                    pip->f.f_noprog = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.f_noprog = (rs > 0) ;
	                    }
	                }
	                break ;

	            case progopt_nosock:
	                if (! pip->final.f_nosock) {
	                    pip->final.f_nosock = TRUE ;
	                    pip->f.f_nosock = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.f_nosock = (rs > 0) ;
	                    }
	                }
	                break ;

	            case progopt_nopipe:
	            case progopt_nofifo:
	                if (! pip->final.f_nopipe) {
	                    pip->final.f_nopipe = TRUE ;
	                    pip->f.f_nopipe = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.f_nopipe = (rs > 0) ;
	                    }
	                }
	                break ;

	            case progopt_nodev:
	                if (! pip->final.f_nodev) {
	                    pip->final.f_nodev = TRUE ;
	                    pip->f.f_nodev = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.f_nodev = (rs > 0) ;
	                    }
	                }
	                break ;

	            case progopt_noname:
	                if (! pip->final.f_noname) {
	                    pip->final.f_noname = TRUE ;
	                    pip->f.f_noname = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.f_noname = (rs > 0) ;
	                    }
	                }
	                break ;

	            case progopt_nolink:
	                if (! pip->final.f_nolink) {
	                    pip->final.f_nolink = TRUE ;
	                    pip->f.f_nolink= TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.f_nolink = (rs > 0) ;
	                    }
	                }
	                break ;

	            case progopt_noextra:
	                if (! pip->final.f_noextra) {
	                    pip->final.f_noextra = TRUE ;
	                    pip->f.f_noextra = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.f_noextra = (rs > 0) ;
	                    }
	                }
	                break ;

	            case progopt_cores:
	                if (! pip->final.cores) {
	                    pip->final.cores = TRUE ;
	                    pip->f.cores = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.cores = (rs > 0) ;
	                    }
	                }
	                break ;

	            case progopt_s:
	                if ((vl > 0) && (! pip->final.sufreq))
	                    rs = procloadsuf(pip,suf_req,vp,vl) ;
	                break ;

	            case progopt_sa:
	                if ((vl > 0) && (! pip->final.sufacc))
	                    rs = procloadsuf(pip,suf_acc,vp,vl) ;
	                break ;

	            case progopt_sr:
	            case progopt_nosuf:
	                if ((vl > 0) && (! pip->final.sufrej))
	                    rs = procloadsuf(pip,suf_rej,vp,vl) ;
	                break ;

	            case progopt_follow:
	                if (! pip->final.follow) {
	                    pip->final.follow = TRUE ;
	                    pip->have.follow = TRUE ;
	                    pip->f.follow = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.follow = (rs > 0) ;
	                    }
	                }
	                break ;

	            case progopt_younger:
	            case progopt_yi:
	                if ((vl > 0) && (! pip->final.younger)) {
	                    pip->final.younger = TRUE ;
	                    pip->have.younger = TRUE ;
	                    rs = cfdecti(vp,vl,&v) ;
	                    pip->younger = v ;
	                }
	                break ;

	            case progopt_iacc:
	                if (! pip->final.iacc) {
	                    pip->final.iacc = TRUE ;
	                    pip->have.iacc = TRUE ;
	                    pip->f.iacc = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.iacc = (rs > 0) ;
	                    }
	                }
	                break ;

	            case progopt_quiet:
	                if (! pip->final.quiet) {
	                    pip->final.quiet = TRUE ;
	                    pip->have.quiet = TRUE ;
	                    pip->f.iacc = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.quiet = (rs > 0) ;
	                    }
	                }
	                break ;

	            case progopt_nice:
	                if ((vl > 0) && (! pip->final.nice)) {
	                    pip->final.nice = TRUE ;
	                    pip->have.nice = TRUE ;
	                    rs = optvalue(vp,vl) ;
	                    pip->nice = rs ;
	                }
	                break ;

	            } /* end switch */

	        } /* end if */

	        if (rs < 0) break ;
	    } /* end while */

	    keyopt_curend(kop,&kcur) ;
	} /* end if (cursor) */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procfts(pip)
struct proginfo	*pip ;
{
	PARAMOPT	*pop = &pip->aparams ;
	PARAMOPT_CUR	cur ;

	int	rs = SR_OK ;
	int	vl ;
	int	fti ;
	int	c = 0 ;

	const char	*varftypes = getenv(VARFTYPES) ;
	const char	*vp ;


	if (varftypes != NULL)
	    rs = paramopt_loads(pop,po_fts,varftypes,-1) ;

	if (rs < 0) goto ret0 ;

	if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {

	    while (rs >= 0) {

	        vl = paramopt_fetch(pop,po_fts,&cur,&vp) ;
	        if (vl == SR_NOTFOUND) break ;

	        if (vl == 0) continue ;

	        rs = vl ;
	        if (rs < 0)
	            break ;

	        fti = matostr(ftstrs,1,vp,vl) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/procfts: v=%t\n",vp,vl) ;
#endif

	        if (fti >= 0) {

	            switch (fti) {

	            case ftstr_regular:
	            case ftstr_file:
	                pip->fts |= (1 << ft_r) ;
	                break ;

	            case ftstr_directory:
	                pip->fts |= (1 << ft_d) ;
	                break ;

	            case ftstr_block:
	                pip->fts |= (1 << ft_b) ;
	                break ;

	            case ftstr_character:
	                pip->fts |= (1 << ft_c) ;
	                break ;

	            case ftstr_pipe:
	            case ftstr_fifo:
	                pip->fts |= (1 << ft_p) ;
	                break ;

	            case ftstr_socket:
	                pip->fts |= (1 << ft_s) ;
	                break ;

	            case ftstr_link:
	                pip->fts |= (1 << ft_l) ;
	                break ;

	            case ftstr_name:
	                pip->fts |= (1 << ft_n) ;
	                break ;

	            case ftstr_door:
	                pip->fts |= (1 << ft_D) ;
	                break ;

	            case ftstr_exists:
	                pip->fts |= (1 << ft_e) ;
	                break ;

	            } /* end switch */

	            c += 1 ;

	        } /* end if */

	    } /* end while */

	    paramopt_curend(pop,&cur) ;
	} /* end if (cursor) */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procfts) */


int procname(pip,name)
struct proginfo	*pip ;
const char	name[] ;
{
	FSDIRTREE_STAT	sb, ssb, *sbp = &sb ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	f_islink = FALSE ;
	int	f_isdir = FALSE ;

	char	tmpfname[MAXPATHLEN+1] ;


	if (name == NULL)
	    return SR_FAULT ;

	if (name[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procname: entered name=%s\n",name) ;
#endif

	if ((name[0] == '/') && (name[1] == 'u')) {
	    rs = mkuserpath(tmpfname,NULL,name,-1) ;
	    if (rs > 0) name = tmpfname ;
	}

	if (rs >= 0)
	    rs = fsdirtreestat(name,1,&sb) ; /* this is LSTAT */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    if (rs < 0) sb.st_mode = 0 ;
	    debugprintf("main/procname: fsdirtreestat() rs=%d mode=%0o\n", 
	        rs,sb.st_mode) ;
	}
#endif

	if (rs < 0) goto ret0 ;

	f_isdir = S_ISDIR(sb.st_mode) ;
	f_islink = S_ISLNK(sb.st_mode) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procname: f_isdir=%u\n",f_isdir) ;
	    debugprintf("main/procname: f_islink=%u\n",f_islink) ;
	}
#endif

	if (f_islink && pip->f.follow) {

	    rs1 = fsdirtreestat(name,0,&ssb) ; /* STAT */
	    if (rs1 >= 0) {
	        f_isdir = S_ISDIR(ssb.st_mode) ;
	        sbp = &ssb ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procname: symlink dangle=%u\n",
	            ((rs1 < 0)?1:0)) ;
#endif

	} /* end if */

	if (rs >= 0) {
	    if (f_isdir) {
	        rs = procdir(pip,name,sbp) ;
	    } else
	        rs = procother(pip,name,sbp) ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procname) */


static int procdir(pip,name,sbp)
struct proginfo	*pip ;
const char	name[] ;
FSDIRTREE_STAT	*sbp ;
{
	int		rs = SR_OK ;
	int		opts ;
	int		size ;
	int		nl ;
	int		c = 0 ;
	char		*p ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/procdir: name=%s\n",name) ;
#endif

	nl = strlen(name) ;
	while ((nl > 0) && (name[nl-1] == '/')) nl -= 1 ;

/* do all of our children */

	size = (nl + 1 + MAXPATHLEN + 1) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    FSDIRTREE		d ;
	    FSDIRTREE_STAT	fsb ;
	    char		*fname = p ;
	    char		*bp ;

	    bp = strwcpy(fname,name,nl) ;
	    *bp++ = '/' ;

	    opts = 0 ;
	    if (pip->f.follow) opts |= FSDIRTREE_MFOLLOW ;

	    if ((rs = fsdirtree_open(&d,name,opts)) >= 0) {
		const int	mpl = MAXPATHLEN ;

	        while (rs >= 0) {
	            rs = fsdirtree_read(&d,&fsb,bp,mpl) ;
	            if ((rs == 0) || ((rs < 0) && (rs != SR_NOENT))) break ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procdir: direntry=%s\n",bp) ;
#endif

	            if (pip->debuglevel >= 2)
	                bprintf(pip->efp,"%s: looking fn=%s (%d)\n",
	                    pip->progname,fname,rs) ;

	            if (rs >= 0) {
	                c += 1 ;
	                rs = procother(pip,fname,&fsb) ;
	                if (rs >= 0) c += 1 ;
	            }
	            if (rs == SR_NOENT) rs = SR_OK ;

	        } /* end while (reading entries) */

	        fsdirtree_close(&d) ;
	    } /* end if (reading directory entries) */

	    if (rs < 0) {
	        if ((rs == SR_ACCESS) && pip->f.iacc) rs = SR_OK ;
	        if ((! pip->f.quiet) && (rs < 0)) {
	            bprintf(pip->efp,"%s: dname=%s (%d)\n",
	                pip->progname,name,rs) ;
		}
	        if ((! istermrs(rs)) && pip->f.nostop) rs = SR_OK ;
	    }

	    if (p != NULL) uc_free(p) ;
	} /* end if (memory allocation) */

/* do ourself */

	if (rs >= 0) {
	    c += 1 ;
	    rs = procother(pip,name,sbp) ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procdir: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdir) */


static int procother(pip,name,sbp)
struct proginfo	*pip ;
const char	name[] ;
FSDIRTREE_STAT	*sbp ;
{
	struct fileinfo	ck, *ckp = &ck ;

#if	CF_FOLLOWFILES
	FSDIRTREE_STAT	ssb ;
#endif

	int	rs = SR_OK ;
	int	rs1 ;
	int	bnl = 0 ;
	int	f_process = FALSE ;
	int	f_accept = FALSE ;
	int	f_suf ;

	const char	*bnp ;


	memset(ckp,0,sizeof(struct fileinfo)) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    const char	*cp ;
	    debugprintf("main/procother: entered filepath=%s\n",name) ;
	    if (S_ISLNK(sbp->st_mode)) cp = "LINK" ;
	    else if (S_ISDIR(sbp->st_mode)) cp = "DIR" ;
	    else if (S_ISREG(sbp->st_mode)) cp = "REG" ;
	    else cp = "OTHER" ;
	    debugprintf("main/procother: filetype=%s\n",cp) ;
	}
#endif /* CF_DEBUG */

	pip->c_files += 1 ;
	if (sbp->st_ctime == 0)
	    goto ret0 ;

/* fill in some information */

#if	CF_FTCASE
	{
	    const int	ftype = (sbp->st_mode & S_IFMT) ;
	    int		fts = 0 ;
	    switch (ftype) {
	    case S_IFIFO: fts |= (1 << ft_p) ; break ;
	    case S_IFCHR: fts |= (1 << ft_c) ; break ;
	    case S_IFDIR: fts |= (1 << ft_d) ; break ;
	    case S_IFNAM: fts |= (1 << ft_n) ; break ;
	    case S_IFBLK: fts |= (1 << ft_b) ; break ;
	    case S_IFREG: fts |= (1 << ft_r) ; break ;
	    case S_IFLNK: fts |= (1 << ft_l) ; break ;
	    case S_IFSOCK: fts |= (1 << ft_s) ; break ;
	    case S_IFDOOR: fts |= (1 << ft_D) ; break ;
	    } /* end switch */
	    ckp->fts = fts ;
	}
#else /* CF_FTCASE */
	{
	    register uint	ft = 0 ;
	    if (S_ISREG(sbp->st_mode)) ft |= (1 << ft_r) ;
	    else if (S_ISDIR(sbp->st_mode)) ft |= (1 << ft_d) ;
	    else if (S_ISBLK(sbp->st_mode)) ft |= (1 << ft_b) ;
	    else if (S_ISCHR(sbp->st_mode)) ft |= (1 << ft_c) ;
	    else if (S_ISFIFO(sbp->st_mode)) ft |= (1 << ft_p) ;
	    else if (S_ISLNK(sbp->st_mode)) ft |= (1 << ft_l) ;
	    else if (S_ISSOCK(sbp->st_mode)) ft |= (1 << ft_s) ;
	    else if (S_ISNAM(sbp->st_mode)) ft |= (1 << ft_n) ;
	    else if (S_ISDOOR(sbp->st_mode)) ft |= (1 << ft_D) ;
	    ckp->fts = ft ;
	}
#endif /* CF_FTCASE */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    int	i ;
	    for (i = 0 ; i < ft_overlast ; i += 1) {
	        if (BATSTI(&ckp->fts,i))
	            debugprintf("main/procother: ft[%u]=TRUE\n",i) ;
	    }
	}
#endif /* CF_DEBUG */

/* check age */

	if (pip->younger > 0) {
	    if ((pip->daytime - sbp->st_mtime) >= pip->younger)
	        goto done ;
	}

/* if this is a file link, see if it is a directory */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procother: symbolic link check?\n") ;
#endif

	if (S_ISLNK(sbp->st_mode)) {

	    if (pip->f.f_nolink)
	        goto done ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procother: symbolic_link YES\n") ;
#endif

#if	CF_FOLLOWFILES
	    if (pip->f.follow) {

	        sbp = &ssb ;
	        rs = fsdirtreestat(name,0,&ssb) ; /* STAT */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procother: fsdirtreestat() rs=%d\n",rs) ;
#endif

	        if (rs < 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procother: symlink DANGLING\n") ;
#endif
	            if (! pip->f.nostop) goto done ;
	            rs = SR_OK ;
	        }

	    } /* end if (follow link) */
#endif /* CF_FOLLOWFILES */

	} /* end if (symbolic-link-file) */

/* if types were specified, is this file one of them? (if not return now) */

	if ((pip->fts > 0) && ((ckp->fts & pip->fts) == 0))
	    goto done ;

	if ((pip->fnos > 0) && ((ckp->fts & pip->fnos) != 0))
	    goto done ;

/* prepare for suffix checks */

	f_suf = (pip->have.sufreq || pip->f.sufacc || pip->f.sufrej) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procother: f_suf=%u\n",f_suf) ;
#endif

	if (f_suf || pip->f.cores)
	    bnl = sfbasename(name,-1,&bnp) ;

	if (f_suf) {
	    if (bnl <= 0) f_suf = FALSE ;
	    if (f_suf && (bnl > 0) && (bnp[0] == '.')) {

	        if ((bnl == 1) ||
	            ((bnl == 2) && (bnp[1] == '.'))) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procother: name was a dotter\n") ;
#endif

	            goto ret0 ;
	        }

	    } /* end if */
	} /* end if (funny name check) */

/* check if it has a suffix already */

	f_process = TRUE ;
	if (f_suf) {
	    VECPSTR	*slp ;
	    int		sl ;
	    const char	*tp, *sp ;
	    if ((tp = strnrchr(bnp,bnl,'.')) != NULL) {
	        sp = (tp+1) ;
	        sl = ((bnp+bnl)-(tp+1)) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procother: suf=%t \n",sp,sl) ;
#endif

/* check against the suffix-required list */

	        if (pip->have.sufreq) {
	            rs = procsufhave(pip,sp,sl) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procother: procsufhave() rs=%d\n",
				rs) ;
#endif

	            if (rs == 0) {
	                f_process = FALSE ;
	            } else
	                f_accept = TRUE ;
	        }

/* check against the suffix-acceptance list */

	        if (f_process && pip->f.sufacc && (! f_accept)) {
	            slp = (pip->sufs + suf_acc) ;
	            rs1 = vecpstr_findn(slp,sp,sl) ;
	            f_accept = (rs1 >= 0) ;
	        }

/* check against the suffix-rejectance list */

	        if (f_process && pip->f.sufrej && (! f_accept)) {
	            slp = (pip->sufs + suf_rej) ;
	            rs1 = vecpstr_findn(slp,sp,sl) ;
	            if (rs1 >= 0) f_process = FALSE ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procother: suf fa=%u f_process=%u\n",
	                f_accept,f_process) ;
#endif

	    } else {
	        f_suf = FALSE ;
	        if (pip->have.sufreq) f_process = FALSE ;
	    }
	} /* end if (suffix lists) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procother: mid rs=%d f_suf=%u f_process=%u\n",
	        rs,f_suf,f_process) ;
#endif

/* check if it is a program (and disallowed) */

	if ((rs >= 0) && f_process && pip->f.f_noprog && (! f_accept)) {
	    if (S_ISREG(sbp->st_mode)) {
	        rs = fileobject(name) ;
	        f_process = (rs == 0) ;
	    }
	} /* end if (no-program) */

/* check if it is a program (and required) */

	if ((rs >= 0) && f_process && pip->f.cores && (! f_accept)) {
	    f_process = FALSE ;
	    if (S_ISREG(sbp->st_mode) && (strwcmp("core",bnp,bnl) == 0)) {
	        rs = fileobject(name) ;
	        f_process = (rs > 0) ;
	    }
	} /* end if (yes-program) */

/* process this file */

	if ((rs >= 0) && f_process) {

	    pip->c_processed += 1 ;
	    switch (pip->progmode) {

	    case progmode_filesize:
	        rs = procsize(pip,name,sbp,ckp) ;
	        break ;

	    case progmode_filefind:
	        {
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procother: name=>%s<\n",name) ;
#endif
	            if (! pip->f.quiet)
	                rs = bprintf(pip->ofp,"%s\n", name) ;
	        }
	        break ;

	    case progmode_filelinker:
	        rs = proclink(pip,name,sbp,ckp) ;
	        break ;

	    case progmode_filesyncer:
	        rs = procsync(pip,name,sbp,ckp) ;
	        break ;

	    case progmode_filerm:
	        rs = procrm(pip,name,sbp,ckp) ;
	        break ;

	    default:
	        rs = SR_NOANODE ;
	        break ;

	    } /* end switch */

	} /* end if (processed) */

done:
	if ((pip->debuglevel > 0) | ((rs < 0) && (! pip->f.quiet))) {
	    if ((rs != SR_ACCESS) || (! pip->f.iacc))
	        bprintf(pip->efp,"%s: fname=%s (%d:%u)\n",
	            pip->progname,name,rs,f_process) ;
	}

ret1:
	if (rs < 0) {
	    if ((rs == SR_ACCESS) && pip->f.iacc) rs = SR_OK ;
	    if ((! istermrs(rs)) && pip->f.nostop) rs = SR_OK ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procother: ret rs=%d f_process=%u\n",
	        rs,f_process) ;
#endif

	return (rs >= 0) ? f_process : rs ;
}
/* end subroutine (procother) */


static int procsufhave(pip,sp,sl)
struct proginfo	*pip ;
const char	*sp ;
int		sl ;
{
	PARAMOPT	*pop = &pip->aparams ;
	PARAMOPT_CUR	cur ;

	int	rs = SR_OK ;
	int	vl ;
	int	m ;
	int	f = FALSE ;

	const char	*key = po_sufreq ;
	const char	*vp ;


	if (sl < 0) sl = strlen(sp) ;

	if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {

	    while (rs >= 0) {
	        vl = paramopt_fetch(pop,key,&cur,&vp) ;
	        if (vl == SR_NOTFOUND) break ;
	        rs = vl ;
	        if ((rs >= 0) && (sl == vl)) {
	            m = nleadstr(sp,vp,vl) ;
	            f = (m == vl) ;
	            if (f) break ;
	        }
	    } /* end while */

	    paramopt_curend(pop,&cur) ;
	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procsufhave) */


static int procsufbegin(pip)
struct proginfo	*pip ;
{
	PARAMOPT	*pop = &pip->aparams ;

	VECPSTR		*vlp ;

	int	rs = SR_OK ;
	int	si ;
	int	c = 0 ;

	const char	*po = NULL ;


	for (si = 0 ; si < suf_overlast ; si += 1) {
	    int	f = FALSE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsufbegin: si=%u c=%u\n",si,c) ;
#endif

	    switch (si) {
	    case suf_req:
	        f = pip->have.sufreq ;
	        if (f) {
	            po = sufs[si] ;
	            vlp = (pip->sufs + si) ;
	        }
	        break ;
	    case suf_acc:
	        f = pip->have.sufacc ;
	        if (f) {
	            po = sufs[si] ;
	            vlp = (pip->sufs + si) ;
	        }
	        break ;
	    case suf_rej:
	        f = pip->have.sufrej ;
	        if (f) {
	            po = sufs[si] ;
	            vlp = (pip->sufs + si) ;
	        }
	        break ;
	    } /* end switch */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsufbegin: si=%u f=%u\n",si,f) ;
#endif

	    if (f) {
	        int n = paramopt_countvals(pop,po) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/procsufbegin: si=%u n=%u\n",si,n) ;
#endif

	        if (n > 0) {
	            PARAMOPT_CUR	cur ;
	            if ((rs = vecpstr_start(vlp,n,0,0)) >= 0) {
	                switch (si) {
	                case suf_req: 
	                    pip->open.sufreq = TRUE ; 
	                    break ;
	                case suf_acc: 
	                    pip->open.sufacc = TRUE ; 
	                    break ;
	                case suf_rej: 
	                    pip->open.sufrej = TRUE ; 
	                    break ;
	                } /* end switch */
	                if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {
	                    int	vl ;
	                    const char	*vp ;
	                    while (rs >= 0) {
	                        vl = paramopt_fetch(pop,po,&cur,&vp) ;
	                        if (vl == SR_NOTFOUND)
	                            break ;
	                        rs = vl ;
	                        if ((rs >= 0) && (vl > 0)) {
	                            rs = vecpstr_adduniq(vlp,vp,vl) ;
	                            if (rs < INT_MAX) c += 1 ;

#if	CF_DEBUG
	                            if (DEBUGLEVEL(3) && (rs < INT_MAX))
	                                debugprintf("main/procsufbegin: "
					    "suf=%t\n",
	                                    vp,vl) ;
#endif

	                        }
	                    } /* end while */
	                    paramopt_curend(pop,&cur) ;
	                } /* end if */
	                if (c > 0) {
	                    switch (si) {
	                    case suf_req:
	                        pip->f.sufreq = TRUE ;
	                        break ;
	                    case suf_acc:
	                        pip->f.sufacc = TRUE ;
	                        break ;
	                    case suf_rej:
	                        pip->f.sufrej = TRUE ;
	                        break ;
	                    } /* end switch */
	                } /* end if */
	            } /* end if */
	        } /* end if (n) */
	    } /* end if */

	} /* end for */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsufbegin: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procsufbegin) */


static int procsufend(pip)
struct proginfo	*pip ;
{
	VECPSTR	*slp ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;
	int	f ;


	for (i = 0 ; i < suf_overlast ; i += 1) {
	    slp = (pip->sufs + i) ;
	    f = FALSE ;
	    switch (i) {
	    case suf_req:
	        f = pip->open.sufreq ;
	        pip->open.sufreq = FALSE ;
	        break ;
	    case suf_acc:
	        f = pip->open.sufacc ;
	        pip->open.sufacc = FALSE ;
	        break ;
	    case suf_rej:
	        f = pip->open.sufrej ;
	        pip->open.sufrej = FALSE ;
	        break ;
	    } /* end switch */
	    if (f) {
	        rs1 = vecpstr_finish(slp) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (procsufend) */


static int procloadsuf(pip,si,ap,al)
struct proginfo	*pip ;
int		si ;
const char	*ap ;
int		al ;
{
	PARAMOPT	*pop = &pip->aparams ;

	int	rs = SR_OK ;
	int	c = 0 ;

	const char	*po ;
	const char	*var ;


#if	CF_DEBUGS
	if (si < suf_overlast)
	    debugprintf("main/procloadsuf: suf=%s(%u)\n",sufs[si],si) ;
#endif

	if (ap == NULL) goto ret0 ;

	switch (si) {
	case suf_req:
	    po = po_sufreq ;
	    var = VARSUFREQ ;
	    break ;
	case suf_acc:
	    po = po_sufacc ;
	    var = VARSA ;
	    break ;
	case suf_rej:
	    po = po_sufrej ;
	    var = VARSR ;
	    break ;
	default:
	    rs = SR_NOANODE ;
	    break ;
	} /* end switch */
	if (rs < 0)
	    goto ret0 ;

	if (strwcmp("-",ap,al) != 0) {
	    if (strwcmp("+",ap,al) == 0) {
	        ap = getenv(var) ;
	        al = -1 ;
	    }
	    if (ap != NULL) {
	        rs = paramopt_loads(pop,po,ap,al) ;
	        c = rs ;

#if	CF_DEBUGS
	        debugprintf("main/procloadsuf: paramopt_loads() rs=%d\n",rs) ;
#endif

	        if ((rs >= 0) && (c > 0)) {
	            switch (si) {
	            case suf_req: 
	                pip->have.sufreq = TRUE ; 
	                break ;
	            case suf_acc: 
	                pip->have.sufacc = TRUE ; 
	                break ;
	            case suf_rej: 
	                pip->have.sufrej = TRUE ; 
	                break ;
	            } /* end switch */
	        }
	    }
	} /* end if */

ret0:

#if	CF_DEBUGS
	debugprintf("main/procloadsuf: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procloadsuf) */


static int procprintfts(pip,po)
struct proginfo	*pip ;
const char	*po ;
{
	PARAMOPT	*pop = &pip->aparams ;
	PARAMOPT_CUR	cur ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	vl ;
	int	wlen = 0 ;

	const char	*vp ;


	if (pip->debuglevel == 0)
	    goto ret0 ;

	if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {

	    while ((vl = paramopt_fetch(pop,po,&cur,&vp)) >= 0) {

	        rs1 = bprintf(pip->efp,"%s: ft=%t\n", pip->progname,vp,vl) ;
	        if (rs1 > 0) wlen += rs1 ;

	    } /* end while */

	    paramopt_curend(pop,&cur) ;
	} /* end if */

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprintfts) */


static int procprintsufs(pip,po)
struct proginfo	*pip ;
const char	*po ;
{
	PARAMOPT	*pop = &pip->aparams ;
	PARAMOPT_CUR	cur ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	vl ;
	int	wlen = 0 ;

	const char	*vp ;


	if (pip->debuglevel == 0)
	    goto ret0 ;

	if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {

	    while ((vl = paramopt_fetch(pop,po,&cur,&vp)) >= 0) {

	        rs1 = bprintf(pip->efp,"%s: suf=%t\n",pip->progname,vp,vl) ;
	        if (rs1 > 0) wlen += rs1 ;

	    } /* end while */

	    paramopt_curend(pop,&cur) ;
	} /* end if */

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprintsufs) */


static int proclinkbegin(struct proginfo *pip)
{
	HDB		*dbp = &pip->links ;

	const int	n = 50 ;
	const int	at = 1 ;	/* use 'lookaside(3dam)' */

	int	rs = SR_OK ;


	rs = hdb_start(dbp,n,at,linkhash,linkcmp) ;
	pip->open.links = (rs >= 0) ;

	return rs ;
}
/* end subroutine (proclinkbegin) */


static int proclinkend(struct proginfo *pip)
{
	HDB		*dbp = &pip->links ;
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;

	LINKINFO	*lip ;

	int	rs = SR_OK ;
	int	rs1 ;


	if (! pip->open.links) goto ret0 ;

	pip->open.links = FALSE ;

/* delete this whole DB (for this entry) */

	if ((rs1 = hdb_curbegin(dbp,&cur)) >= 0) {

	    while (hdb_enum(dbp,&cur,&key,&val) >= 0) {
	        lip = (LINKINFO *) val.buf ;

	        if (lip != NULL) {
	            linkinfo_finish(lip) ;
	            uc_free(lip) ;
	        }

	    } /* end while */

	    hdb_curend(dbp,&cur) ;
	} /* end if */
	if (rs >= 0) rs = rs1 ;

	rs1 = hdb_finish(&pip->links) ;
	if (rs >= 0) rs = rs1 ;

ret0:
	return rs ;
}
/* end subroutine (proclinkend) */


static int proclinkadd(pip,ino,fp)
struct proginfo	 *pip ;
const char	 *fp ;
ino64_t		ino ;
{
	HDB		*dbp = &pip->links ;
	HDB_DATUM	key, val ;

	LINKINFO	*lip ;

	const int	size = sizeof(LINKINFO) ;

	int	rs ;


	if ((rs = uc_malloc(size,&lip)) >= 0) {
	    rs = linkinfo_start(lip,ino,fp) ;
	    if (rs >= 0) {
	        key.buf = &lip->ino ;
	        key.len = sizeof(ino64_t) ;
	        val.buf = lip ;
	        val.len = size ;
	        rs = hdb_store(dbp,key,val) ;
	        if (rs < 0)
	            linkinfo_finish(lip) ;
	    }
	    if (rs < 0)
	        uc_free(lip) ;
	} /* end if */

	return rs ;
}
/* end subroutine (proclinkadd) */


static int proclinkhave(struct proginfo *pip,ino64_t ino,LINKINFO **rpp)
{
	HDB		*dbp = &pip->links ;
	HDB_DATUM	key, val ;

	int	rs ;


	key.buf = &ino ;
	key.len = sizeof(ino64_t) ;
	rs = hdb_fetch(dbp,key,NULL,&val) ;

	if ((rs >= 0) && (rpp != NULL)) {
	    *rpp = (LINKINFO *) val.buf ;
	}

	return rs ;
}
/* end subroutine (proclinkhave) */


static int procsize(pip,name,sbp,ckp)
struct proginfo	*pip ;
const char	name[] ;
FSDIRTREE_STAT	*sbp ;
struct fileinfo	*ckp ;
{
	ULONG	bytes ;
	ULONG	size ;

	int	rs ;


	if (name == NULL) return SR_FAULT ;

	size = sbp->st_size ;

	bytes = pip->bytes ;
	bytes += (size % MEGABYTE) ;

	pip->bytes = (bytes % MEGABYTE) ;
	pip->megabytes += (bytes / MEGABYTE) ;
	pip->megabytes += (size / MEGABYTE) ;

	rs = (int) (size & INT_MAX) ;
	return rs ;
}
/* end subroutine (procsize) */


static int proclink(pip,name,sbp,ckp)
struct proginfo	*pip ;
const char	name[] ;
FSDIRTREE_STAT	*sbp ;
struct fileinfo	*ckp ;
{
	FSDIRTREE_STAT	tsb ;

	const mode_t	dm = 0775 ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	w = 0 ;
	int	f_dolink = TRUE ;

	char	tarfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/proclink: name=%s\n",name) ;
#endif

	if (sbp->st_dev != pip->tarstat.st_dev) {
	    pip->c_linkerr += 1 ;
	    if (! pip->f.nostop) rs = SR_XDEV ;
	    goto ret0 ;
	}

	rs = mkpath2(tarfname,pip->tardname,name) ;
	if (rs < 0)
	    goto ret0 ;

	rs1 = fsdirtreestat(tarfname,1,&tsb) ; /* LSTAT */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/proclink: tarfname=%s rs=%d\n",name,rs) ;
#endif

	if (rs1 >= 0) {
	    if (! S_ISDIR(sbp->st_mode)) {
	        int	f = TRUE ;
	        f = f && (tsb.st_dev == sbp->st_dev) ;
	        f = f && (tsb.st_ino == sbp->st_ino) ;
	        if (! f) {
	            if (S_ISDIR(tsb.st_mode)) {
	                w = 1 ;
	                rs = removes(tarfname) ;
	            } else {
	                w = 2 ;
	                rs = uc_unlink(tarfname) ;
	            }
	        } else
	            f_dolink = FALSE ;
	    } else {
	        if (! S_ISDIR(tsb.st_mode)) {
	            w = 3 ;
	            rs = uc_unlink(tarfname) ;
	        } else
	            f_dolink = FALSE ;
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/proclink: mid rs=%d f_dolink=%u\n",
	        rs,f_dolink) ;
#endif

	if ((rs >= 0) && f_dolink) {
	    if (! S_ISDIR(sbp->st_mode)) {
	        w = 4 ;
	        rs = uc_link(name,tarfname) ;
	        if ((rs == SR_NOTDIR) || (rs == SR_NOENT)) {
	            w = 5 ;
	            rs = mkpdirs(tarfname,dm) ;
	            if (rs >= 0) {
	                w = 6 ;
	                rs = uc_link(name,tarfname) ;
	            }
	        }
	    } else {
	        w = 7 ;
	        rs = uc_mkdir(tarfname,dm) ;
	        if ((rs == SR_NOTDIR) || (rs == SR_NOENT)) {
	            w = 8 ;
	            rs = mkpdirs(tarfname,dm) ;
	            if (rs >= 0) {
	                w = 9 ;
	                rs = uc_mkdir(tarfname,dm) ;
	            }
	        }
	    }
	} /* end if */

	if ((rs == SR_EXIST) && (! pip->f.quiet))
	    bprintf(pip->efp,"%s: exists=%u\n",pip->progname,w) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/proclink: ret rs=%d f_link=%u\n",rs,f_dolink) ;
#endif

	return (rs >= 0) ? f_dolink : rs ;
}
/* end subroutine (proclink) */


static int procsync(pip,name,sbp,ckp)
struct proginfo	*pip ;
const char	name[] ;
FSDIRTREE_STAT	*sbp ;
struct fileinfo	*ckp ;
{
	LINKINFO	*lip ;

	int	rs = SR_OK ;
	int	rs1 ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsync: name=%s\n",name) ;
#endif

/* do we have a link to this file already? */

	rs1 = proclinkhave(pip,sbp->st_ino,&lip) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsync: proclinkhave() rs=%d\n",rs1) ;
#endif

	if (rs1 >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsync: procsynclink()\n") ;
#endif

	    rs = procsynclink(pip,name,sbp,lip) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsync: procsynclink() rs=%d\n",rs) ;
#endif

	    if ((rs == SR_NOENT) || (rs == SR_XDEV))
	        rs = procsyncer(pip,name,sbp) ;

	} else if (rs1 == SR_NOTFOUND) {

	    rs = procsyncer(pip,name,sbp) ;

	} else
	    rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsync: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procsync) */


static int procsynclink(pip,name,sbp,lip)
struct proginfo	*pip ;
const char	name[] ;
FSDIRTREE_STAT	*sbp ;
LINKINFO	*lip ;
{
	FSDIRTREE_STAT	ssb, dsb ;

	const mode_t	dm = DMODE ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	w = 0 ;
	int	f_dolink = TRUE ;

	char	srcfname[MAXPATHLEN + 1] ;
	char	dstfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsynclink: name=%s\n",name) ;
#endif

	rs = mkpath2(srcfname,pip->tardname,lip->fname) ;
	if (rs < 0) goto ret0 ;

	rs = mkpath2(dstfname,pip->tardname,name) ;
	if (rs < 0) goto ret0 ;

	rs = fsdirtreestat(srcfname,1,&ssb) ; /* LSTAT */
	if (rs < 0) goto ret0 ;

	rs1 = fsdirtreestat(dstfname,1,&dsb) ; /* LSTAT */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsynclink: dstfname=%s rs=%d\n",name,rs) ;
#endif

	if (rs1 >= 0) {
	    if (! S_ISDIR(ssb.st_mode)) {
	        int	f = TRUE ;
	        f = f && (ssb.st_dev == dsb.st_dev) ;
	        f = f && (ssb.st_ino == dsb.st_ino) ;
	        if (! f) {
	            if (S_ISDIR(dsb.st_mode)) {
	                w = 1 ;
	                rs = removes(dstfname) ;
	            } else {
	                w = 2 ;
	                rs = uc_unlink(dstfname) ;
	            }
	        } else
	            f_dolink = FALSE ;
	    } else {
	        if (! S_ISDIR(dsb.st_mode)) {
	            w = 3 ;
	            rs = uc_unlink(dstfname) ;
	        } else
	            f_dolink = FALSE ;
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsynclink: mid rs=%d f_dolink=%u\n",
	        rs,f_dolink) ;
#endif

	if ((rs >= 0) && f_dolink) {
	    if (! S_ISDIR(ssb.st_mode)) {
	        w = 4 ;
	        rs = uc_link(srcfname,dstfname) ;
	        if ((rs == SR_NOTDIR) || (rs == SR_NOENT)) {
	            w = 5 ;
	            rs = mkpdirs(dstfname,dm) ;
	            if (rs >= 0) {
	                w = 6 ;
	                rs = uc_link(srcfname,dstfname) ;
	            }
	        }
	    } else {
	        w = 7 ;
	        rs = uc_mkdir(dstfname,dm) ;
	        if ((rs == SR_NOTDIR) || (rs == SR_NOENT)) {
	            w = 8 ;
	            rs = mkpdirs(dstfname,dm) ;
	            if (rs >= 0) {
	                w = 9 ;
	                rs = uc_mkdir(dstfname,dm) ;
	            }
	        }
	    }
	} /* end if */

	if ((rs == SR_EXIST) && (! pip->f.quiet))
	    bprintf(pip->efp,"%s: exists=%u\n",pip->progname,w) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsynclink: ret rs=%d f_link=%u\n",
	        rs,f_dolink) ;
#endif

	return (rs >= 0) ? f_dolink : rs ;
}
/* end subroutine (procsynclink) */


static int procsyncer(pip,name,sbp)
struct proginfo	*pip ;
const char	name[] ;
FSDIRTREE_STAT	*sbp ;
{
	SIGBLOCK	blocker ;

	int	rs = SR_OK ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer: name=%s\n",name) ;
#endif

	if ((! S_ISDIR(sbp->st_mode)) && (sbp->st_nlink > 1)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer: proclinkadd() ino=%llu\n",
	            sbp->st_ino) ;
#endif

	    rs = proclinkadd(pip,sbp->st_ino,name) ;
	}
	if (rs < 0) goto ret0 ;

	if ((rs = sigblock_start(&blocker,NULL)) >= 0) {

	    if (S_ISREG(sbp->st_mode)) {

	        rs = procsyncer_reg(pip,name,sbp) ;

	    } else if (S_ISDIR(sbp->st_mode)) {

	        rs = procsyncer_dir(pip,name,sbp) ;

	    } else if (S_ISLNK(sbp->st_mode)) {

	        rs = procsyncer_lnk(pip,name,sbp) ;

	    } else if (S_ISFIFO(sbp->st_mode)) {

	        rs = procsyncer_fifo(pip,name,sbp) ;

	    } else if (S_ISSOCK(sbp->st_mode)) {

	        rs = procsyncer_sock(pip,name,sbp) ;

	    } /* end if */

	    sigblock_finish(&blocker) ;
	} /* end if (blocking signals) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procsyncer) */


static int procsyncer_reg(pip,name,sbp)
struct proginfo	*pip ;
const char	name[] ;
FSDIRTREE_STAT	*sbp ;
{
	struct utimbuf	ut ;

	struct ustat	dsb ;

	size_t		fsize = 0 ;

	const mode_t	dm = DMODE ;
	const mode_t	nm = (sbp->st_mode & (~ S_IFMT)) | 0600 ;

	uid_t	duid = -1 ;

	int	rs = SR_OK ;
	int	of ;
	int	f_create = FALSE ;
	int	f_update = FALSE ;
	int	f_updated = FALSE ;

	char	dstfname[MAXPATHLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procsyncer_reg: tardname=%s\n",pip->tardname) ;
	    debugprintf("main/procsyncer_reg: name=%s\n",name) ;
	}
#endif

	ut.actime = sbp->st_atime ;
	ut.modtime = sbp->st_mtime ;

/* exit on large files! */

	if (sbp->st_size > INT_MAX) goto ret0 ;

	rs = mkpath2(dstfname,pip->tardname,name) ;
	if (rs < 0) goto ret0 ;

/* continue */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer_reg: dstfname=%s\n",dstfname) ;
#endif

	if ((rs = u_lstat(dstfname,&dsb)) >= 0) {

	    if (S_ISREG(dsb.st_mode)) {
	        int	f = FALSE ;
	        duid = dsb.st_uid ;
	        fsize = (size_t) dsb.st_size ;
	        f = f || (sbp->st_size != dsb.st_size) ;
	        f = f || (sbp->st_mtime > dsb.st_mtime) ;
	        if (f) {
	            f_update = TRUE ;
	        } /* end if */
	    } else {
	        f_create = TRUE ;
	        if (S_ISDIR(dsb.st_mode)) {
	            rs = removes(dstfname) ;
	        } else
	            rs = uc_unlink(dstfname) ;
	    }

	} else if (rs < 0) {

	    f_create = TRUE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_reg: dst u_lstat() rs=%d\n",rs) ;
#endif

	    if (rs == SR_NOTDIR) {
	        int		dnl ;
	        const char	*dnp ;
	        dnl = sfdirname(dstfname,-1,&dnp) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/procsyncer_reg: dst dname=%t\n",dnp,dnl) ;
#endif
	        rs = SR_OK ;
	        if (dnl > 0) {
	            rs = mkpath1w(tmpfname,dnp,dnl) ;
	            if (rs >= 0)
	                rs = uc_unlink(tmpfname) ;
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_reg: no entry source rs=%d\n",
	            rs) ;
#endif

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer_reg: source rs=%d "
	        "f_update=%u f_create=%u\n",
	        rs,f_update,f_create) ;
#endif

	if (rs < 0) goto ret0 ;
	if (! (f_update || f_create)) goto ret0 ;

/* create any needed directories (as necessary) */

	if (f_create) {
	    int		dnl ;
	    const char	*dnp ;
	    dnl = sfdirname(dstfname,-1,&dnp) ;
	    if (dnl > 0) {
	        rs = mkpath1w(tmpfname,dnp,dnl) ;
	        if (rs >= 0) {
	            struct ustat	sb ;
	            int	rs1 = u_lstat(tmpfname,&sb) ;
	            int	f = FALSE ;
	            if (rs1 >= 0) {
	                if (S_ISLNK(sb.st_mode)) {
	                    rs1 = u_stat(tmpfname,&sb) ;
	                    f = ((rs1 < 0) || (! S_ISDIR(sb.st_mode))) ;
	                } else if (! S_ISDIR(sb.st_mode))
	                    f = TRUE ;
	                if (f) {
	                    rs = uc_unlink(tmpfname) ;
#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("main/procsyncer_reg: "
					"uc_unlink() rs=%d\n",
	                            rs) ;
#endif
	                }
	            } else
	                f = TRUE ;
	            if (f)
	                rs = mkdirs(tmpfname,dm) ;
	        }
	    }
	}
	if (rs < 0) goto ret0 ;

/* update (or create) the target file */

	of = O_WRONLY ;
	if (f_create) of |= O_CREAT ;

	rs = u_open(dstfname,of,nm) ;
	if ((rs == SR_ACCESS) && (! f_create)) {
	    int		dnl ;
	    const char	*dnp ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_reg: SR_ACCESS\n") ;
#endif
	    dnl = sfdirname(dstfname,-1,&dnp) ;
	    if ((dnl > 0) && ((rs = mkpath1w(tmpfname,dnp,dnl)) >= 0)) {
	        if ((rs = u_access(tmpfname,W_OK)) >= 0) {
	            f_create = TRUE ;
	            rs = uc_unlink(dstfname) ;
	            of |= O_CREAT ;
	            if (rs >= 0)
	                rs = u_open(dstfname,of,nm) ;
	        }
	    }
	}
	if (rs >= 0) { /* opened */
	    int	dfd = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_reg: u_open() rs=%d\n",rs) ;
#endif

	    if (f_create) {
	        rs = uc_fminmod(dfd,nm) ;
	    } /* end if (needed to be created) */

/* do we need to UPDATE the destination? */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_reg: "
	            "need update? rs=%d f_update=%d\n",
	            rs,f_update) ;
#endif

	    if (rs >= 0) {
	        f_updated = TRUE ;
	        if ((rs = u_open(name,O_RDONLY,0666)) >= 0) {
	            int	len ;
	            int	sfd = rs ;
	            rs = uc_copy(sfd,dfd,-1) ;
	            len = rs ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("main/procsyncer_reg: "
				"fsize=%u uc_copy() rs=%d\n",
	                    fsize,rs) ;
#endif
	            if ((rs >= 0) && (len != INT_MAX) && (len < fsize)) {
	                offset_t	uoff = len ;
	                rs = uc_ftruncate(dfd,uoff) ;
	            }
	            u_close(sfd) ;
	        if (rs >= 0) {
	            int	f_utime = FALSE ;
	            f_utime = f_utime || (duid < 0) ;
	            f_utime = f_utime || (duid == pip->euid) ;
	            f_utime = f_utime || (pip->euid == 0) ;
	            if (f_utime) {
#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("main/procsyncer_reg: utime()\n") ;
#endif
	                u_utime(dstfname,&ut) ;
	            }
	        }
	        } else if (rs == SR_NOENT) {
			rs = SR_OK ;
	                uc_unlink(dstfname) ;
		} /* end if (open source) */
	    } /* end if (update was needed) */

	    u_close(dfd) ;
	} /* end if (destination file opened) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer_reg: ret rs=%d f_updated=%u\n",
	        rs,f_updated) ;
#endif

	return (rs >= 0) ? f_updated : rs ;
}
/* end subroutine (procsyncer_reg) */


static int procsyncer_dir(pip,name,sbp)
struct proginfo	*pip ;
const char	name[] ;
FSDIRTREE_STAT	*sbp ;
{
	struct utimbuf	ut ;

	struct ustat	dsb ;

	const mode_t	nm = (sbp->st_mode & (~ S_IFMT)) | DMODE ;

	uid_t	duid = -1 ;

	int	rs = SR_OK ;
	int	f_create = FALSE ;
	int	f_update = FALSE ;
	int	f_mode = FALSE ;
	int	f_mtime = FALSE ;
	int	f_updated = FALSE ;

	char	dstfname[MAXPATHLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;


	ut.actime = sbp->st_atime ;
	ut.modtime = sbp->st_mtime ;

	rs = mkpath2(dstfname,pip->tardname,name) ;
	if (rs < 0) goto ret0 ;

/* continue */

	if ((rs = u_lstat(dstfname,&dsb)) >= 0) {

	    duid = dsb.st_uid ;
	    if (S_ISDIR(dsb.st_mode)) {
	        f_mtime = (dsb.st_mtime != sbp->st_mtime) ;
	        f_mode = (dsb.st_mode != nm) ;
	        if (f_mode || f_mtime) {
	            f_update = TRUE ;
	        }
	    } else {
	        f_create = TRUE ;
	        rs = uc_unlink(dstfname) ;
	    }

	} else if (rs < 0) {
	    f_create = TRUE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_dir: dst u_lstat() rs=%d\n",rs) ;
#endif

	    if (rs == SR_NOTDIR) {
	        int		dnl ;
	        const char	*dnp ;
	        dnl = sfdirname(dstfname,-1,&dnp) ;
	        rs = SR_OK ;
	        if (dnl > 0) {
	            rs = mkpath1w(tmpfname,dnp,dnl) ;
	            if (rs >= 0)
	                rs = uc_unlink(tmpfname) ;
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_dir: no entry for source \n") ;
#endif

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer_dir: source rs=%d "
	        "f_update=%u f_create=%u\n",
	        rs,f_update,f_create) ;
#endif

	if (rs < 0) goto ret0 ;
	if (! (f_update || f_create)) goto ret0 ;

/* create any needed directories (as necessary) */

	if (f_create) {
	    int		dnl ;
	    const char	*dnp ;
	    dnl = sfdirname(dstfname,-1,&dnp) ;
	    if (dnl > 0) {
	        rs = mkpath1w(tmpfname,dnp,dnl) ;
	        if (rs >= 0) {
	            struct ustat	sb ;
	            int	rs1 = u_lstat(tmpfname,&sb) ;
	            int	f = FALSE ;
	            if (rs1 >= 0) {
	                if (S_ISLNK(sb.st_mode)) {
	                    rs1 = u_stat(tmpfname,&sb) ;
	                    f = ((rs1 < 0) || (! S_ISDIR(sb.st_mode))) ;
	                } else if (! S_ISDIR(sb.st_mode))
	                    f = TRUE ;
	                if (f)
	                    rs = uc_unlink(tmpfname) ;
	            } else
	                f = TRUE ;
	            if (f)
	                rs = mkdirs(tmpfname,nm) ;
	        }
	    }
	}
	if (rs < 0) goto ret0 ;

/* update (or create) the target file */

	if (f_create)
	    rs = mkdir(dstfname,nm) ;
	if ((rs >= 0) && ((duid < 0) || (duid == pip->euid))) {
	    f_updated = TRUE ;
	    if (f_mode) rs = u_chmod(dstfname,nm) ;
	    if ((rs >= 0) && f_mtime) u_utime(dstfname,&ut) ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer_dir: ret rs=%d f_updated=%u\n",
	        rs,f_updated) ;
#endif

	return (rs >= 0) ? f_updated : rs ;
}
/* end subroutine (procsyncer_dir) */


static int procsyncer_lnk(pip,name,sbp)
struct proginfo	*pip ;
const char	name[] ;
FSDIRTREE_STAT	*sbp ;
{
	struct ustat	dsb ;

	const mode_t	dm = DMODE ;

	int	rs = SR_OK ;
	int	f_create = FALSE ;
	int	f_update = FALSE ;
	int	f_updated = FALSE ;

	char	dstfname[MAXPATHLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	dstlink[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procsyncer_lnk: tardname=%s\n",pip->tardname) ;
	    debugprintf("main/procsyncer_lnk: name=%s\n",name) ;
	}
#endif

	rs = u_readlink(name,dstlink,MAXPATHLEN) ;
	if (rs < 0) goto ret0 ;

	rs = mkpath2(dstfname,pip->tardname,name) ;
	if (rs < 0) goto ret0 ;

/* continue */

	if ((rs = u_lstat(dstfname,&dsb)) >= 0) {

	    if (S_ISLNK(dsb.st_mode)) {
	        int	f = TRUE ;
	        f = f && (dsb.st_size == sbp->st_size) ;
	        if (f) {
	            rs = u_readlink(dstfname,tmpfname,MAXPATHLEN) ;
	            if (rs >= 0) f = (strcmp(dstlink,tmpfname) == 0) ;
	        }
	        if (! f) {
	            f_update = TRUE ;
	        } /* end if */
	    } else {
	        f_create = TRUE ;
	        if (S_ISDIR(dsb.st_mode)) {
	            rs = removes(dstfname) ;
	        } else
	            rs = uc_unlink(dstfname) ;
	    }

	} else if (rs < 0) {
	    f_create = TRUE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_lnk: dst u_lstat() rs=%d\n",rs) ;
#endif

	    if (rs == SR_NOTDIR) {
	        int		dnl ;
	        const char	*dnp ;
	        dnl = sfdirname(dstfname,-1,&dnp) ;
	        rs = SR_OK ;
	        if (dnl > 0) {
	            rs = mkpath1w(tmpfname,dnp,dnl) ;
	            if (rs >= 0)
	                rs = uc_unlink(tmpfname) ;
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_lnk: no entry for source \n") ;
#endif

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer_lnk: source rs=%d "
	        "f_update=%u f_create=%u\n",
	        rs,f_update,f_create) ;
#endif

	if (rs < 0) goto ret0 ;
	if (! (f_update || f_create)) goto ret0 ;

/* create any needed directories (as necessary) */

	if (f_create) {
	    int		dnl ;
	    const char	*dnp ;
	    dnl = sfdirname(dstfname,-1,&dnp) ;
	    if (dnl > 0) {
	        rs = mkpath1w(tmpfname,dnp,dnl) ;
	        if (rs >= 0) {
	            struct ustat	sb ;
	            int	rs1 = u_lstat(tmpfname,&sb) ;
	            int	f = FALSE ;
	            if (rs1 >= 0) {
	                if (S_ISLNK(sb.st_mode)) {
	                    rs1 = u_stat(tmpfname,&sb) ;
	                    f = ((rs1 < 0) || (! S_ISDIR(sb.st_mode))) ;
	                } else if (! S_ISDIR(sb.st_mode))
	                    f = TRUE ;
	                if (f)
	                    rs = uc_unlink(tmpfname) ;
	            } else
	                f = TRUE ;
	            if (f)
	                rs = mkdirs(tmpfname,dm) ;
	        }
	    }
	}
	if (rs < 0) goto ret0 ;

/* update (or create) the target file */

	if (rs >= 0) {
	    f_updated = TRUE ;
	    if ((! f_create) && f_update) {
	        rs = uc_unlink(dstfname) ;
	    }
	    if (rs >= 0)
	        rs = u_symlink(dstlink,dstfname) ;
	} /* end if (update was needed) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer_lnk: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_updated : rs ;
}
/* end subroutine (procsyncer_lnk) */


static int procsyncer_fifo(pip,name,sbp)
struct proginfo	*pip ;
const char	name[] ;
FSDIRTREE_STAT	*sbp ;
{
	int	rs = SR_OK ;


	if (pip == NULL) return SR_FAULT ;


	return rs ;
}
/* end subroutine (procsyncer_fifo) */


static int procsyncer_sock(pip,name,sbp)
struct proginfo	*pip ;
const char	name[] ;
FSDIRTREE_STAT	*sbp ;
{
	int	rs = SR_OK ;


	return rs ;
}
/* end subroutine (procsyncer_sock) */


static int procrm(pip,name,sbp,ckp)
struct proginfo	*pip ;
const char	name[] ;
FSDIRTREE_STAT	*sbp ;
struct fileinfo	*ckp ;
{
	int	rs = SR_OK ;


	if (pip == NULL) return SR_FAULT ;

	if (pip->delay > 0) {
	    if (S_ISDIR(sbp->st_mode)) {
	        rs = removes(name) ;
	    } else
	        rs = unlinkd(name,pip->delay) ;
	} else {
	    if (S_ISDIR(sbp->st_mode)) {
	        rs = removes(name) ;
	    } else
	        rs = uc_unlink(name) ;
	} /* end if (delay or not) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procrm: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procrm) */


static int linkinfo_start(LINKINFO *lip,ino64_t ino,const char *fp)
{
	int	rs ;

	const char	*cp ;


	lip->ino = ino ;
	rs = uc_mallocstrw(fp,-1,&cp) ;
	if (rs >= 0) lip->fname = cp ;

	return rs ;
}
/* end subroutine (linkinfo_start) */


static int linkinfo_finish(LINKINFO *lip)
{


	if (lip->fname != NULL) {
	    uc_free(lip->fname) ;
	    lip->fname = NULL ;
	}

	return SR_OK ;
}
/* end subroutine (linkinfo_finish) */


/* make *parent* directories as needed */
static int mkpdirs(const char *tarfname,mode_t dm)
{
	int	rs = SR_OK ;
	int	dl ;

	const char	*dp ;

	dl = sfdirname(tarfname,-1,&dp) ;
	if (dl > 0) {
	    char	dname[MAXPATHLEN + 1] ;
	    rs = mkpath1w(dname,dp,dl) ;
	    if (rs >= 0) {
	        uc_unlink(dname) ; /* just a little added help */
	        rs = mkdirs(dname,dm) ;
	    }
	} else
	    rs = SR_NOENT ;

	return rs ;
}
/* end subroutine (mkpdirs) */


static int istermrs(rs)
int	rs ;
{
	int	i ;
	int	f = FALSE ;


	for (i = 0 ; termrs[i] != 0 ; i += 1) {
	    f = (rs == termrs[i]) ;
	    if (f)
	        break ;
	} /* end if */

	return f ;
}
/* end subroutine (istermrs) */


static int linkcmp(struct linkinfo *e1p,struct linkinfo *e2p,int len)
{
	int64_t	d ;

	int	rc = 0 ;


	d = (e1p->ino - e2p->ino) ;
	if (d > 0) rc = 1 ;
	else if (d < 0) rc = -1 ;

	return rc ;
}
/* end subroutine (linkcmp) */


static uint linkhash(const void *vp,int vl)
{
	uint	h = 0 ;

	ushort	*sa = (ushort *) vp ;


	h = h ^ ((sa[1] << 16) | sa[0]) ;
	h = h ^ ((sa[0] << 16) | sa[1]) ;
	if (vl > sizeof(uint)) {
	    h = h ^ ((sa[3] << 16) | sa[2]) ;
	    h = h ^ ((sa[2] << 16) | sa[3]) ;
	}

#if	CF_DEBUGS
	debugprintf("linkhash: h=%08x\n",h) ;
#endif

	return h ;
}
/* end subroutine (linkhash) */



