/* b_chacl */

/* change ACLs (on OSes that have them) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocations */
#define	CF_DEFMAXACLS	1		/* use MAXACLS as default */
#define	CF_NOSOCKETS	1		/* do not process sockets */


/* revision history:

	= 2005-02-24, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2005 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ chacl <acl(s)> [<file(s) ...>]


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

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
#include	<sys/acl.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<estrings.h>
#include	<field.h>
#include	<vecobj.h>
#include	<fsdirtree.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_chacl.h"
#include	"defs.h"
#include	"aclinfo.h"


/* local defines */

#ifndef	ABUFLEN
#ifdef	ALIASNAMELEN
#define	ABUFLEN		ALIASNAMELEN
#else
#define	ABUFLEN		64
#endif
#endif

#ifndef	VBUFLEN
#ifdef	MAILADDRLEN
#define	VBUFLEN		MAILADDRLEN
#else
#define	VBUFLEN		2048
#endif
#endif

#ifdef	MAX_ACL_ENTRIES
#define	MAXACLS		MAX_ACL_ENTRIES
#else
#define	MAXACLS		1024
#endif

#ifndef	IDNAMELEN
#define	IDNAMELEN	LOGNAMELEN
#endif

#if	defined(USER_OBJ) && (! defined(ACLTYPE_USEROBJ))
#define	ACLTYPE_USEROBJ		USER_OBJ
#define	ACLTYPE_USER		USER
#define	ACLTYPE_GROUPOBJ	GROUP_OBJ
#define	ACLTYPE_GROUP		GROUP
#define	ACLTYPE_OTHEROBJ	OTHER_OBJ
#define	ACLTYPE_CLASSOBJ	CLASS_OBJ
#define	ACLTYPE_DEFUSEROBJ	DEF_USER_OBJ
#define	ACLTYPE_DEFUSER		DEF_USER
#define	ACLTYPE_DEFGROUPOBJ	DEF_GROUP_OBJ
#define	ACLTYPE_DEFGROUP	DEF_GROUP
#define	ACLTYPE_DEFOTHEROBJ	DEF_OTHER_OBJ
#define	ACLTYPE_DEFCLASSOBJ	DEF_CLASS_OBJ
#endif

#undef	S_OK
#define	S_OK		(1<<3)

#undef	T_OK
#define	T_OK		(1<<4)

#define	PO_TYPE		"type"
#define	PO_SUFFIX	"suffix"

#define	LOCINFO		struct locinfo
#define	LOCINFO_FT	struct locinfo_ftypes
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	fsdirtreestat(cchar *,int,FSDIRTREE_STAT *) ;
extern int	getuid_user(cchar *,int) ;
extern int	getgid_group(cchar *,int) ;
extern int	hasalldig(cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;
extern int	isInvalid(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strdcp1w(char *,int,cchar *,int) ;
extern char	*strnrchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_ftypes {
	uint		f:1 ;		/* regular */
	uint		d:1 ;		/* directory */
	uint		b:1 ;		/* block */
	uint		c:1 ;		/* character */
	uint		p:1 ;		/* pipe or FIFO */
	uint		n:1 ;		/* name */
	uint		l:1 ;		/* symbolic link */
	uint		s:1 ;		/* socket */
	uint		D:1 ;		/* door */
} ;

struct locinfo_flags {
	uint		ftypes:1 ;
	uint		fsuffixes:1 ;
	uint		nhf:1 ;		/* "no hidden files" */
	uint		delempty:1 ;
	uint		recurse:1 ;
	uint		nostop:1 ;
	uint		follow:1 ;
	uint		min:1 ;
	uint		max:1 ;
	uint		maskcalc:1 ;
	uint		acls:1 ;
	uint		suid:1 ;
	uint		sgid:1 ;
} ;

struct locinfo {
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	vecobj		acls ;
	aclent_t	*aclbuf ;	/* new ACLs being applied */
	PROGINFO	*pip ;
	LOCINFO_FL	have, f, final ;
	LOCINFO_FT	ft ;
	gid_t		gid_owner, gid_new ;
	uid_t		uid_owner, uid_new ;
	int		naclbuf ;	/* number of new ACLs */
	int		entries ;	/* total count */
	int		changed ;	/* total count */
	int		errored ;	/* total count */
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *) ;
static int	procacls(PROGINFO *,cchar *) ;
static int	procacl(PROGINFO *,cchar *,int) ;
static int	procname(PROGINFO *,cchar *) ;
static int	procnamer(PROGINFO *,cchar *) ;
static int	checkname(PROGINFO *,cchar *,FSDIRTREE_STAT *) ;
static int	checkowner(PROGINFO *,FSDIRTREE_STAT *,cchar *) ;
static int	procoutverbose(PROGINFO *,LOCINFO *,cchar *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_procopts(LOCINFO *) ;
static int	locinfo_ftypes(LOCINFO *) ;
static int	locinfo_isfsuffix(LOCINFO *,cchar *) ;
static int	locinfo_isftype(LOCINFO *,cchar *,FSDIRTREE_STAT *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_addacl(LOCINFO *,aclinfo_t *) ;
static int	locinfo_mksol(LOCINFO *) ;

static int	aclents_match(aclent_t *,int,aclinfo_t *) ;
static int	aclents_minmax(aclent_t *,int,int *,int *) ;
static int	aclents_compact(aclent_t *,int) ;
static int	aclents_maskmat(aclent_t *,int) ;
static int	aclents_maskneed(aclent_t *,int) ;
static int	aclents_defmaskneed(aclent_t *,int) ;

#if	CF_DEBUG || CF_DEBUGS
static int aclents_print(aclent_t *,int) ;
#endif

static int	aclent_empty(aclent_t *) ;
static int	aclent_idtype(aclent_t *) ;

static int	parseperms(cchar *,int) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"min",
	"max",
	"mm",
	"mc",
	"suid",
	"sgid",
	"cu",
	"cg",
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
	argopt_min,
	argopt_max,
	argopt_mm,
	argopt_mc,
	argopt_suid,
	argopt_sgid,
	argopt_cu,
	argopt_cg,
	argopt_overlast
} ;

static const PIVARS	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
} ;

static const MAPEX	mapexs[] = {
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
	"nhf",
	"suid",
	"sgid",
	NULL
} ;

enum progopts {
	progopt_nhf,
	progopt_suid,
	progopt_sgid,
	progopt_overlast
} ;

enum aclops {
	aclop_add,
	aclop_subtract,
	aclop_overlast
} ;

static const char	*ftypes[] = {
	"file",
	"directory",
	"block",
	"character",
	"pipe",
	"fifo",
	"socket",
	"link",
	"door",
	"regular",
	NULL
} ;

enum ftypes {
	ftype_file,
	ftype_directory,
	ftype_block,
	ftype_character,
	ftype_pipe,
	ftype_fifo,
	ftype_socket,
	ftype_link,
	ftype_door,
	ftype_regular,
	ftype_overlast
} ;

static const uchar	aclterms[] = {
	0x00, 0x3E, 0x00, 0x00,
	0x01, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int b_chacl(int argc,cchar *argv[],void *contextp)
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    cchar	**envv = (cchar **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_chacl) */


int p_chacl(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_chacl) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	SHIO		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos, ai_continue ;
	int		rs, rs1 ;
	int		cl ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*aclspec = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("chacl: starting DFD=%d\n",rs) ;
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

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;

	pip->lip = lip ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

	lip->f.min = DEFMINMAX ;
	lip->f.max = DEFMINMAX ;
	lip->f.maskcalc = DEFMASKCALC ;

/* argumuments */

	if (rs >= 0) rs = bits_start(&pargs,0) ;
	if (rs < 0) goto badpargs ;

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

	            argval = (argp+1) ;

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

	                case argopt_help:
	                    f_help = TRUE ;
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

	                case argopt_min:
	                    lip->f.min = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.min = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_max:
	                    lip->f.max = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.max = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_mm:
	                    lip->f.min = TRUE ;
	                    lip->f.max = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.min = (rs > 0) ;
	                            lip->f.max = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_mc:
	                    lip->f.maskcalc = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.maskcalc = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_suid:
	                    lip->have.suid = TRUE ;
	                    lip->final.suid = TRUE ;
	                    lip->f.suid = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.suid = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_sgid:
	                    lip->have.sgid = TRUE ;
	                    lip->final.sgid = TRUE ;
	                    lip->f.sgid = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.sgid = (rs > 0) ;
	                        }
	                    }
	                    break ;

/* change to new user */
	                    case argopt_cu:
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = getuid_user(argp,argl) ;
	                                lip->uid_new = rs ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* change to new user */
	                    case argopt_cg:
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = getgid_group(argp,argl) ;
	                                lip->gid_new = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
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

/* program-root */
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

/* delete empty ACLs */
	                    case 'd':
	                        lip->f.delempty = TRUE ;
	                        break ;

/* follow symbolic links */
	                    case 'f':
	                        lip->f.follow = TRUE ;
	                        break ;

	                    case 'g':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = getgid_group(argp,argl) ;
	                                lip->gid_owner = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* no stopping on error */
	                    case 'n':
	                    case 'c':
	                        lip->f.nostop = TRUE ;
	                        break ;

/* recurse */
	                    case 'r':
	                        lip->f.recurse = TRUE ;
	                        break ;

/* program options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					KEYOPT	*kop = &lip->akopts ;
	                                rs = keyopt_loads(kop,argp,argl) ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* file suffixes */
	                    case 's':
	                        cp = NULL ;
	                        cl = -1 ;
	                        lip->f.fsuffixes = TRUE ;
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                cp = argp ;
	                                cl = argl ;
	                            }
	                            if (cp != NULL) {
	                                PARAMOPT	*plp = &lip->aparams ;
	                                cchar		*po = PO_SUFFIX ;
	                                rs = paramopt_loads(plp,po,cp,cl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* file types */
	                    case 't':
	                        cp = NULL ;
	                        cl = -1 ;
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                cp = argp ;
	                                cl = argl ;
	                            }
	                            if (cp != NULL) {
	                                PARAMOPT	*plp = &lip->aparams ;
	                                cchar		*po = PO_TYPE ;
	                                rs = paramopt_loads(plp,po,cp,cl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* select on this user (UID) */
	                    case 'u':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	    				rs = getuid_user(argp,argl) ;
	        			lip->uid_owner = rs ;
	    			    }
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

	    } /* end if (key letter-word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = shio_open(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    shio_control(&errfile,SHIO_CSETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0) goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("chacl: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n",
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
	    shio_printf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help) {
#if	CF_SFIO
	    printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif
	}

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* some initialization */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if ((rs = locinfo_procopts(lip)) >= 0) {
	    rs = locinfo_ftypes(lip) ;
	}

	ai_continue = 1 ;
	if (rs >= 0) {
	    for (ai = ai_continue ; ai < argc ; ai += 1) {
	        f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	        f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            aclspec = argv[ai] ;
	            ai_continue = (ai + 1) ;
	            break ;
	        }
	    } /* end for */
	} /* end if (ok) */

	if ((rs >= 0) && ((aclspec == NULL) || (aclspec[0] == '\0'))) {
	    ex = EX_USAGE ;
	    rs = SR_INVALID ;
	    shio_printf(pip->efp,
	        "%s: no or invalid ACLs were specified\n",
	        pip->progname) ;
	}

/* parse out the specified ACLs */

	rs = procacls(pip,aclspec) ;

	if ((rs < 0) && (ex == EX_OK)) {
	    ex = EX_USAGE ;
	}

/* process any username we were given */

	if ((rs < 0) && (ex == EX_OK)) {
	    ex = EX_NOUSER ;
	    shio_printf(pip->efp,"%s: invalid username specified\n",
	        pip->progname) ;
	}

/* calculate the Solaris types for later */

	if (rs >= 0) {
	    rs = locinfo_mksol(lip) ;
	}

/* OK, finally do it */

#if	CF_DEFMAXACLS
	pip->n = MAXACLS ;
#else
	pip->n = DEFACLS ;
#endif

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;
	ainfo.ai_continue = ai_continue ;

	if (rs >= 0) {
	    cchar	*afn = afname ;
	    cchar	*ofn = ofname ;
	    if ((rs = process(pip,&ainfo,&pargs,afn)) >= 0) {
	        rs = procoutverbose(pip,lip,ofn,rs) ;
	    }
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

	if (((rs < 0) && (! pip->f.quiet)) || (pip->debuglevel > 0)) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: entries=%u\n" ;
	    shio_printf(pip->efp,fmt,pn,lip->entries) ;
	    fmt = "%s: changed=%u\n" ;
	    shio_printf(pip->efp,fmt,pn,lip->changed) ;
	    fmt = "%s: errored=%u\n" ;
	    shio_printf(pip->efp,fmt,pn,lip->errored) ;
	} /* end if (error output) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	    if (! pip->f.quiet) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt ;
		fmt = "%s: processing error (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    }
	} else if ((rs >= 0) && (ex == EX_OK)) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

/* we are done */
retearly:
	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: exiting ex=%u (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("chacl: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    shio_close(pip->efp) ;
	    pip->efp = NULL ;
	}

	bits_finish(&pargs) ;

badpargs:
	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("chacl: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (mainsub) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s <acl(s)> [<file(s)> ...] [-af <afile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-r] [-f] [-c] [-u <user>] [-of <ofile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-s <suf(s)>] [-t <type(s)>] [-epf <exfile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-d] [-min[=<b>]] [-max[=<b>]] [-mm[=<b>]]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procacls(PROGINFO *pip,cchar *aclspec)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;

	if ((rs = field_start(&fsb,aclspec,-1)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aclterms,&fp)) >= 0) {
	        if (fl > 0) {
	            c += 1 ;
	            rs = procacl(pip,fp,fl) ;
	        }
	        if (rs < 0) {
	            if (pip->debuglevel > 0) {
	                shio_printf(pip->efp,
	                    "%s: a bad ACL was specified (%d) >%t<\n",
	                    pip->progname,rs,fp,fl) ;
	            }
	            break ;
	        }
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("chacl: parse ACLs c=%u\n",c) ;
#endif

	if ((rs < 0) || (c == 0)) {
	    if (rs < 0) {
	        shio_printf(pip->efp,
	            "%s: a bad ACL was specified\n",
	            pip->progname) ;
	    } else {
	        shio_printf(pip->efp,
	            "%s: no ACL was found in specification\n",
	            pip->progname) ;
	    }
	    rs = SR_INVALID ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("chacl: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procacls) */


static int procacl(PROGINFO *pip,cchar *abuf,int alen)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	cchar		*tp ;

	if ((tp = strnpbrk(abuf,alen,"=+-")) != NULL) {
	    aclinfo_t	ai ;
	    int		idlen = 0 ;
	    int		typelen = (tp - abuf) ;
	    int		permlen ;
	    cchar	*typespec = abuf ;
	    cchar	*idspec = NULL ;
	    cchar	*permspec ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("procacl: type=%t\n",typespec,typelen) ;
#endif

	    if (tp[0] == '=') {
	        cchar	*sp = (tp + 1) ;
	        int	sl = ((abuf + alen) - idspec) ;
	        idspec = (tp + 1) ;
	        if ((tp = strnpbrk(sp,sl,"+-")) != NULL) {
	            idlen = (tp - idspec) ;
	        }
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("procacl: id=%t\n",idspec,idlen) ;
#endif
	    } /* end if (had an ID) */

	    if (tp != NULL) {

	        ai.op = (tp[0] == '+') ? aclop_add : aclop_subtract ;

	        permspec = (tp + 1) ;
	        permlen = ((abuf + alen) - permspec) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3)) {
	            debugprintf("procacl: op=%u\n",ai.op) ;
	            debugprintf("procacl: permspec=%t\n",
	                permspec,permlen) ;
	        }
#endif

	        rs = parseperms(permspec,permlen) ;
	        ai.perm = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("procacl: parseperms() rs=%d perm=\\o%02o\n",
	                rs,ai.perm) ;
#endif

/* find what the type is */

	        if (rs >= 0) {
	            ai.type = getacltype(typespec,typelen) ;
	            rs = (ai.type >= 0) ? SR_OK : SR_INVALID ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("procacl: type rs=%d type=%d\n",
	                    rs,ai.type) ;
#endif
	        } /* end if (getting ACL type) */

	        if (rs >= 0) {
	            ai.uid = -1 ;
	            ai.gid = -1 ;

/* get the UID or GID */

	            if ((idspec != NULL) && (idlen > 0)) {

	                if ((ai.type == acltype_user) ||
	                    (ai.type == acltype_defuser)) {

	                    rs = getuid_user(idspec,idlen) ;
	                    ai.uid = rs ;

	                } else if ((ai.type == acltype_group) ||
	                    (ai.type == acltype_defgroup)) {

	                    rs = getgid_group(idspec,idlen) ;
	                    ai.gid = rs ;

	                } /* end if (translations) */

	            } /* end if (had an ID of some sort) */

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("procacl: after rs=%d\n",rs) ;
#endif

	            if (rs >= 0) {
			rs = locinfo_addacl(lip,&ai) ;
	            }

	        }  /* end if (ok) */

	    } else
	        rs = SR_INVALID ;

	} else
	    rs = SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procacl: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procacl) */


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = ids_load(&pip->id)) >= 0) {
	    const int	size = (pip->n * sizeof(aclent_t)) ;
	    char	*bp ;
	    if ((rs = uc_malloc(size,&bp)) >= 0) {
	        LOCINFO		*lip = pip->lip ;
	        int		pan = 0 ;
	        int		cl ;
	        cchar		*cp ;
	        pip->buffer = bp ;

	        if (rs >= 0) {
	            int		ai ;
	            int		f ;
	            cchar	**argv = aip->argv ;
	            for (ai = aip->ai_continue ; ai < aip->argc ; ai += 1) {
	                f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	                f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	                if (f) {
	                    cp = argv[ai] ;
	                    if (cp[0] != '\0') {
	                        pan += 1 ;
	                        rs = procname(pip,cp) ;
	                        if (rs >= 0) c += 1 ;
	                    }
	                }
	                if ((rs < 0) && lip->f.nostop) rs = SR_OK ;
	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
	                if (rs < 0) break ;
	            } /* end for */
	        } /* end if (ok) */

	        if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	            SHIO	afile, *afp = &afile ;

	            if (strcmp(afn,"-") == 0) afn = STDINFNAME ;

	            if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	                const int	llen = LINEBUFLEN ;
	                int		len ;
	                char		lbuf[LINEBUFLEN + 1] ;

	                while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                    len = rs ;

	                    if (lbuf[len - 1] == '\n') len -= 1 ;
	                    lbuf[len] = '\0' ;

	                    if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                        if (cp[0] != '#') {
	                            pan += 1 ;
			    	    lbuf[(cp+cl)-lbuf] = '\0' ;
	                            rs = procname(pip,cp) ;
	                            if (rs > 0) c += 1 ;
	                        }
	                    }

	                    if ((rs < 0) && lip->f.nostop) rs = SR_OK ;
	                    if (rs >= 0) rs = lib_sigterm() ;
	                    if (rs >= 0) rs = lib_sigintr() ;
	                    if (rs < 0) break ;
	                } /* end while (reading lines) */

	                rs1 = shio_close(afp) ;
	                if (rs >= 0) rs = rs1 ;
	            } else {
	                cchar	*pn = pip->progname ;
	                cchar	*fmt ;
	                fmt = "%s: inaccessible argument-list (%d)\n" ;
	                shio_printf(pip->efp,fmt,pn,rs) ;
	                shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	            } /* end if */

	        } /* end if (processing file argument file list) */

	        rs1 = uc_free(pip->buffer) ;
	        if (rs >= 0) rs = rs1 ;
	        pip->buffer = NULL ;
	    } /* end if (m-a-f) */
	    rs1 = ids_release(&pip->id) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ids) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (process) */


static int procname(PROGINFO *pip,cchar *fname)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		c = 0 ;

	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_chacl/procname: ent fname=%s\n",fname) ;
#endif

	if (fname[0] != '-') {
	    FSDIRTREE_STAT	sb, ssb ;
	    int			f_done = FALSE ;
	    int			f_islnk = FALSE ;
	    int			f_dir ;

	    if ((rs = fsdirtreestat(fname,1,&sb)) >= 0) {
	        f_dir = S_ISDIR(sb.st_mode) ;
	        if (S_ISLNK(sb.st_mode)) {
	            f_islnk = TRUE ;
	            if (lip->f.follow) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("b_chacl/procmain: sym-link\n") ;
#endif

	                if ((rs = fsdirtreestat(fname,0,&ssb)) >= 0) {
	                    f_dir = S_ISDIR(ssb.st_mode) ;
	                    if (f_dir) {
	                        sb = ssb ;
	                    } else {
	                        f_islnk = FALSE ;
			    }
	                }

	            } /* end if (follow-mode) */
	        } /* end if (is-link) */
	    } /* end if (following symbolic link) */

	    if ((rs >= 0) && S_ISLNK(sb.st_mode) && (! lip->f.follow)) {
	        f_done = TRUE ;
	    }

	    if ((! f_done) && (rs < 0)) {

	        if (! pip->f.quiet) {
		    cchar	*pn = pip->progname ;
		    cchar	*fmt ;
		    fmt = "%s: file=%s\n" ;
	            shio_printf(pip->efp,fmt,pn,fname) ;
		    fmt = "%s: could not get status (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	        } /* end if */

	        if (lip->f.nostop) {
	            f_done = TRUE ;
	            rs = SR_OK ;
	        }

	    } /* end if */

	    if ((rs >= 0) && (! f_done) && f_dir && lip->f.recurse) {
	        rs = procnamer(pip,fname) ;
	        c += rs ;
	    } /* end if (opened direcory for recursion) */

	    if ((rs >= 0) && (! f_islnk) && (! f_done)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: target name=%s\n",fname) ;
#endif

	        c += 1 ;
	        rs = checkname(pip,fname,&sb) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: checkname() rs=%d\n",rs) ;
#endif

	        if ((rs < 0) && lip->f.nostop)
	            rs = SR_OK ;

	    } /* end if (check on principal name) */

	} /* end if (enabled) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procname) */


static int procnamer(PROGINFO *pip,cchar *fname)
{
	LOCINFO		*lip = pip->lip ;
	FSDIRTREE	dt ;
	int		rs ;
	int		rs1 ;
	int		dtopts = 0 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process: ent\n") ;
#endif

	dtopts |= ((lip->f.follow) ? FSDIRTREE_MFOLLOW : 0) ;
	if ((rs = fsdirtree_open(&dt,fname,dtopts)) >= 0) {
	    struct ustat	esb ;
	    const int		mpl = MAXPATHLEN ;
	    char		dename[MAXPATHLEN + 1] ;
	    char		tmpfname[MAXPATHLEN + 1] ;

	    while ((rs = fsdirtree_read(&dt,&esb,dename,mpl)) > 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: dir name=%s\n",dename) ;
#endif

	        if (lip->f.nhf && (dename[0] == '.'))
	            continue ;

	        if (! S_ISLNK(esb.st_mode)) {
	            if ((rs = mkpath2(tmpfname,fname,dename)) > 0) {
	                c += 1 ;
	                rs = checkname(pip,tmpfname,&esb) ;
	            }
	        }

	        if ((rs < 0) && lip->f.nostop)
	            rs = SR_OK ;

	        if (rs >= 0) rs = lib_sigterm() ;
	        if (rs >= 0) rs = lib_sigintr() ;
	        if (rs < 0) break ;
	    } /* end while (looping through entries) */

	    rs1 = fsdirtree_close(&dt) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    if (! pip->f.quiet) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt ;
	        fmt = "%s: could not open directory (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        fmt = "%s: directory=%s\n" ;
	        shio_printf(pip->efp,fmt,pn,fname) ;
	    }
	    if (lip->f.nostop) rs = SR_OK ;
	} /* end if (could not open directory) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procnamer) */


static int checkname(PROGINFO *pip,cchar *fname,FSDIRTREE_STAT *sbp)
{
	LOCINFO		*lip = pip->lip ;
	aclinfo_t	*ap ;
	aclent_t	*aclbuf = (aclent_t *) pip->buffer ;
	int		rs = SR_OK ;
	int		fd = -1 ;
	int		nacls ;
	int		f_changed = FALSE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("checkname: ent fname=%s\n",fname) ;
#endif

	lip->entries += 1 ;

#if	CF_NOSOCKETS
	if (S_ISSOCK(sbp->st_mode)) goto ret0 ;
#endif

	if (rs >= 0) {
	    rs = 1 ;
	    if ((rs > 0) && lip->f.fsuffixes) {
	        rs = locinfo_isfsuffix(lip,fname) ;
	    } /* end if (file-suffixes) */
	    if ((rs > 0) && lip->f.ftypes) {
	        rs = locinfo_isftype(lip,fname,sbp) ;
	    } /* end if (file-type check) */
	    if ((rs > 0) && (lip->uid_owner >= 0)) {
		rs = (sbp->st_uid == lip->uid_owner) ;
	    }
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("checkname: continue rs=%d\n",rs) ;
#endif

	if (rs <= 0) goto ret0 ; /* get out if not selected */

/* continue */

	if (S_ISREG(sbp->st_mode)) {
	    if ((rs = u_open(fname,O_RDONLY,0666)) >= 0) {
	        fd = rs ;
	    } else if (isNotPresent(rs)) {
		rs = SR_OK ;
	    }
	} /* end if (opened) */

	if (rs >= 0) {
	    if ((lip->gid_new >= 0) && (lip->gid_new != sbp->st_gid)) {
		if (fd >= 0) {
	    	    rs = u_fchown(fd,-1,lip->gid_new) ;
		} else {
	    	    rs = uc_chown(fname,-1,lip->gid_new) ;
		}
	    }
	} /* end if (ok) */

	if (rs >= 0) {
	    mode_t	fm = sbp->st_mode ;
	    int		f_set = FALSE ;
	    if (lip->f.suid && S_ISREG(fm)) {
		if (! (fm & S_ISUID)) {
		    f_set = TRUE ;
		    fm |= S_ISUID ;
		}
	    }
	    if (lip->f.sgid && (S_ISREG(fm) || S_ISDIR(fm))) {
		if (! (fm & S_ISGID)) {
		    f_set = TRUE ;
		    fm |= S_ISGID ;
		}
	    }
	    if (f_set) {
	        if (fd >= 0) {
	            rs = u_fchmod(fd,fm) ;
	        } else {
	            rs = uc_chmod(fname,fm) ;
	        }
	    } /* end if (needed) */
	} /* end if (ok) */

	if (rs >= 0) {
	    if (fd >= 0) {
	        rs = u_facl(fd,GETACL,pip->n,aclbuf) ;
	    } else {
	        rs = u_acl(fname,GETACL,pip->n,aclbuf) ;
	    }
	    nacls = rs ;
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("checkname: GET u_facl() rs=%d\n",rs) ;
#endif

#if	(! CF_DEFMAXACLS)
	if (rs == SR_NOSPC) {

	}
#endif /* CF_DEFMAXACLS */

	if (rs >= 0) {
	    int		i, j ;
	    int		perm_new ;
	    int		perm_min = 0 ;
	    int		perm_max = 0 ;
	    int		f_minus = FALSE ;
	    int		f_idtype ;

/* find the minimum and the maximum perms */

	    if (lip->f.min || lip->f.max) {
	        aclents_minmax(aclbuf,nacls,&perm_min,&perm_max) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("checkname: perm_min=\\o%02o\n",perm_min) ;
	            debugprintf("checkname: perm_max=\\o%02o\n",perm_max) ;
	        }
#endif
	    } /* end if */

/* apply the specified perms */

	    for (i = 0 ; vecobj_get(&lip->acls,i,&ap) >= 0 ; i += 1) {
	        if (ap == NULL) continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("checkname: processing one\n") ;
#endif

	        f_idtype = FALSE ;
	        j = aclents_match(aclbuf,nacls,ap) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("checkname: aclents_match() rs=%d\n",j) ;
#endif

	        if (j >= 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("checkname: matched\n") ;
#endif

	            if (ap->op == aclop_add) {
	                perm_new = aclbuf[j].a_perm | ap->perm ;
	            } else {
	                f_minus = TRUE ;
	                perm_new = aclbuf[j].a_perm & (~ ap->perm) ;
	            }

	            if ((lip->f.min || lip->f.max) &&
	                (aclent_idtype(aclbuf + j) > 0)) {

	                if (lip->f.min && (perm_min >= 0)) {
	                    perm_new |= perm_min ;
			}

	                if (lip->f.max && (perm_max >= 0)) {
	                    perm_new &= perm_max ;
			}

	            }

	            if (aclbuf[j].a_perm != perm_new) {
	                f_changed = TRUE ;
	                aclbuf[j].a_perm = (short) perm_new ;
	            }

	        } else if ((ap->op == aclop_add) &&
	            (nacls < (MAXACLS - 1))) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("checkname: did not match\n") ;
#endif

	            if ((aclinfo_isdeftype(ap) == 0) ||
	                S_ISDIR(sbp->st_mode)) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("checkname: not_default or dir\n") ;
#endif

	                j = nacls ;
	                aclbuf[j].a_type = ap->soltype ;
	                aclbuf[j].a_id = 0 ;
	                if ((ap->uid >= 0) &&
	                    ((ap->type == acltype_user) ||
	                    (ap->type == acltype_defuser))) {

	                    f_idtype = TRUE ;
	                    aclbuf[j].a_id = ap->uid ;

	                } else if ((ap->gid >= 0) &&
	                    ((ap->type == acltype_group) ||
	                    (ap->type == acltype_defgroup))) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("checkname: group or defgroup\n") ;
#endif

	                    f_idtype = TRUE ;
	                    aclbuf[j].a_id = ap->gid ;

	                } /* end if */

	                perm_new = ap->perm ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    debugprintf("checkname: perm_new=\\o%2o\n",
	                        perm_new) ;
	                    debugprintf("checkname: f_idtype=%u\n",
	                        f_idtype) ;
	                }
#endif /* CF_DEBUG */

	                if ((lip->f.min || lip->f.max) && f_idtype) {
	                    if (lip->f.min && (perm_min >= 0)) {
	                        perm_new |= perm_min ;
			    }
	                    if (lip->f.max && (perm_max >= 0)) {
	                        perm_new &= perm_max ;
			    }
	                }

	                if (perm_new > 0) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("checkname: perm_new greater\n") ;
#endif

	                    if (lip->f.min && f_idtype) {

#if	CF_DEBUG
	                        if (DEBUGLEVEL(4))
	                            debugprintf("checkname: min and "
	                                "typeextra\n") ;
#endif

	                        if ((perm_min & perm_new) != perm_new) {
	                            f_changed = TRUE ;
	                            aclbuf[nacls++].a_perm = (short) perm_new ;
	                        }

	                    } else {
	                        f_changed = TRUE ;
	                        aclbuf[nacls++].a_perm = (short) perm_new ;
	                    }

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("checkname: perm_new GT "
	                            "f_changed=%u\n",
	                            f_changed) ;
#endif

	                } /* end if (perm_new) */

	            } /* end if (not_default or directory) */

	        } /* end if (matched or not) */

	    } /* end for (looping through specified ACLs) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("checkname: f_changed=%u\n",f_changed) ;
	        debugprintf("checkname: check for min and minus\n") ;
	    }
#endif

	    if (lip->f.min && f_minus) {
	        int	f_gotone = FALSE ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("checkname: min and minus\n") ;
#endif

	        for (i = 0 ; vecobj_get(&lip->acls,i,&ap) >= 0 ; i += 1) {
	            if (ap == NULL) continue ;

	            if (aclinfo_isidtype(ap) > 0) {

	                j = aclents_match(aclbuf,nacls,ap) ;

	                if ((j >= 0) && 
	                    (aclbuf[j].a_perm == perm_min)) {
	                    f_changed = TRUE ;
	                    f_gotone = TRUE ;
	                    aclbuf[j].a_type = -1 ;
	                }

	            } /* end if (was an extra type) */

	        } /* end for (specified ACLs) */

	        if (f_gotone)
	            nacls = aclents_compact(aclbuf,nacls) ;

	    } /* end if (minimum permissions in effect) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("checkname: f_changed=%u\n",f_changed) ;
#endif

	    if (lip->f.delempty) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("checkname: delete_empty\n") ;
#endif

	        for (j = 0 ; j < nacls ; j += 1) {

	            if (aclent_empty(aclbuf + j) > 0) {

	                f_changed = TRUE ;
	                if (j < (nacls - 1)) {
	                    aclbuf[j] = aclbuf[nacls - 1] ;
	                } /* end if (swapping) */

	                nacls -= 1 ;
	                if (j < nacls)
	                    j -= 1 ;

	            } /* end if (found empry one) */

	        } /* end for */

	    } /* end if (check for empty ACLs) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("checkname: nacls=%u\n",nacls) ;
#endif

/* verify that a "mask" ACL entry is present */

	    if (nacls > 0) {
	        int	perm_mask = 0 ;
	        int	rc ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("checkname: is mask present?\n") ;
#endif

/* default mask */

	        j = aclents_defmaskneed(aclbuf,nacls) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("checkname: aclents_defmaskneed() rs=%d\n",
	                j) ;
#endif

	        if (j < -1) {

	            if (nacls < (MAXACLS - 1)) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("checkname: default mask i=%u\n",
	                        nacls) ;
#endif

	                j = nacls++ ;
	                aclbuf[j].a_type = ACLTYPE_DEFCLASSOBJ ;
	                aclbuf[j].a_id = 0 ;
	                aclbuf[j].a_perm = 0007 ;
	                f_changed = TRUE ;

	            } /* end if */

	        } /* end if (needed a default mask entry) */

/* regular mask */

	        j = aclents_maskneed(aclbuf,nacls) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("checkname: aclents_maskneed() rs=%d\n",
	                j) ;
#endif

	        if (j < -1) {

	            if (nacls < (MAXACLS - 1)) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("checkname: adding mask i=%u\n",
	                        nacls) ;
#endif

	                j = nacls++ ;
	                aclbuf[j].a_type = ACLTYPE_CLASSOBJ ;
	                aclbuf[j].a_id = 0 ;
	                aclbuf[j].a_perm = 07 ;
	                f_changed = TRUE ;

	            } /* end if */

	        } else if (j >= 0)
	            perm_mask = aclbuf[j].a_perm ;

	        if (lip->f.maskcalc) {

	            if ((j < 0) && (nacls < (MAXACLS - 1))) {

	                j = nacls++ ;
	                aclbuf[j].a_type = ACLTYPE_CLASSOBJ ;
	                aclbuf[j].a_id = 0 ;
	                aclbuf[j].a_perm = 0 ;
	                f_changed = TRUE ;

	            } /* end if */

#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                int	which ;
	                rc = aclcheck(aclbuf,nacls,&which) ;
	                debugprintf("checkname: aclcheck() rc=%d\n",rc) ;
	                if (rc != 0) {
	                    debugprintf("checkname: which=%u\n",which) ;
	                    switch (rc) {
	                    case GRP_ERROR:
	                        debugprintf("checkname: group error\n") ;
	                        break ;
	                    case USER_ERROR:
	                        debugprintf("checkname: user error\n") ;
	                        break ;
	                    case CLASS_ERROR:
	                        debugprintf("checkname: class error\n") ;
	                        break ;
	                    case OTHER_ERROR:
	                        debugprintf("checkname: other error\n") ;
	                        break ;
	                    case DUPLICATE_ERROR:
	                        debugprintf("checkname: duplicate error\n") ;
	                        break ;
	                    case ENTRY_ERROR:
	                        debugprintf("checkname: entry error\n") ;
	                        break ;
	                    case MISS_ERROR:
	                        debugprintf("checkname: miss error\n") ;
	                        break ;
	                    case MEM_ERROR:
	                        debugprintf("checkname: mem error\n") ;
	                        break ;
	                    } /* end switch */
	                    aclents_print(aclbuf,nacls) ;
	                } /* end if (bad check) */
	            }
#endif /* CF_DEBUG */

/* re-calculate the "mask" entry */

	            rc = aclsort(nacls,TRUE,aclbuf) ;
	            rs = (rc == 0) ? SR_OK : SR_INVALID ;

	            if (rs >= 0) {

	                j = aclents_maskmat(aclbuf,nacls) ;

	                if ((j >= 0) &&
	                    (aclbuf[j].a_perm != perm_mask))
	                    f_changed = TRUE ;

	            } /* end if */

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("checkname: aclsort() rs=%d\n",rs) ;
#endif

	        } /* end if (mask re-calculation) */

	    } /* end if (mask check) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("checkname: f_changed=%u\n",f_changed) ;
#endif

/* finally set the actual ACL (as needed) on the file */

	    if ((rs >= 0) && f_changed && (nacls > 0)) {

	        if ((rs = checkowner(pip,sbp,fname)) >= 0) {

	            if (fd >= 0) {
	                rs = u_facl(fd,SETACL,nacls,aclbuf) ;
	            } else {
	                rs = u_acl(fname,SETACL,nacls,aclbuf) ;
	            }

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("checkname: SET fd=%d u_facl() rs=%d\n",
	                    fd,rs) ;
#endif

	        } /* end if */

	    } /* end if (installed new ACLs) */

/* do we need to change the owner? */

	if ((rs >= 0) && (lip->uid_new >= 0)) {
	    if (sbp->st_uid != lip->uid_new) {
	        if (fd >= 0) {
	            rs = u_fchown(fd,lip->uid_new,-1) ;
	        } else {
	            rs = u_chown(fname,lip->uid_new,-1) ;
		}
	    }
	} /* end if (ownership) */

	} /* end if (ok) */

/* clean up */

	if (fd >= 0) {
	    u_close(fd) ;
	    fd = -1 ;
	}

/* continue */

	if (rs != SR_PERM) {
	    if ((rs < 0) && isNotPresent(rs)) {
	        lip->errored += 1 ;
	        f_changed = FALSE ;
	        if (! pip->f.quiet) {
		    cchar	*pn = pip->progname ;
		    cchar	*fmt ;
		    fmt = "%s: file=%s\n" ;
	            shio_printf(pip->efp,fmt,pn,fname) ;
	            if (rs == SR_NOENT) {
			fmt = "%s: not found (%d)\n" ;
	                shio_printf(pip->efp,pn,rs) ;
	            } else {
			fmt = "%s: error (%d)\n" ;
	                shio_printf(pip->efp,pn,rs) ;
		    }
	        } /* end if */
	        if (lip->f.nostop) rs = SR_OK ;
	    } /* end if (error) */
	} /* end if (not permission error) */

ret0:
	if ((rs >= 0) && f_changed)
	    lip->changed += 1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("checkname: ret rs=%d f_changed=%u\n",
	        rs,f_changed) ;
#endif

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (checkname) */


/* check ownership (needed for any changes) */
static int checkowner(PROGINFO *pip,FSDIRTREE_STAT *sbp,cchar fname[])
{
	int		rs = SR_OK ;
	if ((pip->id.uid != 0) && (sbp->st_uid != pip->id.uid)) {
	    rs = SR_PERM ;
	    if (! pip->f.quiet) {
		cchar	*pn = pip->progname ;
		cchar	*fmt ;
		fmt = "%s: not owner (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
		fmt = "%s: file=%s\n" ;
	        shio_printf(pip->efp,fmt,pn,fname) ;
	    } /* end if */
	} /* end if */
	return rs ;
}
/* end subroutine (checkowner) */


/* output verbose information */
static int procoutverbose(PROGINFO *pip,LOCINFO *lip,cchar *ofn,int c)
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

	if (ofn == NULL) ofn = getourenv(pip->envv,VAROFNAME) ;
	if (ofn == NULL) ofn = STDOUTFNAME ;

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: ofile=%s (%u)\n",
	        pip->progname,ofn,c) ;
	}

	if (pip->verboselevel >= 2) {
	if ((rs = shio_open(ofp,ofn,"wca",0666)) >= 0) {
	    if (rs >= 0) {
	        rs = shio_printf(ofp,"entries=%u\n",lip->entries) ;
	        wlen += rs ;
	    }
	    if (rs >= 0) {
	        rs = shio_printf(ofp,"changed=%u\n",lip->changed) ;
	        wlen += rs ;
	    }
	    if (rs >= 0) {
	        rs = shio_printf(ofp,"errored=%u\n",lip->errored) ;
	        wlen += rs ;
	    }
	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (output file opened) */
	} /* end if (verbose) */

	return (rs >= 0) ? wlen : rs1 ;
}
/* end subroutine (procoutverbose) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->uid_owner = -1 ;
	lip->uid_new = -1 ;
	lip->gid_new = -1 ;

	if ((rs = keyopt_start(&lip->akopts)) >= 0) {
	    pip->open.akopts = TRUE ;
	    if ((rs = paramopt_start(&lip->aparams)) >= 0) {
	        const int	esize = sizeof(aclinfo_t) ;
	        pip->open.aparams = TRUE ;
	        if ((rs = vecobj_start(&lip->acls,esize,10,0)) >= 0) {
	            lip->f.acls = TRUE ;
	        }
	        if (rs < 0) {
	            pip->open.aparams = FALSE ;
	            paramopt_finish(&lip->aparams) ;
	        }
	    } /* end if (paramopt_start) */
	    if (rs < 0) {
	        pip->open.akopts = FALSE ;
	        keyopt_finish(&lip->akopts) ;
	    }
	} /* end if (keyopt_start) */

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->f.acls) {
	    lip->f.acls = FALSE ;
	    rs1 = vecobj_finish(&lip->acls) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    rs1 = paramopt_finish(&lip->aparams) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    rs1 = keyopt_finish(&lip->akopts) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


/* process the program options */
static int locinfo_procopts(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	KEYOPT		*kop = &lip->akopts ;
	KEYOPT_CUR	kcur ;
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(progopts,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case progopt_nhf:
			    if (! lip->final.nhf) {
	                        lip->final.nhf = TRUE ;
	                        lip->have.nhf = TRUE ;
	                        lip->f.nhf = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.nhf = (rs > 0) ;
	                        }
			    }
	                    break ;
	                case progopt_suid:
			    if (! lip->final.suid) {
	                        lip->final.suid = TRUE ;
	                        lip->have.suid = TRUE ;
	                        lip->f.suid = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.suid = (rs > 0) ;
	                        }
			    }
	                    break ;
	                case progopt_sgid:
			    if (! lip->final.sgid) {
	                        lip->final.sgid = TRUE ;
	                        lip->have.sgid = TRUE ;
	                        lip->f.sgid = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.sgid = (rs > 0) ;
	                        }
			    }
	                    break ;
	                } /* end switch */

	                c += 1 ;
	            } /* end if (valid option) */

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_procopts) */


static int locinfo_ftypes(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	PARAMOPT_CUR	cur ;
	int		rs = SR_OK ;
	int		sl ;
	int		fti ;
	int		c = 0 ;
	cchar		*sp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("chacl/locinfo_ftypes: ent\n") ;
#endif

	if ((rs = paramopt_curbegin(&lip->aparams,&cur)) >= 0) {

	    while (rs >= 0) {

	        sl = paramopt_fetch(&lip->aparams,PO_TYPE,&cur,&sp) ;
	        if (sl == SR_NOTFOUND) break ;

	        if (sl == 0) continue ;

	        rs = sl ;
	        if (rs < 0) break ;

	        if ((fti = matostr(ftypes,1,sp,sl)) >= 0) {
	            int	f_prior = FALSE ;
	            switch (fti) {

	            case ftype_regular:
	            case ftype_file:
	                f_prior = lip->ft.f ;
	                lip->ft.f = TRUE ;
	                break ;

	            case ftype_directory:
	                f_prior = lip->ft.d ;
	                lip->ft.d = TRUE ;
	                break ;

	            case ftype_block:
	                f_prior = lip->ft.b ;
	                lip->ft.b = TRUE ;
	                break ;

	            case ftype_character:
	                f_prior = lip->ft.c ;
	                lip->ft.c = TRUE ;
	                break ;

	            case ftype_pipe:
	            case ftype_fifo:
	                f_prior = lip->ft.p ;
	                lip->ft.p = TRUE ;
	                break ;

	            case ftype_socket:
	                f_prior = lip->ft.s ;
	                lip->ft.s = TRUE ;
	                break ;

	            case ftype_link:
	                f_prior = lip->ft.l ;
	                lip->ft.l = TRUE ;
	                break ;

	            case ftype_door:
	                f_prior = lip->ft.D ;
	                lip->ft.D = TRUE ;
	                break ;

	            } /* end switch */
	            c += 1 ;
	            if ((pip->debuglevel > 0) && (! f_prior)) {
	                shio_printf(pip->efp,"%s: ftype=%s\n",
	                    pip->progname,ftypes[fti]) ;
	            }
	        } /* end if */

	    } /* end while */

	    paramopt_curend(&lip->aparams,&cur) ;
	} /* end if (cursor) */

	if (c > 0)
	    lip->f.ftypes = TRUE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("chacl/locinfo_ftypes: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_ftypes) */


static int locinfo_isfsuffix(LOCINFO *lip,cchar fname[])
{
	PROGINFO	*pip = lip->pip ;
	PARAMOPT	*pp = &lip->aparams ;
	PARAMOPT_CUR	cur ;
	int		rs = SR_OK ;
	int		sl ;
	int		bl ;
	int		f = FALSE ;
	cchar		*po = PO_SUFFIX ;
	cchar		*tp ;
	cchar		*sp ;
	cchar		*cp ;
	cchar		*bp ;

	if (pip == NULL) return SR_FAULT ;

	if ((bl = sfbasename(fname,-1,&bp)) > 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("locinfo_isfsuffix: bn=%t\n",bp,bl) ;
#endif

	    if ((tp = strnrchr(bp,bl,'.')) != NULL) {
	        cp = (tp + 1) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("locinfo_isfsuffix: fs=%s\n",cp) ;
#endif

	        if ((rs = paramopt_curbegin(pp,&cur)) >= 0) {

	            while (rs >= 0) {

	                sl = paramopt_fetch(pp,po,&cur,&sp) ;
	                if (sl == SR_NOTFOUND)
	                    break ;

	                if (sl == 0) continue ;

	                rs = sl ;
	                if (rs < 0)
	                    break ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("locinfo_isfsuffix: s=%t\n",sp,sl) ;
#endif

	                f = (strncmp(cp,sp,sl) == 0) && (cp[sl] == '\0') ;
	                if (f)
	                    break ;

	            } /* end while */

	            paramopt_curend(pp,&cur) ;
	        } /* end if (cursor) */

	    } /* end if (non-null) */

	} /* end if (positive) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("locinfo_isfsuffix: fname=%s\n",fname) ;
	    debugprintf("locinfo_isfsuffix: ret rs=%d f=%u\n",rs,f) ;
	}
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (locinfo_isfsuffix) */


static int locinfo_isftype(LOCINFO *lip,cchar fname[],FSDIRTREE_STAT *sbp)
{
	const mode_t	m = sbp->st_mode ;
	int		f = FALSE ;

	if (fname == NULL) return SR_FAULT ;

	if (S_ISREG(m)) {
	    f = lip->ft.f ;

	} else if (S_ISDIR(m)) {
	    f = lip->ft.d ;

	} else if (S_ISCHR(m)) {
	    f = lip->ft.c ;

	} else if (S_ISBLK(m)) {
	    f = lip->ft.b ;

	} else if (S_ISFIFO(m)) {
	    f = lip->ft.p ;

	} else if (S_ISSOCK(m)) {
	    f = lip->ft.s ;

	} else if ((m & S_IFMT) == S_IFNAM) {
	    f = lip->ft.n ;

	} else if (S_ISLNK(m)) {
	    f = lip->ft.l ;

	} else if (S_ISDOOR(m))
	    f = lip->ft.D ;

	return f ;
}
/* end subroutine (locinfo_isftype) */


static int locinfo_addacl(LOCINFO *lip,aclinfo_t *aip)
{
	return vecobj_add(&lip->acls,aip) ;
}
/* end subroutine (locinfo_addacl) */


static int locinfo_mksol(LOCINFO *lip)
{
	vecobj		*alp = &lip->acls ;
	aclinfo_t	*ap ;
	int		i ;
	for (i = 0 ; vecobj_get(alp,i,&ap) >= 0 ; i += 1) {
	        if (ap != NULL) {
	            aclinfo_mksol(ap) ;
	        }
	} /* end for */
	return 0 ;
} 
/* end subroutine (locinfo_mksol) */


/* aclents (ACL entries) */
static int aclents_match(aclent_t aclbuf[],int nacls,struct aclinfo *ap)
{
	int		j ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("aclents_match: searching soltype=%u uid=%d gid=%d\n",
	    ap->soltype,ap->uid,ap->gid) ;
#endif

	for (j = 0 ; j < nacls ; j += 1) {

#if	CF_DEBUGS
	    debugprintf("aclents_match: existing soltype=%u\n", 
	        aclbuf[j].a_type) ;
	    debugprintf("aclents_match: id=%d\n",aclbuf[j].a_id) ;
#endif

	    if (aclbuf[j].a_type == ap->soltype) {

#if	CF_DEBUGS
	        debugprintf("aclents_match: soltype match\n") ;
#endif

	        f = TRUE ;
	        if ((ap->type == acltype_user) ||
	            (ap->type == acltype_defuser)) {

#if	CF_DEBUGS
	            debugprintf("aclents_match: user type eid=%d\n",
	                aclbuf[j].a_id) ;
#endif

	            if (ap->uid >= 0)
	                f = (ap->uid == aclbuf[j].a_id) ;

	        } else if ((ap->type == acltype_group) ||
	            (ap->type == acltype_defgroup)) {

#if	CF_DEBUGS
	            debugprintf("aclents_match: before group type f=%u\n",f) ;
#endif

	            if (ap->gid >= 0)
	                f = (ap->gid == aclbuf[j].a_id) ;

#if	CF_DEBUGS
	            debugprintf("aclents_match: group type f=%u\n",f) ;
#endif

	        } /* end if */

	        if (f) break ;
	    } /* end if (soltype match) */

	} /* end for */

#if	CF_DEBUGS
	debugprintf("aclents_match: match=%u j=%d\n",f,j) ;
#endif

	return (f) ? j : -1 ;
}
/* end subroutine (aclents_match) */


static int aclents_maskmat(aclent_t aclbuf[],int nacls)
{
	int		j ;

	for (j = 0 ; j < nacls ; j += 1) {
	    if (aclbuf[j].a_type == ACLTYPE_CLASSOBJ) break ;
	} /* end for */

	return (j < nacls) ? j : -1 ;
}
/* end subroutine (aclents_maskmat) */


/* does this set need a MASK ACL? */

/****
	Returns:
	>=0	had mask and this is its index
	-1	did not have mask and don't need one
	-2	did not have mask and need one
****/

static int aclents_maskneed(aclent_t aclbuf[],int nacls)
{
	int		j ;
	int		f_need = FALSE ;

	for (j = 0 ; j < nacls ; j += 1) {
	    if (aclbuf[j].a_type == ACLTYPE_CLASSOBJ) break ;
	    if (! f_need) {
	        f_need = f_need || (aclbuf[j].a_type == ACLTYPE_USER) ;
	        f_need = f_need || (aclbuf[j].a_type == ACLTYPE_GROUP) ;
	    }
	} /* end for */

	return (j < nacls) ? j : ((f_need) ? -2 : -1) ;
}
/* end subroutine (aclents_maskneed) */


/* does this set need a DEFAULT MASK ACL? */

/****
	Returns:
	>=0	had mask and this is its index
	-1	did not have mask and don't need one
	-2	did not have mask and need one
****/

static int aclents_defmaskneed(aclent_t aclbuf[],int nacls)
{
	int		j ;
	int		f_need = FALSE ;

	for (j = 0 ; j < nacls ; j += 1) {
	    if (aclbuf[j].a_type == ACLTYPE_DEFCLASSOBJ) break ;
	    if (! f_need) {
	        f_need = f_need || (aclbuf[j].a_type == ACLTYPE_DEFUSER) ;
	        f_need = f_need || (aclbuf[j].a_type == ACLTYPE_DEFGROUP) ;
	    }
	} /* end for */

	return (j < nacls) ? j : ((f_need) ? -2 : -1) ;
}
/* end subroutine (aclents_defmaskneed) */


/* find the minimum and maximum permissions for this set */
static int aclents_minmax(aclent_t aclbuf[],int nacls,int *pminp,int *pmaxp)
{
	int		i ;
	int		c = 0 ;

	*pminp = -1 ;
	*pmaxp = -1 ;
	for (i = 0 ; (c < 2) && (i < nacls) ; i += 1) {
	    if (aclbuf[i].a_type == USER_OBJ) {
	        c += 1 ;
	        *pmaxp = aclbuf[i].a_perm ;
	    } else if (aclbuf[i].a_type == OTHER_OBJ) {
	        c += 1 ;
	        *pminp = aclbuf[i].a_perm ;
	    } /* end if */
	} /* end for */

	return c ;
}
/* end subroutine (aclents_minmax) */


#if	CF_DEBUGS || CF_DEBUG

static int aclents_print(aclent_t aclbuf[],int nacls)
{
	cchar		*textbuf ;
	cchar		*tp, *sp ;

	if ((textbuf = acltotext(aclbuf,nacls)) != NULL) {

	    sp = textbuf ;
	    while ((tp = strchr(sp,',')) != NULL) {

	        debugprintf("chacl/aclents_print: | %t\n",
	            sp,(tp - sp)) ;

	        sp = (tp + 1) ;

	    } /* end while */

	    if (*sp != '\0')
	        debugprintf("chacl/aclents_print: | %s\n",sp) ;

	    uc_free(textbuf) ;
	} else
	    debugprintf("chacl/aclents_print: couldn't get text\n") ;

	return 0 ;
}
/* end subroutine (aclents_print) */

#endif /* CF_DEBUGS || CF_DEBUG */


static int aclents_compact(aclent_t aclbuf[],int nacls)
{
	int		i ;
	int		c = 0 ;
	int		f_flipped = FALSE ;

	while ((nacls > 0) && (aclbuf[nacls - 1].a_type < 0)) {
	    nacls -= 1 ;
	}

	for (i = 0 ; i < nacls ; i += 1) {
	    if (aclbuf[i].a_type >= 0) {
	        if (f_flipped) aclbuf[c] = aclbuf[i] ;
	        c += 1 ;
	    } else
	        f_flipped = TRUE ;
	} /* end for */

	return c ;
}
/* end subroutine (aclents_compact) */


/* aclent (ACL entry, a single entry) */
static int aclent_empty(aclent_t *aclp)
{
	int		f = FALSE ;

	if (aclp->a_perm == 0) {
	    switch (aclp->a_type) {
	    case USER:
	    case GROUP:
	    case DEF_USER:
	    case DEF_GROUP:
	        f = TRUE ;
	        break ;
	    } /* end switch */
	} /* end if (no perms) */

	return f ;
}
/* end subroutine (aclent_empty) */


static int aclent_idtype(aclent_t *aclp)
{
	int		f = FALSE ;

	switch (aclp->a_type) {
	case USER:
	case GROUP:
	case DEF_USER:
	case DEF_GROUP:
	    f = TRUE ;
	    break ;
	} /* end switch */

	return f ;
}
/* end subroutine (aclent_idtype) */


static int parseperms(cchar *psp,int psl)
{
	int		i ;
	int		ch ;
	int		rc = 0 ;

	for (i = 0 ; (rc >= 0) && (i < psl) && (psp[i] != '\0') ; i += 1) {
	    ch = (psp[i] & 0xff) ;
	    switch (ch) {
	    case 't':
	        rc |= T_OK ;
	        break ;
	    case 's':
	        rc |= S_OK ;
	        break ;
	    case 'r':
	        rc |= R_OK ;
	        break ;
	    case 'w':
	        rc |= W_OK ;
	        break ;
	    case 'x':
	        rc |= X_OK ;
	        break ;
	    default:
	        rc = SR_INVALID ;
	        break ;
	    } /* end switch */
	} /* end for */

	return rc ;
}
/* end subroutine (parseperms) */


