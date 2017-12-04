/* b_smesg */

/* this is an enhanced SHELL built-in version of 'mesg(1)' */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_DEBUGN	0		/* special debugging */
#define	CF_TERMDEV	1		/* use TERMDEV environment variable */
#define	CF_LOCSETENT	1		/* |locinfo_setentry()| */
#define	CF_PERCACHE	0		/* use persistent cache */


/* revision history:

	= 2004-03-01, David A­D­ Morano
        This subroutine was originally written as a KSH built-in command. This
        command (which also can be compiled as an independent program) provides
        some additional capabilities over the standard UNIX® System MESG(1)
        program.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ smesg [-y|-n] [<fromuser(s)>]


*******************************************************************************/


#include	<envstandards.h>

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

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<vecint.h>
#include	<vechand.h>
#include	<osetstr.h>
#include	<field.h>
#include	<sntmtime.h>
#include	<ids.h>
#include	<getxusername.h>
#include	<getutmpent.h>
#include	<grmems.h>
#include	<sysrealname.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_smesg.h"
#include	"defs.h"
#include	"proglog.h"
#include	"proguserlist.h"
#include	"percache.h"


/* local defines */

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif /* LINEBUFLEN */

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

#ifndef	DEVDNAME
#define	DEVDNAME	"/dev"
#endif

#define	NDF		"smesg.deb"

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags
#define	LOCINFO_GMCUR	struct locinfo_gmcur
#define	LOCINFO_RNCUR	struct locinfo_rncur

#define	LOCNOTE		struct locnote


/* typedefs */

typedef int	(*vcmpfun_t)(const void *,const void *) ;


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matpstr(cchar **,int,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	mkplogid(char *,int,cchar *,int) ;
extern int	mksublogid(char *,int,cchar *,int) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	tolc(int) ;
extern int	touc(int) ;
extern int	tofc(int) ;
extern int	hasalldig(const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	hasMeAlone(cchar *,int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strdcpy1(char *,int,cchar *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */

#if	CF_PERCACHE
extern PERCAHE	pc ;			/* unitialized it stays in BSS */
#endif


/* local structures */

struct locinfo_gmcur {
	GRMEMS_CUR	gmcur ;
} ;

struct locinfo_rncur {
	SYSREALNAME_CUR	rncur ;
} ;

struct locinfo_flags {
	uint		stores:1 ;
	uint		percache:1 ;
	uint		notes:1 ;
	uint		notes_prev:1 ;
	uint		msgs:1 ;
	uint		owner:1 ;
	uint		sort:1 ;
	uint		types:1 ;
	uint		linelen:1 ;
	uint		gm:1 ;
	uint		rn:1 ;
	uint		max:1 ;
	uint		datelong:1 ;
} ;

struct locinfo {
	PROGINFO	*pip ;
	const char	*newstate ;
	const char	*termtype ;
	const char	*devdname ;
	const char	*termdev ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	VECSTR		stores ;
	GRMEMS		gm ;
	SYSREALNAME	rn ;
	VECINT		types ;
	VECHAND		notes ;
	int		oldmesgval ;
	int		newmesgval ;
	int		newbiffval ;
	int		linelen ;
	int		to ;
	int		nmax ;
	int		nnotes ;
	int		nusers ;
	int		typesort ;
	char		unbuf[USERNAMELEN+1] ;
	char		gnbuf[GROUPNAMELEN+1] ;
} ;

struct locnote {
	time_t		stime ;
	uint		type ;
	int		nlen ;
	cchar		*nbuf ;
	char		user[SESMSG_USERLEN+1] ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procmesg(PROGINFO *,SHIO *) ;
static int	procmesgout(PROGINFO *,SHIO *,cchar *,struct ustat *) ;
static int	procnotes(PROGINFO *,ARGINFO *,BITS *,SHIO *,cchar *) ;
static int	procnotesout(PROGINFO *,SHIO *) ;
static int	procnoteouter(PROGINFO *,SHIO *,LOCNOTE *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *) ;
static int	procargers(PROGINFO *,OSETSTR *) ;
static int	procloadnames(PROGINFO *,OSETSTR *,const char *,int) ;
static int	procloadname(PROGINFO *,OSETSTR *,const char *,int) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;
static int	procuserinfo_logid(PROGINFO *) ;

static int	proclog_info(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_defs(LOCINFO *) ;
static int	locinfo_username(LOCINFO *) ;
static int	locinfo_groupname(LOCINFO *) ;
static int	locinfo_gmcurbegin(LOCINFO *,LOCINFO_GMCUR *) ;
static int	locinfo_gmcurend(LOCINFO *,LOCINFO_GMCUR *) ;
static int	locinfo_gmlook(LOCINFO *,LOCINFO_GMCUR *,const char *,int) ;
static int	locinfo_gmread(LOCINFO *,LOCINFO_GMCUR *,char *,int) ;
static int	locinfo_rncurbegin(LOCINFO *,LOCINFO_RNCUR *) ;
static int	locinfo_rncurend(LOCINFO *,LOCINFO_RNCUR *) ;
static int	locinfo_rnlook(LOCINFO *,LOCINFO_RNCUR *,cchar *,int) ;
static int	locinfo_rnread(LOCINFO *,LOCINFO_RNCUR *,char *,int) ;
static int	locinfo_typesadds(LOCINFO *,const char *,int) ;
static int	locinfo_typesadder(LOCINFO *,const char *,int) ;
static int	locinfo_types(LOCINFO *) ;
static int	locinfo_typesbegin(LOCINFO *) ;
static int	locinfo_typesend(LOCINFO *) ;
#if	CF_TYPESCOUNT
static int	locinfo_typescount(LOCINFO *) ;
#endif
static int	locinfo_typesmatch(LOCINFO *,int) ;
static int	locinfo_termdevbegin(LOCINFO *) ;
static int	locinfo_termdevend(LOCINFO *) ;
static int	locinfo_notesbegin(LOCINFO *) ;
static int	locinfo_notesend(LOCINFO *) ;
static int	locinfo_notesfins(LOCINFO *) ;
static int	locinfo_notes(LOCINFO *) ;
static int	locinfo_notesarrange(LOCINFO *) ;
static int	locinfo_noteload(LOCINFO *,KSHLIB_NOTE *) ;
static int	locinfo_userbegin(LOCINFO *,OSETSTR *) ;
static int	locinfo_userend(LOCINFO *) ;
static int	locinfo_usermatch(LOCINFO *,OSETSTR *,cchar *) ;
static int	locinfo_noteadm(LOCINFO *) ;
static int	locinfo_setsort(LOCINFO *,cchar *,int) ;
static int	locinfo_optdate(LOCINFO *,cchar *,int) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif /* CF_LOCSETENT */

static int 	locnote_start(LOCNOTE *,KSHLIB_NOTE *) ;
static int 	locnote_finish(LOCNOTE *) ;

#if	CF_PERCACHE
static void	ourfini() ;
#endif /* CF_PERCACHE */

static int	vcmpfor(const void *,const void *) ;
static int	vcmprev(const void *,const void *) ;


/* local variables */

static cchar	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"pm",
	"sn",
	"af",
	"ef",
	"of",
	"dd",
	"dev",
	"line",
	"td",
	"notes",
	"max",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_pm,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_dd,
	argopt_dev,
	argopt_line,
	argopt_td,
	argopt_notes,
	argopt_max,
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

static cchar	*akonames[] = {
	"owner",
	"sort",
	"line",
	"notes",
	"max",
	"date",
	NULL
} ;

enum akonames {
	akoname_owner,
	akoname_sort,
	akoname_line,
	akoname_notes,
	akoname_max,
	akoname_date,
	akoname_overlast
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

static cchar	*ntypes[] = {
	"exit",
	"noop",
	"gen",
	"biff",
	"other",
	NULL
} ;

enum ntypes {
	ntype_exit,
	ntype_noop,
	ntype_gen,
	ntype_biff,
	mtype_other,
	ntype_overlast
} ;

static cchar	*typesorts[] = {
	"forward",
	"reverse",
	NULL
} ;

enum typesorts {
	typesort_forward,
	typesort_reverse,
	typesort_overlast
} ;

static cchar	*progmodes[] = {
	"smesg",
	"notes",
	NULL
} ;

enum progmodes {
    progmode_smesg,
    progmode_notes,
    progmode_overlast
} ;

static cchar	*datetypes[] = {
	"long",
	NULL
} ;

enum datetypes {
	datetype_long,
	datetype_overlast
} ;

static cchar	types[] = "¤°¥¶¢" ;


/* exported subroutines */


int b_smesg(int argc,cchar **argv,void *contextp)
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    cchar	**envv = (const char **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_smesg) */


int b_notes(int argc,cchar **argv,void *contextp)
{
	return b_smesg(argc,argv,contextp) ;
}
/* end subroutine (b_notes) */


int p_smesg(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_smesg) */


int p_notes(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_smesg) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	SHIO		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		cl ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pm = NULL ;
	const char	*sn = NULL ;
	const char	*pr = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_smesg: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

#if	CF_DEBUGN
	nprintf(NDF,"main: ent\n") ;
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
	pip->f.logprog = OPT_LOGPROG ;

	pip->lip = lip ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,0) ;
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
		const int ach = MKCHAR(argp[1]) ;

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

/* program-mode */
	                case argopt_pm:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pm = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pm = argp ;
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

/* argument-list file */
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

/* device directory */
	                case argopt_dd:
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->devdname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    break ;

/* handle all keyword defaults */
	                case argopt_dev:
	                case argopt_line:
	                case argopt_td:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            lip->termdev = argp ;
			    } else
	                        rs = SR_INVALID ;
	                    break ;

	                case argopt_notes:
			    lip->final.notes = TRUE ;
			    lip->have.notes = TRUE ;
			    lip->f.notes = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
				    lip->f.notes = (rs > 0) ;
				}
			    }
	                    break ;

	                case argopt_max:
			    cp = NULL ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
				    cl = avl ;
				}
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                cp = argp ;
					cl = argl ;
				    }
				} else
	                            rs = SR_INVALID ;
	                    }
			    if ((rs >= 0) && (cp != NULL)) {
				rs = optvalue(cp,cl) ;
				lip->nmax = rs ;
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

/* terminal-type */
	                    case 'T':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->termtype = argp ;
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* biff */
	                    case 'b':
	                        lip->newmesgval = 1 ;
	                        lip->newbiffval = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
					rs = optbool(avp,avl) ;
	                                lip->newbiffval = (rs > 0) ;
				    }
	                        }
	                        break ;

/* device base directory */
	                    case 'd':
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                lip->devdname = avp ;
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl)
	                                    lip->devdname = argp ;
				    } else
	                                rs = SR_INVALID ;
	                        }
	                        break ;

			    case 'm':
				lip->f.msgs = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
					lip->f.msgs = (rs > 0) ;
				    }
	                        }
				break ;

/* say "NO" */
	                    case 'n':
	                        lip->newmesgval = 0 ;
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

/* quiet mode */
	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* reverse sort */
			    case 'r':
				if (! lip->final.sort) {
	    			    lip->final.sort = TRUE ;
	    			    lip->have.sort = TRUE ;
				    lip->typesort = typesort_reverse ;
				    lip->f.sort = TRUE ;
				}
				break ;

/* sort mode */
			    case 's':
				lip->have.sort = TRUE ;
				cp = NULL ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                cp = avp ;
					cl = avl ;
				    }
	                        }
				if ((rs >= 0) && (cp != NULL)) {
				    rs = locinfo_setsort(lip,cp,cl) ;
				}
	                        break ;

/* terminal device */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = locinfo_typesadds(lip,argp,argl) ;
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

/* width (columns) */
	                    case 'w':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->have.linelen = TRUE ;
	                                lip->final.linelen = TRUE ;
	                                rs = optvalue(argp,argl) ;
	                                lip->linelen = rs ;
	                            }
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* say "YES" */
	                    case 'y':
	                        lip->newmesgval = 1 ;
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

	} /* end while (all command line argument procfileing) */

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
	    debugprintf("b_smesg: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: builtin=%u\n" ;
	    shio_printf(pip->efp,fmt,pn,(contextp != NULL)) ;
	}

/* figure out a program mode */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_smesg: specified pm=%s\n",pm) ;
#endif

	if (pm == NULL) pm = pip->progname ;

	if (lip->f.msgs) {
	    pip->progmode = progmode_notes ;
	} else {
	    int	progmode = matostr(progmodes,1,pm,-1) ;
	    if (progmode < 0) progmode = 0 ;
	    if (sn == NULL) sn = progmodes[progmode] ;
	    pip->progmode = progmode ;
	}
	if (pip->debuglevel > 0) {
	    const int	pm = pip->progmode ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: pm=%s(%u)\n" ;
	    shio_printf(pip->efp,fmt,pn,progmodes[pm],pm) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_smesg: progmode=%u\n",pip->progmode) ;
#endif

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
	    shio_printf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
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

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

#ifdef	COMMENT
	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;
#endif

	if ((lip->nmax == 0) && (argval != NULL)) {
	    if ((rs = optvalue(argval,-1)) >= 0) {
		lip->nmax = rs ;
	    }
	}

	if (rs >= 0) {
	    if ((rs = procopts(pip,&akopts)) >= 0) {
	    	rs = locinfo_defs(lip) ;
	    }
	}

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    USERINFO	u ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
		if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
		    if ((rs = proglog_begin(pip,&u)) >= 0) {
			if ((rs = proclog_info(pip)) >= 0) {
			    if ((rs = proguserlist_begin(pip)) >= 0) {
			        if ((rs = locinfo_noteadm(lip)) >= 0) {
				    ARGINFO	*aip = &ainfo ;
				    BITS	*bop = &pargs ;
				    cchar	*ofn = ofname ;
				    cchar	*afn = afname ;
		    	            rs = process(pip,aip,bop,ofn,afn) ;
		                } /* end if (locinfo_noteadm) */
			        rs1 = proguserlist_end(pip) ;
			        if (rs >= 0) rs = rs1 ;
			    } /* end if (proguserlist) */
			} /* end if (proclog_info) */
			rs1 = proglog_end(pip) ;
			if (rs >= 0) rs = rs1 ;
		    } /* end if (proglog) */
		    rs1 = procuserinfo_end(pip) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (procuserinfo) */
		rs1 = userinfo_finish(&u) ;
		if (rs >= 0) rs = rs1 ;
	    } else {
	            cchar	*pn = pip->progname ;
	            cchar	*fmt = "%s: userinfo failure (%d)\n" ;
	            ex = EX_NOUSER ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	    } /* end if (userinfo) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	}

	if ((rs >= 0) && (lip->oldmesgval == 0)) {
	    ex = 1 ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	        switch (rs) {
	        case SR_INVALID:
	            ex = EX_USAGE ;
		    break ;
	        case SR_PERM:
	        case SR_ACCESS:
		    ex = EX_NOPERM ;
		    break ;
	        default:
	    	    ex = mapex(mapexs,rs) ;
		    break ;
	        } /* end switch */
	    if (! pip->f.quiet) {
		cchar	*pn = pip->progname ;
		cchar	*fmt ;
		fmt = "%s: could not process (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    }
	} else if (rs >= 0) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

retearly:
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("b_smesg/main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    shio_close(pip->efp) ;
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
	shio_printf(pip->efp,
	    "%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (mainsub) */


#if	CF_PERCACHE
/* execute this on module (shared-object) un-load */
void ourfini()
{
	percache_fini(&pc) ;
}
/* end subroutine (ourfini) */
#endif /* CF_PERCACHE */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-y|-n] [-b[={y|n}]] [-dev <terminal>] \n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-m[=<max>]] -<max> [<fromuser(s)>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case akoname_owner:
	                    if (! lip->final.owner) {
	                        lip->have.owner = TRUE ;
	                        lip->final.owner = TRUE ;
	                        lip->f.owner = TRUE ;
	                        if (vl > 0) {
			            rs = optbool(vp,vl) ;
	                            lip->f.owner = (rs > 0) ;
			        }
	                    }
	                    break ;
	                case akoname_sort:
			    lip->have.sort = TRUE ;
			    rs = locinfo_setsort(lip,vp,vl) ;
			    break ;
			case akoname_line:
			    if (lip->termdev == NULL) {
				if (vl > 0) {
				    cchar	**vpp = &lip->termdev ;
				    rs = locinfo_setentry(lip,vpp,vp,vl) ;
				}
			    }
			    break ;
			case akoname_notes:
	                    if (! lip->final.notes) {
	                        lip->have.notes = TRUE ;
	                        lip->final.notes = TRUE ;
	                        lip->f.notes = TRUE ;
	                        if (vl > 0) {
			            rs = optbool(vp,vl) ;
				    lip->f.notes = (rs > 0) ;
				}
			    }
			    break ;
			case akoname_max:
	                    if (! lip->final.max) {
	                        lip->final.max = TRUE ;
	                        if (vl > 0) {
			            rs = optvalue(vp,vl) ;
				    lip->nmax = rs ;
				}
			    }
			    break ;
	                case akoname_date:
	                    if (! lip->final.datelong) {
	                        lip->final.datelong = TRUE ;
	                        lip->have.datelong = TRUE ;
	                        lip->f.datelong = TRUE ;
	                        if (vl) {
				    rs = locinfo_optdate(lip,vp,vl) ;
	                        }
	                    }
	                    break ;
	                } /* end switch */

	                c += 1 ;
	            } else
			rs = SR_INVALID ;

		    if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
	    {
	        switch (pip->progmode) {
	        case progmode_smesg:
	            rs = procmesg(pip,ofp) ;
		    break ;
	        case progmode_notes:
	            rs = procnotes(pip,aip,bop,ofp,afn) ;
		    break ;
	        } /* end switch */
	        wlen += rs ;
	    }
	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_smesg/process: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


/* process (sort out) this mesg */
static int procmesg(PROGINFO *pip,SHIO *ofp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs = locinfo_termdevbegin(lip)) >= 0) {
	    struct ustat	sb ;
	    const char		*termdev = lip->termdev ;
	    if ((rs = uc_stat(termdev,&sb)) >= 0) {
	        rs = procmesgout(pip,ofp,termdev,&sb) ;
		wlen += rs ;
	    } /* end if (stat) */
	    rs1 = locinfo_termdevend(lip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (locinfo-term) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmesg) */


static int procmesgout(PROGINFO *pip,SHIO *ofp,cchar *td,struct ustat *usbp)
{
	LOCINFO		*lip = pip->lip ;
	mode_t		newmode = usbp->st_mode ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	int		f_mesg = FALSE ;
	int		f_biff = FALSE ;

/* get the current state */

	f_mesg = (usbp->st_mode & S_IWGRP) ;
	f_biff = (usbp->st_mode & S_IEXEC) ;

	lip->oldmesgval = f_mesg ;

	if ((lip->newmesgval < 0) && (lip->newstate != NULL)) {
	    const int	ch = MKCHAR(lip->newstate[0]) ;
	    lip->newmesgval = (tolc(ch) == 'y') ? 1 : 0 ;
	}

/* switch it if called for */

	if (lip->newmesgval >= 0) {
	    if (lip->newmesgval > 0) {
	        newmode |= S_IWGRP ;
	    } else {
	        newmode &= (~ S_IWGRP) ;
	    }
	}

	if (lip->newbiffval >= 0) {
	    if (lip->newbiffval > 0) {
	        newmode |= S_IXUSR ;
	    } else {
	        newmode &= (~ S_IXUSR) ;
	    }
	}

	if (usbp->st_mode != newmode) {
	    rs = u_chmod(td,newmode) ;
	}

/* optionally print various things out */

	if (rs >= 0) {

	    if (pip->verboselevel == 1) {
	        if ((lip->newmesgval < 0) && (lip->newbiffval < 0)) {
		    if (! lip->have.notes) {
			int	ch = (f_mesg) ? 'y' : 'n' ;
	        	rs = shio_printf(ofp,"is %c\n",ch) ;
	        	wlen += rs ;
		    }
		}
	    } else if (pip->verboselevel >= 2) {
		int	ch ;
		if (rs >= 0) {
		    ch = (f_mesg) ? 'y' : 'n' ;
	            rs = shio_printf(ofp,"messing was %c\n",ch) ;
	            wlen += rs ;
		}
		if (rs >= 0) {
		    ch = (f_biff) ? 'y' : 'n' ;
	            rs = shio_printf(ofp,"biffing was %c\n",ch) ;
	            wlen += rs ;
		}
	        if (rs >= 0) {
		    int	f_notes_prev = FALSE ;
		    int	f_notes_new = FALSE ;
		    int	c = 0 ;
		    int	cmd ;
		    if (lip->have.notes_prev) {
		        f_notes_prev = lip->f.notes_prev ;
		    }
		    cmd = kshlibcmd_notestate ;
		    if ((rs = lib_noteadm(cmd)) > 0) {
			if (! lip->have.notes_prev) {
		    	    f_notes_prev = TRUE ;
			}
		        f_notes_new = TRUE ;
		        cmd = kshlibcmd_notecount ;
			if ((rs = lib_noteadm(cmd)) > 0) {
			    c = rs ;
			}
		    }
#if	CF_DEBUGN
		    nprintf(NDF,
			"main/procmesgout: mid1 rs=%d f_notes=%u c=%u\n",
			rs,f_notes,c) ;
#endif
		    if (rs >= 0) {
			cchar	*fmt = "notices was %c\n" ;
		        ch = (f_notes_prev) ? 'y' : 'n' ;
			if (f_notes_new) {
			    fmt = "notices was %c (%u)\n" ;
			}
			proglog_printf(pip,fmt,ch,c) ;
	                rs = shio_printf(ofp,fmt,ch,c) ;
	                wlen += rs ;
		    }
	        } /* end if (ok) */
	    } /* end if (extra verbose) */

	    if (pip->verboselevel >= 2) {
	        rs = shio_printf(ofp,"terminal=%s\n",td) ;
	        wlen += rs ;
	    }

	} /* end if (printing things out) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmesgout) */


static int procnotes(PROGINFO *pip,ARGINFO *aip,BITS *bop,SHIO *ofp,cchar *afn)
{
	LOCINFO		*lip = pip->lip ;
	const int	cmd = kshlibcmd_notecount ;
	int		rs ;
	int		wlen = 0 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/procnotes: ent\n") ;
#endif
	if ((rs = lib_noteadm(cmd)) > 0) {
	    pip->n = rs ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_smesg/procnotes: num-notes=%u\n",rs) ;
#endif
	    if ((rs = locinfo_notes(lip)) >= 0) {
		if ((rs = procargs(pip,aip,bop,afn)) >= 0) {
		    if ((rs = locinfo_notesarrange(lip)) >= 0) {
		        rs = procnotesout(pip,ofp) ;
			wlen += rs ;
		    } /* end if (locinfo_notesarrange) */
		} /* end if (procargs) */
	    } /* end if (locinfo_notes) */
	} /* end if (lib_notecount) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/procnotes: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procnotes) */


static int procnotesout(PROGINFO *pip,SHIO *ofp)
{
	LOCINFO		*lip = pip->lip ;
	VECHAND		*klp ;
	const int	max = (lip->nmax > 0) ? lip->nmax : INT_MAX ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;
	int		wlen = 0 ;
	LOCNOTE		*onp ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/procnotesout: ent max=%d\n",max) ;
#endif
	klp = &lip->notes ;
	for (i = 0 ; (rs1 = vechand_get(klp,i,&onp)) >= 0 ; i += 1) {
	    if (onp != NULL) {
		c += 1 ;
		rs = procnoteouter(pip,ofp,onp) ;
		wlen += rs ;
	    }
	    if (c >= max) break ;
	    if (rs < 0) break ;
	} /* end for */
	if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/procnotesout: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procnoteeout) */


static int procnoteouter(PROGINFO *pip,SHIO *ofp,LOCNOTE *onp)
{
	LOCINFO		*lip = pip->lip ;
	TMTIME		tm ;
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/procnotsouter: ent t=%u u=%s\n",
		onp->type,onp->user) ;
#endif

	if ((rs = tmtime_localtime(&tm,onp->stime)) >= 0) {
	    const int	tlen = TIMEBUFLEN ;
	    cchar	*fmt = "%c %s %s « %t\n" ;
	    cchar	*tf = "%R" ;
	    char	tbuf[TIMEBUFLEN+1] ;
	    if (lip->f.datelong) tf = "%Y%b%d %R" ;
	    if ((rs = sntmtime(tbuf,tlen,&tm,tf)) >= 0) {
	        int	tl = (lip->linelen-11) ; /* default is eleven */
		int	pl ;
	        int	tch ;

		if (lip->f.datelong) tl -= 10 ; /* additional ten for "long" */
		tl -= strlen(onp->user) ;
		tch = (onp->type < 4) ? types[onp->type] : '¿' ;
		pl = MIN(onp->nlen,tl) ;
		rs = shio_printf(ofp,fmt,tch,tbuf,onp->user,onp->nbuf,pl) ;
		wlen += rs ;

	    } /* end if (sntmtime) */
	} /* end if (tmtime_localtime) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/procnoteouter: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procnoteouter) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn)
{
	OSETSTR		ss ;
	const int	n = 20 ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((rs = osetstr_start(&ss,n)) >= 0) {
	    int		pan = 0 ;
	    int		cl ;
	    cchar	*cp ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;

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
	                    rs = procloadname(pip,&ss,cp,-1) ;
			}
		    }

	            if (rs < 0) break ;
	        } /* end for (handling positional arguments) */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        SHIO	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0)
	            afn = STDINFNAME ;

	        if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

			if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
			    if (cp[0] != '#') {
			        pan += 1 ;
			        rs = procloadnames(pip,&ss,cp,cl) ;
			    }
			}

		        if (rs >= 0) rs = lib_sigterm() ;
		        if (rs >= 0) rs = lib_sigintr() ;
	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = shio_close(afp) ;
		    if (rs >= 0) rs = rs1 ;
	        } else {
	            if (! pip->f.quiet) {
			fmt = "%s: inaccessible argument-list (%d)\n" ;
	                shio_printf(pip->efp,fmt,pn,rs) ;
	                shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	            }
	        } /* end if */

	    } /* end if (processing file argument file list) */

	    if (rs >= 0) {
		if (pip->debuglevel > 0) {
		    fmt = "%s: restricted users=%u\n" ;
		    shio_printf(pip->efp,fmt,pn,pan) ;
		}
		if ((rs = procargers(pip,&ss)) >= 0) {
		    if (pip->debuglevel > 0) {
		        fmt = "%s: messages selected=%u\n" ;
		        shio_printf(pip->efp,fmt,pn,rs) ;
		    }
		}
	    } /* end if (ok) */

	    rs1 = osetstr_finish(&ss) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (osetstr) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procargers(PROGINFO *pip,OSETSTR *osp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/procargers: ent\n") ;
#endif
	if ((rs = locinfo_userbegin(lip,osp)) >= 0) {
	    KSHLIB_NOTE	kn ;
	    int		mi ;
	    for (mi = 0 ; (rs = lib_noteread(&kn,mi)) > 0 ; mi += 1) {
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/procargers: lib_noteread() rs=%d\n",rs) ;
#endif
	        if ((rs = locinfo_typesmatch(lip,kn.type)) > 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/procargers: locinfo_typesmatch() rs=%d\n",rs) ;
#endif
		    if ((rs = locinfo_usermatch(lip,osp,kn.user)) > 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/procargers: locinfo_usermatch() rs=%d\n",rs) ;
#endif
			c += 1 ;
	                rs = locinfo_noteload(lip,&kn) ;
		    }
	        }
	        if (rs < 0) break ;
	    } /* end for */
	    rs1 = locinfo_userend(lip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (locinfo-user) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/procargers: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procargers) */


static int procloadnames(PROGINFO *pip,OSETSTR *nlp,cchar *sp,int sl)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;

	if (nlp == NULL) return SR_FAULT ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    int		fl ;
	    const char	*fp ;

	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procloadname(pip,nlp,fp,fl) ;
		    c += rs ;
		}
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */

	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procloadnames) */


static int procloadname(PROGINFO *pip,OSETSTR *nlp,cchar np[],int nl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	if (np == NULL) return SR_FAULT ;

	if (np[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/procloadname: ent rn=%t\n",np,nl) ;
#endif

	if (nl < 0) nl = strlen(np) ;

	if ((np[0] == '\0') || hasMeAlone(np,nl)) {
	    if ((rs = locinfo_username(lip)) >= 0) {
	        np = lip->unbuf ;
	        nl = rs ;
	        rs = osetstr_add(nlp,np,nl) ;
	        c += rs ;
	    } /* end if (locinfo_username) */
	} else {
	    const int	nch = MKCHAR(np[0]) ;
	    cchar	*tp ;
	    if ((tp = strnchr(np,nl,'+')) != NULL) {
	        nl = (tp-np) ;
	    }
	    if (strnchr(np,nl,'.') != NULL) {
	        LOCINFO_RNCUR	rnc ;
	        if ((rs = locinfo_rncurbegin(lip,&rnc)) >= 0) {
	            if ((rs = locinfo_rnlook(lip,&rnc,np,nl)) > 0) {
	                const int	ul = USERNAMELEN ;
	                char		ub[USERNAMELEN+1] ;
	                while ((rs = locinfo_rnread(lip,&rnc,ub,ul)) > 0) {
	                    rs = osetstr_add(nlp,ub,rs) ;
	        	    c += rs ;
	                    if (rs < 0) break ;
	                } /* end while (reading entries) */
	            } /* end if (locinfo_rnlook) */
	            rs1 = locinfo_rncurend(lip,&rnc) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (srncursor) */
	    } else if (nch == MKCHAR('¡')) {
	        LOCINFO_GMCUR	gc ;
	        cchar		*gnp = (np+1) ;
	        int		gnl = (nl-1) ;
		if (gnl == 0) {
		    rs = locinfo_groupname(lip) ;
		    gnl = rs ;
		    gnp = lip->gnbuf ;
		}
		if (rs >= 0) {
	            if ((rs = locinfo_gmcurbegin(lip,&gc)) >= 0) {
	                if ((rs = locinfo_gmlook(lip,&gc,gnp,gnl)) > 0) {
	                    const int	ul = USERNAMELEN ;
	                    char	ub[USERNAMELEN+1] ;
	                    while ((rs = locinfo_gmread(lip,&gc,ub,ul)) > 0) {
	                        rs = osetstr_add(nlp,ub,rs) ;
	        		c += rs ;
	                        if (rs < 0) break ;
	                    } /* end while */
	                } /* end if */
	                rs1 = locinfo_gmcurend(lip,&gc) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (gmcursor) */
		} /* end if (ok) */
	    } else {
	        if (nch == '!') {
	            np += 1 ;
	            nl -= 1 ;
	        }
		if (nl == 0) {
	    	    if ((rs = locinfo_username(lip)) >= 0) {
	        	np = lip->unbuf ;
			nl = rs ;
		    }
		}
	        if ((rs >= 0) && (nl > 0)) {
	            rs = osetstr_add(nlp,np,nl) ;
	            c += rs ;
		}
	    } /* end if */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/procloadname: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procloadname) */


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->username = uip->username ;
	pip->gecosname = uip->gecosname ;
	pip->realname = uip->realname ;
	pip->name = uip->name ;
	pip->fullname = uip->fullname ;
	pip->mailname = uip->mailname ;
	pip->org = uip->organization ;
	pip->logid = uip->logid ;
	pip->pid = uip->pid ;
	pip->uid = uip->uid ;
	pip->euid = uip->euid ;
	pip->gid = uip->gid ;
	pip->egid = uip->egid ;

#ifdef	COMMENT
	if (rs >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    char	hbuf[MAXHOSTNAMELEN+1] ;
	    cchar	*nn = pip->nodename ;
	    cchar	*dn = pip->domainname ;
	    if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	        cchar	**vpp = &pip->hostname ;
	        rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	    }
	}
#endif /* COMMENT */

	if (rs >= 0) {
	    rs = procuserinfo_logid(pip) ;
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	return rs ;
}
/* end subroutine (procuserinfo_end) */


static int procuserinfo_logid(PROGINFO *pip)
{
	int		rs ;
	if ((rs = lib_runmode()) >= 0) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("procuserinfo_logid: rm=\\b%04ß\n",rs) ;
#endif
	    if (rs & KSHLIB_RMKSH) {
	        if ((rs = lib_serial()) >= 0) {
	            const int	s = rs ;
	            const int	plen = LOGIDLEN ;
	            const int	pv = pip->pid ;
	            cchar	*nn = pip->nodename ;
	            char	pbuf[LOGIDLEN+1] ;
	            if ((rs = mkplogid(pbuf,plen,nn,pv)) >= 0) {
	                const int	slen = LOGIDLEN ;
	                char		sbuf[LOGIDLEN+1] ;
	                if ((rs = mksublogid(sbuf,slen,pbuf,s)) >= 0) {
	                    cchar	**vpp = &pip->logid ;
	                    rs = proginfo_setentry(pip,vpp,sbuf,rs) ;
	                }
	            }
	        } /* end if (lib_serial) */
	    } /* end if (runmode-KSH) */
	} /* end if (lib_runmode) */
	return rs ;
}
/* end subroutine (procuserinfo_logid) */


static int proclog_info(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	cchar		*ps = progmodes[pip->progmode] ;
	{
	    const int	f = MKBOOL(lip->f.notes) ;
	    cchar	*fmt = "pm=%s" ;
	    if (lip->have.notes) {
	        fmt = "pm=%s notes=%u" ;
	    }
	    proglog_printf(pip,fmt,ps,f) ;
	}
	return rs ;
}
/* end subroutine (proclog_info) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->to = -1 ;
	lip->newmesgval = -1 ;
	lip->newbiffval = -1 ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	rs1 = locinfo_notesend(lip) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = locinfo_typesend(lip) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_PERCACHE /* register |ourfini()| for mod-unload */
	if (lip->f.percache) {
	    if ((rs1 = percache_finireg(&pc)) > 0) { /* need registration? */
	        rs1 = uc_atexit(ourfini) ;
	    }
	    if (rs >= 0) rs = rs1 ;
	}
#endif /* CF_PERCACHE */

	lip->unbuf[0] = '\0' ;
	if (lip->open.rn) {
	    lip->open.rn = FALSE ;
	    rs1 = sysrealname_close(&lip->rn) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.gm) {
	    lip->open.gm = FALSE ;
	    rs1 = grmems_finish(&lip->gm) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


#if	CF_LOCSETENT
int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar *vp,int vl)
{
	VECSTR		*slp ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	slp = &lip->stores ;
	if (! lip->open.stores) {
	    rs = vecstr_start(slp,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
		oi = vecstr_findaddr(slp,*epp) ;
	    }
	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(slp,vp,len,epp) ;
	    } else {
	        *epp = NULL ;
	    }
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(slp,oi) ;
	    }
	} /* end if (ok) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */
#endif /* CF_LOCSETENT */


static int locinfo_defs(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip->devdname == NULL) {
	    lip->devdname = DEVDNAME ;
	}

	if (lip->termtype == NULL) {
	    const char	*varterm = VARTERM ;
	    lip->termtype = getourenv(pip->envv,varterm) ;
	}

	if (lip->linelen == 0) {
	    cchar	*cp ;
	    if ((cp = getourenv(pip->envv,VARCOLUMNS)) != NULL) {
		rs = optvalue(cp,-1) ;
		lip->linelen = rs ;
	    }
	}

	if (lip->linelen == 0) lip->linelen = COLUMNS ;

	return rs ;
}
/* end subroutine (locinfo_defs) */


static int locinfo_username(LOCINFO *lip)
{
	int		rs ;
	if (lip == NULL) return SR_FAULT ;
	if (lip->unbuf[0] == '\0') {
	    rs = getusername(lip->unbuf,USERNAMELEN,-1) ;
	} else {
	    rs = strlen(lip->unbuf) ;
	}
	return rs ;
}
/* end subroutine (locinfo_username) */


static int locinfo_groupname(LOCINFO *lip)
{
	int		rs ;
	if (lip == NULL) return SR_FAULT ;
	if (lip->gnbuf[0] == '\0') {
	    rs = getgroupname(lip->gnbuf,GROUPNAMELEN,-1) ;
	} else {
	    rs = strlen(lip->gnbuf) ;
	}
	return rs ;
}
/* end subroutine (locinfo_groupname) */


static int locinfo_gmcurbegin(LOCINFO *lip,LOCINFO_GMCUR *curp)
{
	int		rs = SR_OK ;

	if (curp == NULL) return SR_FAULT ;

	if (! lip->open.gm) {
	    const int	max = 20 ;
	    const int	ttl = (12*3600) ;
	    rs = grmems_start(&lip->gm,max,ttl) ;
	    lip->open.gm = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = grmems_curbegin(&lip->gm,&curp->gmcur) ;
	}

	return rs ;
}
/* end subroutine (locinfo_gmcurbegin) */


static int locinfo_gmcurend(LOCINFO *lip,LOCINFO_GMCUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (curp == NULL) return SR_FAULT ;

	rs1 = grmems_curend(&lip->gm,&curp->gmcur) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_gmcurend) */


static int locinfo_gmlook(LOCINFO *lip,LOCINFO_GMCUR *curp,cchar *gnp,int gnl)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (gnp == NULL) return SR_FAULT ;

	if ((rs = grmems_lookup(&lip->gm,&curp->gmcur,gnp,gnl)) >= 0) {
	    rs = 1 ;
	} else if (rs == rsn) {
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (locinfo_gmlook) */


static int locinfo_gmread(LOCINFO *lip,LOCINFO_GMCUR *curp,char *ubuf,int ulen)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;

	if ((rs = grmems_lookread(&lip->gm,&curp->gmcur,ubuf,ulen)) == rsn) {
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (locinfo_gmread) */


static int locinfo_rncurbegin(LOCINFO *lip,LOCINFO_RNCUR *curp)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (curp == NULL) return SR_FAULT ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/locinfo_rncurbegin: ent\n") ;
#endif
	if (! lip->open.rn) {
	    rs = sysrealname_open(&lip->rn,NULL) ;
	    lip->open.rn = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = sysrealname_curbegin(&lip->rn,&curp->rncur) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/locinfo_rncurbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_rncurbegin) */


static int locinfo_rncurend(LOCINFO *lip,LOCINFO_RNCUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (curp == NULL) return SR_FAULT ;

	rs1 = sysrealname_curend(&lip->rn,&curp->rncur) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_rncurend) */


static int locinfo_rnlook(LOCINFO *lip,LOCINFO_RNCUR *curp,cchar *gnp,int gnl)
{
	PROGINFO	*pip = lip->pip ;
	const int	rsn = SR_NOTFOUND ;
	const int	fo = 0 ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (gnp == NULL) return SR_FAULT ;

	if (pip == NULL) return SR_FAULT ;

	if ((rs = sysrealname_look(&lip->rn,&curp->rncur,fo,gnp,gnl)) >= 0) {
	    rs = 1 ;
	} else if (rs == rsn) {
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/locinfo_rnlook: sysrealname_look() rs=%d\n",
		rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_rnlook) */


static int locinfo_rnread(LOCINFO *lip,LOCINFO_RNCUR *curp,char ubuf[],int ulen)
{
	PROGINFO	*pip = lip->pip ;
	const int	rsn = SR_NOTFOUND ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;
	if (pip == NULL) return SR_FAULT ;

	if ((ulen >= 0) && (ulen < USERNAMELEN)) return SR_OVERFLOW ;

	if ((rs = sysrealname_lookread(&lip->rn,&curp->rncur,ubuf)) == rsn) {
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/locinfo_rnread: sysrealname_lookread() rs=%d\n",
		rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_rnread) */


static int locinfo_typesadds(LOCINFO *lip,cchar *argp,int argl)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (argl < 0) argl = strlen(argp) ;
	if (argl > 0) {
	    int		sl = argl ;
	    const char	*sp = argp ;
	    const char	*tp ;
	    while ((tp = strnpbrk(sp,sl," ,\t ")) != NULL) {
		if ((tp-sp) > 0) {
		    rs = locinfo_typesadder(lip,sp,(tp-sp)) ;
		    c += rs ;
		}
		sl -= ((tp+1)-sp) ;
		sp = (tp+1) ;
		if (rs < 0) break ;
	    } /* end while */
	    if ((rs >= 0) && sl) {
		rs = locinfo_typesadder(lip,sp,sl) ;
		c += rs ;
	    }
	} /* end if (positive) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_typesadds) */


static int locinfo_typesadder(LOCINFO *lip,const char *sp,int sl)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (sl < 0) sl = strlen(sp) ;
	if (sl > 0) {
	    if ((rs = locinfo_types(lip)) >= 0) {
		VECINT	*ntp = &lip->types ;
	        int	nt = 0 ;
	        if (hasalldig(sp,sl)) {
	            if ((rs = optvalue(sp,sl)) >= 0) {
		        nt = rs ;
		        rs = vecint_adduniq(ntp,nt) ;
		        if (rs < INT_MAX) c += 1 ;
	            }
	        } else {
		    if ((nt = matostr(ntypes,1,sp,sl)) >= 0) {
		        rs = vecint_adduniq(ntp,nt) ;
		        if (rs < INT_MAX) c += 1 ;
		    } /* end if (valid note-type) */
	        }
	    } /* end if (locinfo_typesbegin) */
	} /* end if (positive) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_typesadder) */


static int locinfo_types(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (! lip->open.types) {
	    rs = locinfo_typesbegin(lip) ;
	}
	return rs ;
}
/* end subroutine (locinfo_types) */


static int locinfo_typesbegin(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (! lip->open.types) {
	    if ((rs = vecint_start(&lip->types,2,0)) >= 0) {
		lip->open.types = TRUE ;
	    }
	}
	return rs ;
}
/* end subroutine (locinfo_typesbegin) */


static int locinfo_typesend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.types) {
	    lip->open.types = FALSE ;
	    rs1 = vecint_finish(&lip->types) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (locinfo_typesend) */


#if	CF_TYPESCOUNT
static int locinfo_typescount(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (lip->open.types) {
	    rs = vecint_count(&lip->types) ;
	}
	return rs ;
}
/* end subroutine (locinfo_typescount) */
#endif /* CF_TYPESCOUNT */


static int locinfo_typesmatch(LOCINFO *lip,int v)
{
	int		rs = SR_OK ;
	int		f = TRUE ;
	if (lip->open.types) {
	    f = FALSE ;
	    if ((rs = vecint_match(&lip->types,v)) > 0) {
		f = TRUE ;
	    }
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (locinfo_typesmatch) */


static int locinfo_termdevbegin(LOCINFO *lip)
{
	int		rs = SR_OK ;

	if (lip->termdev == NULL) {
	    const int	ullen = MAXPATHLEN ;
	    char	ulbuf[MAXPATHLEN+1] ;
	    if ((rs = getutmpline(ulbuf,ullen,0)) >= 0) {
	        cchar	*devdname = lip->devdname ;
	        char	tbuf[MAXPATHLEN+1] ;
	        if ((rs = mkpath2(tbuf,devdname,ulbuf)) >= 0) {
		    cchar	**vpp = &lip->termdev ;
		    rs = locinfo_setentry(lip,vpp,tbuf,rs) ;
		} /* end if (adding device directory prefix) */
	    } /* end if (getutmpline) */
	} else {
	   if (strncmp(lip->termdev,"/dev",4) != 0) {
	        cchar	*devdname = lip->devdname ;
		cchar	*td = lip->termdev ;
	        char	tbuf[MAXPATHLEN+1] ;
	        if ((rs = mkpath2(tbuf,devdname,td)) >= 0) {
		    cchar	**vpp = &lip->termdev ;
		    rs = locinfo_setentry(lip,vpp,tbuf,rs) ;
		} /* end if (adding device directory prefix) */
	   }
	} /* end if */

	return rs ;
}
/* end subroutine (locinfo_termdevbegin) */


static int locinfo_termdevend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (lip == NULL) return SR_FAULT ;
	return rs ;
}
/* end subroutine (locinfo_termdevend) */


static int locinfo_notesbegin(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (! lip->open.notes) {
	    if ((rs = vechand_start(&lip->notes,0,0)) >= 0) {
		lip->open.notes = TRUE ;
	    }
	}
	return rs ;
}
/* end subroutine (locinfo_notesbegin) */


static int locinfo_notesend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.notes) {
	    rs1 = locinfo_notesfins(lip) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->open.notes = FALSE ;
	    rs1 = vechand_finish(&lip->notes) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (locinfo_notesend) */


static int locinfo_notesfins(LOCINFO *lip)
{
	vechand		*nlp = &lip->notes ;
	LOCNOTE		*onp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	for (i = 0 ; (rs1 = vechand_get(nlp,i,&onp)) >= 0 ; i += 1) {
	    if (onp != NULL) {
		rs1 = locnote_finish(onp) ;
		if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */
	return rs ;
}
/* end subroutine (locinfo_notesfins) */


static int locinfo_notes(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (! lip->open.notes) {
	    rs = locinfo_notesbegin(lip) ;
	}
	return rs ;
}
/* end subroutine (locinfo_notes) */


static int locinfo_noteload(LOCINFO *lip,KSHLIB_NOTE *knp)
{
	PROGINFO	*pip = lip->pip ;
	LOCNOTE		*onp ;
	const int	osize = sizeof(LOCNOTE) ;
	int		rs ;
	if (pip == NULL) return SR_FAULT ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/locinfo_noteload: ent\n") ;
#endif
	if ((rs = uc_malloc(osize,&onp)) >= 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/locinfo_noteload: locnote_start()\n") ;
#endif
	    if ((rs = locnote_start(onp,knp)) >= 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/locinfo_noteload: vechand_add()\n") ;
#endif
	        rs = vechand_add(&lip->notes,onp) ;
		if (rs < 0)
		    locnote_finish(onp) ;
	    }
	    if (rs < 0)
		uc_free(onp) ;
	} /* end if (m-a) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/locinfo_noteload: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_noteload) */


static int locinfo_notesarrange(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (lip->open.notes && lip->have.sort) {
	    vcmpfun_t	vcmp = vcmpfor ;
	    switch (lip->typesort) {
	    case typesort_forward:
		vcmp = vcmpfor ;
		break ;
	    case typesort_reverse:
		vcmp = vcmprev ;
		break ;
	    } /* end switch */
	    rs = vechand_sort(&lip->notes,vcmp) ;
	} /* end if (notes and sorting requested) */
	return rs ;
}
/* end subroutine (locinfo_notesarrange) */


static int locinfo_userbegin(LOCINFO *lip,OSETSTR *osp)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/locinfo_userbegin: ent\n") ;
#endif
	if ((rs = osetstr_count(osp)) > 0) {
	    lip->nusers = rs ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/locinfo_userbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_userbegin) */


static int locinfo_userend(LOCINFO *lip)
{
	if (lip == NULL) return SR_FAULT ;
	lip->nusers = 0 ;
	return SR_OK ;
}
/* end subroutine (locinfo_userend) */


static int locinfo_usermatch(LOCINFO *lip,OSETSTR *osp,cchar *un)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		f = TRUE ;
	if (pip == NULL) return SR_FAULT ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	debugprintf("b_smesg/locinfo_usermatch: ent u=%s\n",un) ;
	debugprintf("b_smesg/locinfo_usermatch: nusers=%u\n",lip->nusers) ;
	}
#endif
	if (lip->nusers > 0) {
	    f = FALSE ;
	    if ((rs = osetstr_already(osp,un,-1)) > 0) {
		f = TRUE ;
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_smesg/locinfo_usermatch: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (locinfo_usermatch) */


static int locinfo_noteadm(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_smesg/locinfo_noteadm: ent have-notes=%u\n",
		lip->have.notes) ;
#endif
	if (lip->have.notes) {
	    const int	si = MKBOOL(lip->f.notes) ;
	    int		ncmd ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_smesg/locinfo_noteadm: si=%u\n",si) ;
#endif
	    switch (si) {
	    case 0:
		ncmd = kshlibcmd_noteoff ;
		break ;
	    case 1:
		ncmd = kshlibcmd_noteon ;
		break ;
	    } /* end switch */
	    if ((rs = lib_noteadm(ncmd)) >= 0) {
		lip->have.notes_prev = TRUE ;
		lip->f.notes_prev = (rs > 0) ;
		if (pip->debuglevel > 0) {
		    cchar	*pn = pip->progname ;
		    cchar	*fmt = "%s: note-adm cmd=%u (%d)\n" ;
		    shio_printf(pip->efp,fmt,pn,ncmd,rs) ;
		}
		rs = 1 ;
	    }
	} /* end if (have-notes) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_smesg/locinfo_noteadm: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_noteadm) */


static int locinfo_setsort(LOCINFO *lip,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	if (! lip->final.sort) {
	    int		v ;
	    lip->have.sort = TRUE ;
	    if (vl < 0) vl = strlen(vp) ;
	    if (vl > 0) {
	        lip->final.sort = TRUE ;
	        if ((v = matostr(typesorts,1,vp,vl)) >= 0) {
	            lip->typesort = v ;
		    lip->f.sort = (v>0) ;
	        } else {
		    rs = SR_INVALID ;
	        }
	    } /* end if (positive) */
	}
	return rs ;
}
/* end subroutine (locinfo_setsort) */


static int locinfo_optdate(LOCINFO *lip,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	if (vl > 0) {
	    int	oi ;
	    if ((oi = matpstr(datetypes,1,vp,vl)) >= 0) {
		switch (oi) {
		case datetype_long:
		    lip->f.datelong = TRUE ;
		    break ;
		} /* end switch */
	    } /* end if (matpstr) */
	} /* end if (have option-value) */
	return rs ;
}
/* end subroutine (locinfo_optdate) */


static int locnote_start(LOCNOTE *onp,KSHLIB_NOTE *knp)
{
	const int	ulen = SESMSG_USERLEN ;
	int		rs ;
	cchar		*cp ;
	onp->type = knp->type ;
	onp->stime = knp->stime ;
	strwcpy(onp->user,knp->user,ulen) ;
	if ((rs = uc_mallocstrw(knp->nbuf,knp->nlen,&cp)) >= 0) {
	    onp->nlen = knp->nlen ;
	    onp->nbuf = cp ;
	}
	return rs ;
}
/* end subroutine (locnote_start) */


static int locnote_finish(LOCNOTE *onp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (onp->nbuf != NULL) {
	    rs1 = uc_free(onp->nbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    onp->nbuf = NULL ;
	}
	return rs ;
}
/* end subroutine (locnote_finish) */


static int vcmpfor(const void *v1pp,const void *v2pp)
{
	LOCNOTE		**e1pp = (LOCNOTE **) v1pp ;
	LOCNOTE		**e2pp = (LOCNOTE **) v2pp ;
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = (+ ((*e1pp)->stime - (*e2pp)->stime)) ;
	        } else
		    rc =  -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (vcmpfor) */


static int vcmprev(const void *v1pp,const void *v2pp)
{
	LOCNOTE		**e1pp = (LOCNOTE **) v1pp ;
	LOCNOTE		**e2pp = (LOCNOTE **) v2pp ;
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = (- ((*e1pp)->stime - (*e2pp)->stime)) ;
	        } else
		    rc =  -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (vcmprev) */


