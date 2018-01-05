/* b_s (s) */
/* language=C89 */

/* KSH built-in version of 's(1d)' */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_DEBUGN	0		/* special debugging */
#define	CF_DEBUGSD	0		/* debug status-display */
#define	CF_DEBUGHEXBUF	0		/* debug w/ HEX-string output */
#define	CF_LINES25	0		/* 25 lines? */
#define	CF_DATEBLINK	0
#define	CF_BASIC	1		/* basic capabilities */
#define	CF_ENHANCED	1		/* enahanced character attributes */
#define	CF_SR		1		/* allow scroll-region */
#define	CF_VCV		1		/* VT cursor visibility */
#define	CF_ACV		1		/* ANSI cursor visibility */
#define	CF_SCV		1		/* SCREEN visibility */
#define	CF_SCS94	1		/* enable supplementary-char-set-94 */
#define	CF_SCS94A	1		/* enable supplementary-char-set-94a */
#define	CF_SCS96	1		/* enable supplementary-char-set-96 */
#define	CF_CSR		1		/* general cursor save-restore */
#define	CF_VCSR		1		/* VT cursor save-restore */
#define	CF_ACSR		1		/* ANSI cursor save-restore */
#define	CF_ANSICSM	1		/* use ANSI-standard Char-Set-Maps */
#define	CF_TERMMAILNAME	1		/* perform 'termmailname()' */


/* revision history:

	= 2004-06-24, David A­D­ Morano
        I rewrote this from scratch. The previous version of this program was a
        hack.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ s [arguments]


	Notes:

	DCS	ESC P
	ST	ESC \
	CSI	ESC <ch_lbracket>
	LS2R	ESC <ch_rbrace>


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
#include	<termios.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<tmtime.h>
#include	<sntmtime.h>
#include	<sbuf.h>
#include	<termstr.h>
#include	<getxusername.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_s.h"
#include	"defs.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		2048
#endif

#undef	CODEBUFLEN
#define	CODEBUFLEN	40

#undef	MCBUFLEN
#define	MCBUFLEN	(4 + 4)

#undef	FROMBUFLEN
#define	FROMBUFLEN	120

#undef	MAXUSERLEN
#define	MAXUSERLEN	12

#undef	MNBUFLEN
#define	MNBUFLEN	60

#undef	HEXBUFLEN
#define	HEXBUFLEN	100

#define	DEFTERMSPEC	"default"

#define	TCF_MDEFAULT	0x0000	/* default */
#define	TCF_MEC		(1<<1)	/* enhanced character attributes */
#define	TCF_MVCV	(1<<2)	/* cursor visibility (VT) */
#define	TCF_MACV	(1<<3)	/* cursor visibility (ANSI) */
#define	TCF_MSCV	(1<<4)	/* cursor visibility (SCREEN) */
#define	TCF_MPSF	(1<<5)	/* has a preferred supplimental font */
#define	TCF_MSCS94	(1<<6)	/* supplemental character set 94 */
#define	TCF_MSCS96	(1<<7)	/* supplemental character set 96 */
#define	TCF_MSD		(1<<8)	/* has a status display (line) */
#define	TCF_MSCS94A	(1<<9)	/* supplemental character set 94a */
#define	TCF_MSR		(1<<10)	/* has setable line-scrolling regions */
#define	TCF_MSL		(1<<11)	/* setable number of lines */
#define	TCF_MVCSR	(1<<12)	/* cursor save-restore (VT) */
#define	TCF_MACSR	(1<<13)	/* cursor save-restore (ANSI) */
#define	TCF_MACSRS	(1<<14)	/* cursor save-restore (ANSI) is screwed */
#define	TCF_MACL	(1<<15)	/* can take ANSI conformance level 1 */

#define	TCF_MBASIC	(TCF_MSR)
#define	TCF_MVT		(TCF_MSR | TCF_MVCSR)
#define	TCF_MVTE	(TCF_MSR | TCF_MVCSR | TCF_MEC)

#define	TCF_MVTADV	\
	(TCF_MVTE) | \
	(TCF_MPSF | TCF_MSCS94 | TCF_MSCS96 | TCF_MVCV | TCF_MSD) | \
	(TCF_MSL | TCF_MACL)

#define	TCF_MANSI	\
	(TCF_MSR | TCF_MEC | TCF_MACV | TCF_MACSR | TCF_MACSRS | TCF_ACL)

#define	TCF_MSCREEN	\
	(TCF_MVTE | TCF_MVCV | TCF_MACV | TCF_MSCV | TCF_MACSR)

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	TERMTYPE	struct termtype

#define	NDF		"s.deb"


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	sncpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	tcgetlines(int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	termconseq(char *,int,int,int,int,int,int) ;
extern int	mkpr(char *,int,cchar *,cchar *) ;
extern int	pcsmailcheck(cchar *,char *,int,cchar *) ;
extern int	nusers(cchar *) ;
extern int	sbuf_loadstrs(SBUF *,cchar **) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthexblock(cchar *,int,cchar *,int) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strwset(char *,int,int) ;
extern char	*timestr_std(time_t,char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		init:1 ;
	uint		all:1 ;
	uint		sd:1 ;
	uint		home:1 ;
	uint		clear:1 ;
	uint		date:1 ;
	uint		scroll:1 ;
	uint		la:1 ;
	uint		mailcheck:1 ;
	uint		mailfrom:1 ;
	uint		mailsubj:1 ;
	uint		nusers:1 ;
	uint		lines:1 ;
} ;

struct locinfo {
	vecstr		stores ;
	PROGINFO	*pip ;
	cchar		*prpcs ;
	cchar		*utfname ;
	cchar		*termspec ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	int		termflags ;
	int		lines ;
	int		nmsgs ;
	int		mnlen ;
	int		linelen ;
	char		*mnbuf ;
} ;

struct termtype {
	cchar		*name ;
	uint		flags ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,cchar *) ;
static int	processer(PROGINFO *,SHIO *) ;
static int	procmailusers(PROGINFO *) ;
static int	procmailuser(PROGINFO *,cchar *,int) ;
static int	terminit(PROGINFO *,SHIO *,char *,int) ;
static int	termclear(PROGINFO *,char *,int) ;
static int	termdate(PROGINFO *,char *,int,cchar *) ;
static int	termdatesd(PROGINFO *,SBUF *,cchar *) ;
static int	termmailname(PROGINFO *,char *,int,int) ;
static int	bufsd(PROGINFO *,int,SBUF *,int,cchar *,int) ;
static int	bufdiv(PROGINFO *,int,SBUF *,int,int,cchar *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_columns(LOCINFO *) ;
static int	locinfo_prpcs(LOCINFO *,cchar *) ;
static int	locinfo_utfname(LOCINFO *,cchar *) ;
static int	locinfo_termlines(LOCINFO *,SHIO *) ;
static int	locinfo_termflags(LOCINFO *) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"PCSROOT",
	"sn",
	"af",
	"ef",
	"of",
	"utf",
	"lines",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_pcsroot,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_utf,
	argopt_lines,
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

static const char	*akonames[] = {
	"init",
	"all",
	"sd",
	"home",
	"clear",
	"date",
	"scroll",
	"la",
	"mailcheck",
	"mailfrom",
	"mailsubj",
	"nusers",
	"lines",
	NULL
} ;

enum akonames {
	akoname_init,
	akoname_all,
	akoname_sd,
	akoname_home,
	akoname_clear,
	akoname_date,
	akoname_scroll,
	akoname_la,
	akoname_mailcheck,
	akoname_mailfrom,
	akoname_mailsubj,
	akoname_nusers,
	akoname_lines,
	akoname_overlast
} ;

static const TERMTYPE	terms[] = {
	{ "sun", 0 },
	{ "vt100", (TCF_MVT) },
	{ "ansi", (TCF_MSR | TCF_MEC | TCF_MACV | TCF_MACSR | TCF_MACSRS) },
	{ "vt101", (TCF_MVTE) },
	{ "vt102", (TCF_MVTE) },
	{ "vt220", (TCF_MVTE | TCF_MSCS94) },
	{ "vt320", (TCF_MVTE | TCF_MSCS94) },
	{ "xterm", (TCF_MVTE) },
	{ "xterm-color", (TCF_MVTE) },
	{ "screen", (TCF_MSCREEN ) },
	{ "screen94a", (TCF_MSCREEN | TCF_MSCS94 | TCF_MSCS94A) },
	{ "screen96", (TCF_MSCREEN | TCF_MSCS94 | TCF_MSCS96) },
	{ "vt420", (TCF_MVTADV) },
	{ "vt430", (TCF_MVTADV) },
	{ "vt440", (TCF_MVTADV) },
	{ "vt520", (TCF_MVTADV) },
	{ "vt530", (TCF_MVTADV) },
	{ "vt540", (TCF_MVTADV) },
	{ NULL, 0 }
} ;

static const char	*s_ansiconform[] = {
	"\033 L",			/* ANSI conformance level 1 */
	NULL
} ;

static const char	*s_basic[] = {
	"\017",				/* shift-in (G0 to GL)*/
	"\033>",			/* numeric keypad mode */
	"\033[?1l",			/* regular cursor keys */
	NULL
} ;

#ifdef	COMMENT
static const char	*s_scroll[] = {
	"\033[1;24r",
	NULL
} ;
#endif /* COMMENT */

static const char	*s_ec[] = {
	"\033[m",			/* all character attributes off */
	"\033[4l",			/* insert-mode off */
	NULL
} ;

static const char	*s_vcv[] = {
	TERMSTR_S_VCUR,			/* VT set cursor ON */
	NULL
} ;

static const char	*s_acv[] = {
	TERMSTR_S_ACUR,			/* ANSI set cursor ON */
	NULL
} ;

static const char	*s_scv[] = {
	TERMSTR_S_SCUR,			/* SCREEN set cursor ON */
	NULL
} ;

static const char	*s_psf[] = {
	"\033P1!uA\033\\",	/* designate ISO Latin-1 as supplimental */
	NULL
} ;

#if	CF_ANSICSM
static const char	*s_scs94[] = {
	"\033\050B",		/* ASCII (94) to G0 */
	"\033*0",		/* DEC Special Graphic (94) as G2 */
	"\033+>",		/* map DEC Technical (94) to G3 */
	"\017",			/* ASCII SHIFT-IN (G0 into GL) */
	NULL
} ;
#else /* CF_ANSICSM */
static const char	*s_scs94[] = {
	"\033\050B",		/* ASCII (94) to G0 */
	"\033\0510",		/* DEC Special Graphic (94) as G1 */
	"\033+>",		/* map DEC Technical (94) to G3 */
	"\017",			/* ASCII SHIFT-IN (G0 into GL) */
	NULL
} ;
#endif /* CF_ANSICSM */

/* this is the holding place for the failings of the SCREEN program! */
/* the SCREEN program thinks that the ISO-Latin-1 set is a 94-char set! */
#if	CF_ANSICSM
static const char	*s_scs94a[] = {
	"\033\051A",		/* map ISO Latin-1 (96) as G1 */
	"\033~",		/* lock shift G1 into GR */
	NULL
} ;
#else
static const char	*s_scs94a[] = {
	"\033*A",		/* map ISO Latin-1 (96) as G2 */
	"\033\175",		/* lock shift G2 into GR */
	NULL
} ;
#endif /* CF_ANSICSM */

#if	CF_ANSICSM
static const char	*s_scs96[] = {
	"\033-A",		/* map ISO Latin-1 (96) as G1 */
	"\033~",		/* lock shift G1 into GR */
	NULL
} ;
#else
static const char	*s_scs96[] = {
	"\033.A",		/* map ISO Latin-1 (96) as G2 */
	"\033\175",		/* lock shift G2 into GR */
	NULL
} ;
#endif /* CF_ANSICSM */

static const char	*s_home[] = {
	"\033[H",			/* home screen */
	NULL
} ;

static const char	*s_clear[] = {
	"\033[J",			/* clear screen */
	NULL
} ;

static const char	blanks[] = "        " ;


/* exported subroutines */


int b_s(int argc,cchar **argv,void *contextp)
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
/* end subroutine (b_s) */


int p_s(int argc,cchar **argv,cchar **envv,void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_s) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar **argv,cchar **envv,void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	BITS		pargs ;
	KEYOPT		akopts ;
	SHIO		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*prpcs = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*utfname = NULL ;
	cchar		*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_s: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

#if	CF_DEBUGN
	nprintf(NDF,"b_s: ent\n") ;
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
	pip->daytime = time(NULL) ;

	pip->lip = &li ;
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

/* program-root */
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

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* PCS program-root */
	                case argopt_pcsroot:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            prpcs = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                prpcs = argp ;
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

/* UTMP DB */
	                case argopt_utf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            utfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                utfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* terminal lines */
	                case argopt_lines:
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
					lip->lines = rs ;
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

/* terminal specification (terminal type) */
	                    case 'T':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->termspec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'a':
	                        lip->have.all = TRUE ;
	                        lip->final.all = TRUE ;
	                        lip->f.all = TRUE ;
	                        break ;

	                    case 'd':
	                        lip->have.date = TRUE ;
	                        lip->final.date = TRUE ;
	                        lip->f.date = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.date = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'i':
	                        lip->have.init = TRUE ;
	                        lip->final.init = TRUE ;
	                        lip->f.init = TRUE ;
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

	                    case 'w':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                lip->linelen = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
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

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = shio_open(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    shio_control(&errfile,SHIO_CSETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_s: debuglevel=%u\n",pip->debuglevel) ;
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

/* load up the environment options */

	if ((rs >= 0) && ((ai_max < 0) || (ai_pos < 0))) rs = SR_BUGCHECK ;

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

/* special option handling */

	if (lip->have.home && lip->f.home) {
	    if (! lip->have.clear) {
	        lip->final.clear = TRUE ;
	        lip->f.clear = FALSE ;
	    }
	} /* end if */

	if (lip->have.clear && (! lip->f.clear)) {
	    if (! lip->have.home) {
	        lip->final.home = TRUE ;
	        lip->f.home = FALSE ;
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_s: f_init=%u\n",lip->f.init) ;
	    debugprintf("b_s: f_all=%u\n",lip->f.all) ;
	    debugprintf("b_s: f_home=%u\n",lip->f.home) ;
	    debugprintf("b_s: f_clear=%u\n",lip->f.clear) ;
	    debugprintf("b_s: f_date=%u\n",lip->f.date) ;
	    debugprintf("b_s: f_sd=%u\n",lip->f.sd) ;
	    debugprintf("b_s: f_mailcheck=%u\n",lip->f.mailcheck) ;
	}
#endif /* CF_DEBUG */

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    shio_printf(pip->efp,"%s: f_init=%u\n",
	        pn,lip->f.init) ;
	    shio_printf(pip->efp,"%s: f_home=%u\n",
	        pn,lip->f.home) ;
	    shio_printf(pip->efp,"%s: f_clear=%u\n",
	        pn,lip->f.clear) ;
	    shio_printf(pip->efp,"%s: f_date=%u\n",
	        pn,lip->f.date) ;
	    shio_printf(pip->efp,"%s: f_scroll=%u\n",
	        pn,lip->f.scroll) ;
	    shio_printf(pip->efp,"%s: f_sd=%u\n",
	        pn,lip->f.sd) ;
	    shio_printf(pip->efp,"%s: f_mailcheck=%u\n",
	        pn,lip->f.mailcheck) ;
	    shio_printf(pip->efp,"%s: f_all=%u\n",
	        pn,lip->f.all) ;
	}

/* find the terminal type, if we have it */

	if (lip->termspec == NULL) lip->termspec = getourenv(envv,VARTERM) ;
	if (lip->termspec == NULL) lip->termspec = DEFTERMSPEC ;

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: term=%s\n",
	        pip->progname,lip->termspec) ;
	}

/* check for an output */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_s: ofname=%s\n",ofname) ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: ofile=%s\n",
	        pip->progname,ofname) ;
	}

/* continue */

	if (rs >= 0) {
	    if ((rs = locinfo_columns(lip)) >= 0) {
	        if ((rs = locinfo_prpcs(lip,prpcs)) >= 0) {
	            if ((rs = locinfo_utfname(lip,utfname)) >= 0) {
	                rs = process(pip,ofname) ;
	            }
	        }
	    } /* end if (locinfo_columns) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_NOENT:
	    case SR_BADFD:
	        ex = EX_CANTCREAT ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} else if ((rs >= 0) && (ex == EX_OK)) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

retearly:
	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",pn,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_s: exiting ex=%u (%d)\n",ex,rs) ;
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

#if	CF_DEBUGN
	nprintf(NDF,"b_s: ret rs=%d\n",rs) ;
#endif

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("main: final mallout=%u\n",mdiff) ;
	    if (mdiff > 0) {
	        UCMALLREG_CUR	cur ;
	        UCMALLREG_REG	reg ;
	        const int	size = (10*sizeof(uint)) ;
	        const char	*ids = "main" ;
	        uc_mallinfo(mi,size) ;
	        debugprintf("main: MIoutnum=%u\n",mi[ucmallreg_outnum]) ;
	        debugprintf("main: MIoutnummax=%u\n",mi[ucmallreg_outnummax]) ;
	        debugprintf("main: MIoutsize=%u\n",mi[ucmallreg_outsize]) ;
	        debugprintf("main: MIoutsizemax=%u\n",
	            mi[ucmallreg_outsizemax]) ;
	        debugprintf("main: MIused=%u\n",mi[ucmallreg_used]) ;
	        debugprintf("main: MIusedmax=%u\n",mi[ucmallreg_usedmax]) ;
	        debugprintf("main: MIunder=%u\n",mi[ucmallreg_under]) ;
	        debugprintf("main: MIover=%u\n",mi[ucmallreg_over]) ;
	        debugprintf("main: MInotalloc=%u\n",mi[ucmallreg_notalloc]) ;
	        debugprintf("main: MInotfree=%u\n",mi[ucmallreg_notfree]) ;
	        ucmallreg_curbegin(&cur) ;
	        while (ucmallreg_enum(&cur,&reg) >= 0) {
	            debugprintf("main: MIreg.addr=%p\n",reg.addr) ;
	            debugprintf("main: MIreg.size=%u\n",reg.size) ;
	            debugprinthexblock(ids,80,reg.addr,reg.size) ;
	        } /* end while */
	        ucmallreg_curend(&cur) ;
	    } /* end if (positive) */
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

	fmt = "%s: USAGE> %s [-i] [-T <termtype>] [-d[=<b>]]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-o <opt(s)>] [-a] [of <ofile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
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
	cchar		*cp ;

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
	                case akoname_all:
	                    if (! lip->final.all) {
	                        lip->have.all = TRUE ;
	                        lip->final.all = TRUE ;
	                        lip->f.all = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.all = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_init:
	                    if (! lip->final.init) {
	                        lip->have.init = TRUE ;
	                        lip->final.init = TRUE ;
	                        lip->f.init = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.init = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_sd:
	                    if (! lip->final.sd) {
	                        lip->have.sd = TRUE ;
	                        lip->final.sd = TRUE ;
	                        lip->f.sd = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.sd = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_home:
	                    if (! lip->final.home) {
	                        lip->have.home = TRUE ;
	                        lip->final.home = TRUE ;
	                        lip->f.home = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.home = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_clear:
	                    if (! lip->final.clear) {
	                        lip->have.clear = TRUE ;
	                        lip->final.clear = TRUE ;
	                        lip->f.clear = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.clear = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_date:
	                    if (! lip->final.date) {
	                        lip->have.date = TRUE ;
	                        lip->final.date = TRUE ;
	                        lip->f.date = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.date = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_scroll:
	                    if (! lip->final.scroll) {
	                        lip->have.scroll = TRUE ;
	                        lip->final.scroll = TRUE ;
	                        lip->f.scroll = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.scroll = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_la:
	                    if (! lip->final.la) {
	                        lip->have.la = TRUE ;
	                        lip->final.la = TRUE ;
	                        lip->f.la = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.la = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_mailcheck:
	                    if (! lip->final.mailcheck) {
	                        lip->have.mailcheck = TRUE ;
	                        lip->final.mailcheck = TRUE ;
	                        lip->f.mailcheck = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.mailcheck = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_mailfrom:
	                    if (! lip->final.mailfrom) {
	                        lip->have.mailfrom = TRUE ;
	                        lip->final.mailfrom = TRUE ;
	                        lip->f.mailfrom = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.mailfrom = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_mailsubj:
	                    if (! lip->final.mailsubj) {
	                        lip->have.mailsubj = TRUE ;
	                        lip->final.mailsubj = TRUE ;
	                        lip->f.mailsubj = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.mailsubj = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_nusers:
	                    if (! lip->final.nusers) {
	                        lip->have.nusers = TRUE ;
	                        lip->final.nusers = TRUE ;
	                        lip->f.nusers = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.nusers = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_lines:
	                    if (! lip->final.lines) {
	                        lip->have.lines = TRUE ;
	                        lip->final.lines = TRUE ;
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            lip->lines = rs ;
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


static int process(PROGINFO *pip,cchar *ofn)
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
	    if ((rs = shio_isterm(ofp)) > 0) {

	        rs = processer(pip,ofp) ;
		wlen += rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b_s: process() rs=%d\n",rs) ;
#endif

	    } /* end if (is-a-terminal) */
	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	} /* end if (opened output) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int processer(PROGINFO *pip,SHIO *ofp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_s/process: TERM=%s\n",lip->termspec) ;
#endif

	if ((rs = locinfo_termflags(lip)) >= 0) {
	    const int	blen = BUFLEN ;
	    int		tf = lip->termflags ;
	    int		colx ;
	    char	buf[BUFLEN + 1] ;

/* do we need mail information? */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_s/process: f_sd=%u\n",lip->f.sd) ;
	    debugprintf("b_s/process: f_mailcheck=%u\n",lip->f.mailcheck) ;
	    debugprintf("b_s/process: term-handle=%u\n",
	        (tf & TCF_MSD)) ;
	}
#endif

	if ((rs >= 0) && lip->f.mailcheck && lip->f.sd) {
	    if (tf & TCF_MSD) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_s/process: procmailusers()\n") ;
#endif

	    rs = procmailusers(pip) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_s/process: procmailusers() rs=%d\n",rs) ;
#endif

	    if (pip->debuglevel > 0) {
	        shio_printf(pip->efp,"%s: mailusers len=%d\n",pn,rs) ;
	    }

	    }
	} /* end if (mail-information) */

/* start in */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_s/process: f_init=%u\n",lip->f.init) ;
#endif

	if ((rs >= 0) && lip->f.init) {
	    if ((rs = terminit(pip,ofp,buf,blen)) >= 0) {
	        rs = shio_write(ofp,buf,rs) ;
	    }
	} /* end if (terminit) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_s/process: f_clear=%u\n",lip->f.clear) ;
#endif

	if ((rs >= 0) && (lip->f.home || lip->f.clear)) {
	    if ((rs = termclear(pip,buf,blen)) >= 0) {
	        rs = shio_write(ofp,buf,rs) ;
	    }
	} /* end if (termclear) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_s/process: f_date=%u\n",lip->f.date) ;
#endif

	if ((rs >= 0) && lip->f.date) {
	    TMTIME	tm ;
	    const int	tl = 16 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("b_s/process: f_sd=%u\n",lip->f.sd) ;
#endif

	    if ((rs = tmtime_localtime(&tm,pip->daytime)) >= 0) {
	        const int	tlen = TIMEBUFLEN ;
	        cchar		*ts = "%a %e %b %R" ;
	        char		tbuf[TIMEBUFLEN + 1] ;

		if ((rs = sntmtime(tbuf,tlen,&tm,ts)) >= 0) {
	            if (lip->f.sd) {
	                if ((rs = termdate(pip,buf,blen,tbuf)) >= 0) {
	                    rs = shio_write(ofp,buf,rs) ;
	                }
	            } else {
	                rs = shio_printf(ofp,"%t\n",tbuf,tl) ;
		    }
		} /* end if (sntmtime) */

	    } /* end if (tmtime_localtime) */

	} /* end if (date) */

#if	CF_TERMMAILNAME
	if ((rs >= 0) && lip->f.mailcheck && lip->f.sd) {
	    if (lip->termflags & TCF_MSD) {
	        colx = 0 ;
	        if ((rs = termmailname(pip,buf,blen,colx)) >= 0) {
	            rs = shio_write(ofp,buf,rs) ;
	        }
	    }
	} /* end if (mailcheck) */
#endif /* CF_TERMMAILNAME */

	} /* end if (locinfo_termflags) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_s/process: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (processer) */


static int procmailusers(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	cchar		*sp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_s/procmailusers: ent\n") ;
#endif

/* get our username */

	if (pip->username == NULL) {
	    const int	ulen = USERNAMELEN ;
	    char	ubuf[USERNAMELEN+1] ;
	    if ((rs = getusername(ubuf,ulen,-1)) >= 0) {
	        cchar	**vpp = &pip->username ;
	        rs = proginfo_setentry(pip,vpp,ubuf,rs) ;
	    }
	}

/* process either environment or the default */

	if (rs >= 0) {
	    cchar	*var = VARMAILUSERS ;
	    if ((sp = getourenv(pip->envv,var)) != NULL) {
	        cchar	*tp ;
		rs = 0 ;
	        while ((rs >= 0) && ((tp = strpbrk(sp,",\t ")) != NULL)) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_s/procmailusers: got=>%t<\n",sp,(tp-sp)) ;
#endif
	            if ((tp - sp) > 0) {
	                rs = procmailuser(pip,sp,(tp - sp)) ;
	            }
	            if (rs > 0) break ;
	            sp = (tp + 1) ;
	        } /* end while */
	        if ((rs == 0) && (sp[0] != '\0')) {
	            rs = procmailuser(pip,sp,-1) ;
	        }
	    } else {
	        rs = procmailuser(pip,pip->username,-1) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_s/procmailusers: pcsmailuser() rs=%d >%s<\n",
	                rs,lip->mnbuf) ;
#endif
	    }
	    if (pip->debuglevel > 0) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: msgs=%d mail=>%s<\n" ;
	        shio_printf(pip->efp,fmt,pn,lip->nmsgs,lip->mnbuf) ;
	    }
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_s/procmailusers: ret rs=%d msgs=%d\n",
	        rs,lip->nmsgs) ;
	    debugprintf("b_s/procmailusers: ret from=>%s<\n",
	        lip->mnbuf) ;
	}
#endif

	return (rs >= 0) ? lip->nmsgs : rs ;
}
/* end subroutine (procmailusers) */


static int procmailuser(PROGINFO *pip,cchar *sp,int sl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		cl ;
	cchar		*cp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_s/procmailuser: ent un=%t\n",sp,sl) ;
#endif

	lip->mnbuf[0] = '\0' ;
	if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	    const int	ulen = USERNAMELEN ;
	    const int	flen = FROMBUFLEN ;
	    const int	maxulen = MIN(USERNAMELEN,MAXUSERLEN) ;
	    int		ul ;
	    char	ubuf[USERNAMELEN + 1] ;
	    char	fbuf[FROMBUFLEN + 1] ;
	    ul = strwcpy(ubuf,cp,MIN(cl,ulen)) - ubuf ;
	    if ((rs = pcsmailcheck(lip->prpcs,fbuf,flen,ubuf)) > 0) {
	        const int	maxlen = MIN(FROMBUFLEN,(lip->mnlen-2)) ;
	        const int	f_us = (strcmp(pip->username,ubuf) == 0) ;
	        lip->nmsgs = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("b_s/procmailuser: pcsmailcheck() rs=%d\n",rs) ;
	            debugprintf("b_s/procmailuser: f=>%s<\n",fbuf) ;
	        }
#endif

	        if (f_us) {
	            fbuf[maxlen] = '\0' ;
	            rs = sncpy2(lip->mnbuf,lip->mnlen,"¶ ",fbuf) ;
	        } else {
	            ubuf[maxulen] = '\0' ;
	            if (ul > maxulen) ul = maxulen ;
	            fbuf[maxlen-ul] = '\0' ;
	            rs = sncpy4(lip->mnbuf,lip->mnlen,"¶ ",ubuf,"« ",fbuf) ;
	        }
	    } else if (isNotPresent(rs)) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_s/procmailuser: pcsmailcheck() rs=%d\n",rs) ;
#endif
	        rs = SR_OK ;
	    } /* end if */
	} /* end if (positive) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_s/procmailuser: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? lip->nmsgs : rs ;
}
/* end subroutine (procmailuser) */


/* create the data to set (reset-fix) the terminal state */
static int terminit(PROGINFO *pip,SHIO *ofp,char *tbuf,int tlen)
{
	LOCINFO		*lip = pip->lip ;
	SBUF		b ;
	const int	clen = CODEBUFLEN ;
	int		rs ;
	int		cl ;
	int		bl = 0 ;
	int		f_curvis = FALSE ;
	cchar		*fmt ;
	char		cbuf[CODEBUFLEN + 1] ;

	if ((rs = sbuf_start(&b,tbuf,tlen)) >= 0) {

/* set ANSI conformance level to '1' */

	    if (lip->termflags & TCF_MACL)
	        sbuf_loadstrs(&b,s_ansiconform) ;

/* more basic stuff */

#if	CF_BASIC
	    sbuf_loadstrs(&b,s_basic) ;
#endif

/* enhanced character attributes */

#if	CF_ENHANCED
	    if (lip->termflags & TCF_MEC)
	        sbuf_loadstrs(&b,s_ec) ;
#endif

/* cursor visibility (VT) */

#if	CF_VCV
	    if ((lip->termflags & TCF_MVCV) && (! f_curvis)) {
	        f_curvis = TRUE ;
	        sbuf_loadstrs(&b,s_vcv) ;
	    }
#endif

/* cursor visibility (ANSI) */

#if	CF_ACV
	    if ((lip->termflags & TCF_MACV) && (! f_curvis)) {
	        f_curvis = TRUE ;
	        sbuf_loadstrs(&b,s_acv) ;
	    }
#endif

/* cursor visibility (SCREEN) */

#if	CF_SCV
	    if ((lip->termflags & TCF_MSCV) && (! f_curvis)) {
	        f_curvis = TRUE ;
	        sbuf_loadstrs(&b,s_scv) ;
	    }
#endif

/* set ISO-Latin1 as preferred supplemental character set */

	    if (lip->termflags & TCF_MPSF)
	        sbuf_loadstrs(&b,s_psf) ;

/* supplemental character set 94 */

#if	CF_SCS94
	    if (lip->termflags & TCF_MSCS94)
	        sbuf_loadstrs(&b,s_scs94) ;
#endif

/* supplemental character set 94a */

#if	CF_SCS94A
	    if (lip->termflags & TCF_MSCS94A)
	        sbuf_loadstrs(&b,s_scs94a) ;
#endif

/* supplemental character set 96 */

#if	CF_SCS96
	    if (lip->termflags & TCF_MSCS96)
	        sbuf_loadstrs(&b,s_scs96) ;
#endif

/* set scroll region to full number of terminal lines */

#if	CF_SR
	    if (rs >= 0) {
		if ((rs = locinfo_termlines(lip,ofp)) > 0) {
	            if (lip->f.scroll && (lip->termflags & TCF_MSR)) {

#if	CF_CSR
	        if (lip->termflags & TCF_MVCSR) {
	            sbuf_strw(&b,TERMSTR_VCURS,-1) ;
	        } else if (lip->termflags & TCF_MACSR) {
	            sbuf_strw(&b,TERMSTR_ACURS,-1) ;
	        }
#endif /* CF_CSR */

	        cl = termconseq(cbuf,clen,'r',1,lip->lines,-1,-1) ;

	        sbuf_strw(&b,cbuf,cl) ;

#if	CF_CSR
	        if (lip->termflags & TCF_MVCSR) {
	            sbuf_strw(&b,TERMSTR_VCURR,-1) ;
	        } else if (lip->termflags & TCF_MACSR) {
	            sbuf_strw(&b,TERMSTR_ACURR,-1) ;
	        }
#endif

	            } /* end if (adjustable line scrolling regions) */
		} /* end if (locinfo_termlines) */
	    } /* end if (ok) */
#endif /* CF_SR */

/* clear screen */

	    if (lip->f.clear) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b_s/terminit: clear\n") ;
#endif

	        lip->f.clear = FALSE ;
	        if (lip->f.home)
	            sbuf_loadstrs(&b,s_home) ;

	        sbuf_loadstrs(&b,s_clear) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b_s/terminit: f_all=%u TERM=%s F_MSD=%u\n",
	                lip->f.all,lip->termspec,
	                ((lip->termflags & TCF_MSD) ? 1 : 0)) ;
#endif

	        if (lip->f.all && (lip->termflags & TCF_MSD)) {
	            fmt = "%s\r%s%s" ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("b_s/terminit: clear SD\n") ;
#endif

	            bl = bufprintf(cbuf,clen,fmt,
	                TERMSTR_S_SD,TERMSTR_ED,TERMSTR_R_SD) ;

#ifdef	COMMENT
	            fmt = "\033[26;1H\r%s\033[1;1H%s" ;
	            bl += bufprintf((buf + bl),BUFLEN,fmt,
	                TERMSTR_ED,TERMSTR_ED) ;
#endif

	            sbuf_strw(&b,cbuf,bl) ;

	        } /* end if ("all" specified) */

	    } /* end if (clear) */

	    bl = sbuf_finish(&b) ;
	    if (rs >= 0) rs = bl ;
	} /* end if (sbuf) */

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (terminit) */


/* clear the terminal */
static int termclear(PROGINFO *pip,char *rbuf,int rlen)
{
	LOCINFO		*lip = pip->lip ;
	SBUF		b ;
	int		rs ;
	int		bl = 0 ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    const int	clen = CODEBUFLEN ;
	    char	cbuf[CODEBUFLEN + 1] ;

/* clear the main screen */

	    if (lip->f.home)
	        sbuf_loadstrs(&b,s_home) ;

	    if (lip->f.clear)
	        sbuf_loadstrs(&b,s_clear) ;

/* clear the status display if there is one and additionally asked to do so */

	    if (lip->f.clear && lip->f.all && (lip->termflags & TCF_MSD)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b_s/termclear: clear SD\n") ;
#endif

	        bl = bufprintf(cbuf,clen,"%s\r%s%s",
	            TERMSTR_S_SD,TERMSTR_ED,TERMSTR_R_SD) ;

	        sbuf_strw(&b,cbuf,bl) ;

	    } /* end if */

	    bl = sbuf_finish(&b) ;
	    if (rs >= 0) rs = bl ;
	} /* end if (sbuf) */

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (termclear) */


static int termdate(PROGINFO *pip,char *rbuf,int rlen,cchar *timebuf)
{
	LOCINFO		*lip = pip->lip ;
	SBUF		b ;
	int		rs ;
	int		bl = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_s/termdate: ent\n") ;
#endif

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    const int	tf = lip->termflags ;
	    const int	ntime = 16 ;
	    const int	cols = lip->linelen ;

	    if (tf & TCF_MSD) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("b_s/termdate: SD\n") ;
#endif

	        rs = termdatesd(pip,&b,timebuf) ;

	    } else if ((tf & TCF_MVCSR) || (tf & TCF_MACSR)) {

	        rs = bufdiv(pip,tf,&b,0,(cols- ntime),timebuf,ntime) ;

	    } else {

	        sbuf_strw(&b,timebuf,ntime) ;
	        sbuf_char(&b,'\n') ;

	    } /* end if */

/* done */

	    bl = sbuf_finish(&b) ;
	    if (rs >= 0) rs = bl ;
	} /* end if (sbuf) */

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (termdate) */


static int termdatesd(PROGINFO *pip,SBUF *bufp,cchar *timebuf)
{
	LOCINFO		*lip = pip->lip ;
	SBUF		lb ;
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		ncols = 0 ;
	int		cols ;
	int		len = 0 ;
	char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_s/termdatesd: ent\n") ;
#endif

/* initialize a local line buffer */

	cols = lip->linelen ;
	if ((rs = sbuf_start(&lb,lbuf,llen)) >= 0) {
	    int		nfcols ;
	    int		n, i ;
	    char	buf[7 + 1] ;
	    char	*pbp = (char *) blanks ;

/* number of users logged into machine (3-column field) */

	    nfcols = 3 ;
	    ncols += nfcols ;

	    if (rs >= 0) {
	        rs1 = SR_OVERFLOW ;
	        if (lip->f.nusers) {

	            if ((rs1 = nusers(lip->utfname)) >= 0) {
	                n = rs1 ;

	                if (n > 99) n = 99 ;
	                rs1 = bufprintf(buf,7,"%2u ",n) ;

#if	CF_DEBUG && CF_DEBUGHEXBUF
	                if (DEBUGLEVEL(3)) {
	                    char	hexbuf[HEXBUFLEN + 1] ;
	                    mkhexstr(hexbuf,HEXBUFLEN,buf,nfcols) ;
	                    debugprintf("b_s/termdatesd: nusers 1 hb=>%s<\n",
	                        hexbuf) ;
	                }
#endif /* CF_DEBUG */

	                if (rs1 >= 0)
	                    pbp = buf ;

	            }

	        } /* end if (nusers) */

	        sbuf_strw(&lb,pbp,nfcols) ;

#if	CF_DEBUG && CF_DEBUGHEXBUF
	        if (DEBUGLEVEL(3)) {
	            char	hexbuf[HEXBUFLEN + 1] ;
	            mkhexstr(hexbuf,HEXBUFLEN,pbp,nfcols) ;
	            debugprintf("b_s/termdatesd: nusers nfcols=%u\n",
	                nfcols) ;
	            debugprintf("b_s/termdatesd: nusers hb=>%s<\n",
	                hexbuf) ;
	            debugprintf("b_s/termdatesd: nusers rs1=%d buf=>%t<\n",
	                rs1,pbp,nfcols) ;
	        }
#endif /* CF_DEBUG */

	    } /* end if (3-column field) */

/* machine load-average (5-column field) */

	    nfcols = 5 ;
	    ncols += nfcols ;
	    if (rs >= 0) {
	        char	labuf[12 + 1] ;
	        cchar	*pbp = blanks ;

	        if (lip->f.la) {
	            double	la[3] ;

	            if ((rs = uc_getloadavg(la,3)) >= 0) {
	                const double	maxla = 99.9 ;

	                for (i = 0 ; i < 3 ; i += 1) {
	                    if (la[i] > maxla) la[i] = maxla ;
	                }

	                rs1 = bufprintf(labuf,12,"%4.1f ",la[0]) ;
	                if (rs1 >= 0) pbp = labuf ;

	            } /* end if */

	        } /* end if (la) */

	        sbuf_strw(&lb,pbp,nfcols) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("b_s/termdatesd: labuf=>%t<\n",pbp,nfcols) ;
#endif

	    } /* end if (5-column field) */

/* number of mail messages (4-column field) */

	    nfcols = 4 ;
	    ncols += nfcols ;
	    if (rs >= 0) {
	        char	mcbuf[MCBUFLEN + 1] ;
	        cchar	*pbp = blanks ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("b_s/termdatesd: mailcheck=%u\n",
	                lip->f.mailcheck) ;
#endif

	        mcbuf[0] = '\0' ;
	        rs1 = lip->nmsgs ;
	        if (lip->f.mailcheck && (rs1 > 0)) {
	            const int	maxmsgs = 99 ;

	            if (rs1 > maxmsgs) rs1 = maxmsgs ;

	            rs1 = bufprintf(mcbuf,MCBUFLEN,"%2u%c ",rs1,0xB6) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_s/termdatesd: bufprintf() rs=%d\n",rs1) ;
#endif

	            if (rs1 >= 0)
	                pbp = mcbuf ;

	        } /* end if (mailcheck) */

	        sbuf_strw(&lb,pbp,nfcols) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("b_s/termdatesd: mcbuf=>%t<\n",pbp,nfcols) ;
#endif

	    } /* end if (4-column field) */

/* time-of-day (19-column field) */

	    nfcols = 16 ;
	    ncols += nfcols ;
	    if (rs >= 0) {

	        sbuf_strw(&lb,timebuf,nfcols) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("b_s/termdatesd: timebuf=>%t<\n",
	                timebuf,nfcols) ;
#endif

	    } /* end if (19-column field) */

	    len = sbuf_finish(&lb) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("b_s/termdatesd: ncols=%u\n",ncols) ;
	    debugprintf("b_s/termdatesd: sdbuf=>%t<\n",lbuf,len) ;
	}
#endif

	if (rs >= 0) {

#if	CF_DEBUGSD
	    len = sncpy1(lbuf,llen,"hello world1!") ;
#endif /* CF_DEBUGSD */

	    rs = bufsd(pip,lip->termflags,bufp,(cols-ncols),lbuf,len) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_s/termdatesd: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (termdatesd) */


static int termmailname(PROGINFO *pip,char *tbuf,int tlen,int colx)
{
	LOCINFO		*lip = pip->lip ;
	SBUF		b ;
	int		nlen ;
	int		rs ;
	int		len = 0 ;
	char		*nbuf ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_s/termmailname: colx=%u\n",colx) ;
#endif

	nlen = lip->mnlen ;
	nbuf = lip->mnbuf ;
	if ((rs = sbuf_start(&b,tbuf,tlen)) >= 0) {
	    int		nl ;
	    int		f_empty = FALSE ;
	    int		ncols = MIN(40,lip->mnlen) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_s/termmailname: mnbuf=>%s<\n",lip->mnbuf) ;
#endif

	    if (lip->mnbuf[0] == '\0') {
	        f_empty = TRUE ;
	        strwset(lip->mnbuf,' ',MIN(nlen,ncols)) ;
	        nl = ncols ;
	    } else {
	        const int	ml = strlen(lip->mnbuf) ;
	        strwset((lip->mnbuf+ml),' ',MIN(nlen,(ncols-ml))) ;
	        nl = ncols ;
	    }

	    if (rs >= 0) {
	        rs = bufsd(pip,lip->termflags,&b,colx,nbuf,nl) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_s/termmailname: bufsd() rs=%d\n",rs) ;
#endif
	    }

	    if (f_empty)
	        lip->mnbuf[0] = '\0' ;

	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_s/termmailname: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (termmailname) */


static int bufsd(pip,termflags,bufp,x,sp,sl)
PROGINFO	*pip ;
int		termflags ;
SBUF		*bufp ;
int		x ;
cchar		sp[] ;
int		sl ;
{
	int		rs ;
	int		len = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bufsd: ent x=%d sl=%d\n",x,sl) ;
#endif

	if (x < 0) x = 0 ;

	if ((rs = sbuf_getlen(bufp)) >= 0) {
	    const int	len_s = rs ;
	    int		len_e ;
	    int		cf_adv = FALSE ;

/* status line */

#if	CF_ACV
	cf_adv = TRUE ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    const int	f = (termflags & TCF_MVCV) ? 1 : 0 ;
	    debugprintf("bufsd: TCF_MVCV=%u\n",f) ;
	}
#endif /* CF_DEBUG */

	if (termflags & TCF_MVCV) {
	    sbuf_strw(bufp,TERMSTR_R_VCUR,-1) ;
	} else if (cf_adv && (termflags & TCF_MACV)) {
	    sbuf_strw(bufp,TERMSTR_R_ACUR,-1) ;
	}

	sbuf_strw(bufp,TERMSTR_VCURS,-1) ; /* save cursor */

	sbuf_strw(bufp,TERMSTR_S_SD,-1) ; /* set status-display mode */

	sbuf_char(bufp,'\r') ;

	if (x > 0) {
	    const int	clen = CODEBUFLEN ;
	    int		cl ;
	    char	cbuf[CODEBUFLEN + 1] ;

	    if ((cl = termconseq(cbuf,clen,'C',x,-1,-1,-1)) > 0) {
	        sbuf_strw(bufp,cbuf,cl) ;
	    }

	} /* end block */

	sbuf_strw(bufp,sp,sl) ;

	sbuf_strw(bufp,TERMSTR_R_SD,-1) ; /* restore status-display mode */

	sbuf_strw(bufp,TERMSTR_VCURR,-1) ; /* restore cursor */

	if (termflags & TCF_MVCV) {
	    sbuf_strw(bufp,TERMSTR_S_VCUR,-1) ;
	} else if (cf_adv && (termflags & TCF_MACV)) {
	    sbuf_strw(bufp,TERMSTR_S_ACUR,-1) ;
	}

/* finish */

	len_e = sbuf_getlen(bufp) ;
	if (rs >= 0) rs = len_e ;

	len = (len_e - len_s) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    const int	cols = COLUMNS ;
	    cchar	*bp ;
	    if ((rs = sbuf_getbuf(bufp,&bp)) >= 0) {
	        debugprintf("b_s/bufsd: buf bl=%u\n",rs) ;
	        debugprinthexblock("bufsd",cols,bp,rs) ;
	    }
	}
#endif /* CF_DEBUG */

	} /* end if (sbuf_getlen) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bufsd: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (bufsd) */


static int bufdiv(pip,termflags,bufp,y,x,sp,sl)
PROGINFO	*pip ;
int		termflags ;
SBUF		*bufp ;
int		y, x ;
cchar		sp[] ;
int		sl ;
{
	int		rs ;
	int		cf_adv = FALSE ;
	int		len = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("bufdiv: ent x=%d\n",x) ;
	    debugprintf("bufdiv: buf=>%t<\n",sp,sl) ;
	}
#endif

#if	CF_ACV
	cf_adv = TRUE ;
#endif

	if (x < 0) x = 0 ;
	if (y < 0) y = 0 ;

	if ((rs = sbuf_getlen(bufp)) >= 0) {
	    const int	len_s = rs ;
	    int		len_e ;

/* status line */

	    if (termflags & TCF_MVCV) {
	        sbuf_strw(bufp,TERMSTR_R_VCUR,-1) ;
	    } else if (cf_adv && (termflags & TCF_MACV)) {
	        sbuf_strw(bufp,TERMSTR_R_ACUR,-1) ;
	    }

	    sbuf_strw(bufp,TERMSTR_VCURS,-1) ;

	    {
	        const int	clen = CODEBUFLEN ;
	        int		cl ;
	        char		cbuf[CODEBUFLEN + 1] ;

	        cl = termconseq(cbuf,clen,'H',(y+1),(x+1),-1,-1) ;

	        if (cl > 0)
	            sbuf_strw(bufp,cbuf,cl) ;

	        sbuf_strw(bufp,sp,sl) ;

	    } /* end block */

	    sbuf_strw(bufp,TERMSTR_VCURR,-1) ;

	    if (termflags & TCF_MVCV) {
	        sbuf_strw(bufp,TERMSTR_S_VCUR,-1) ;
	    } else if (cf_adv && (termflags & TCF_MACV)) {
	        sbuf_strw(bufp,TERMSTR_S_ACUR,-1) ;
	    }

/* finish */

	    len_e = sbuf_getlen(bufp) ;
	    if (rs >= 0) rs = len_e ;
	    len = (len_e - len_s) ;
	} /* end if (sbuf_getlen) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (bufdiv) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	const int	mnlen = MNBUFLEN ;
	int		rs ;
	char		*bp ;
	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->lines = 0 ;
	lip->f.home = TRUE ;
	lip->f.clear = TRUE ;
	lip->f.scroll = TRUE ;
	lip->f.la = TRUE ;
	lip->f.mailcheck = TRUE ;
	lip->f.nusers = TRUE ;
	lip->f.sd = TRUE ;
	if ((rs = uc_malloc((mnlen+1),&bp)) >= 0) {
	    lip->mnbuf = bp ;
	    lip->mnlen = mnlen ;
	}
	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->mnbuf != NULL) {
	    rs1 = uc_free(lip->mnbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->mnbuf = NULL ;
	    lip->mnlen = 0 ;
	}
	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (locinfo_finish) */


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


static int locinfo_columns(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (lip->linelen == 0) {
	    cchar	*vp ;
	    if ((vp = getourenv(pip->envv,VARCOLUMNS)) != NULL) {
	        if ((rs = optvalue(vp,-1)) >= 0) {
	            lip->linelen = rs ;
	        }
	    } else {
		lip->linelen = COLUMNS ;
	    }
	} /* end if (needed) */
	return rs ;
}
/* end subroutine (locinfo_columns) */


static int locinfo_prpcs(LOCINFO *lip,cchar *prpcs)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (lip->f.mailcheck) {
	    if ((prpcs != NULL) && (prpcs[0] != '\0')) {
		struct ustat	sb ;
		if ((rs = uc_stat(prpcs,&sb)) >= 0) {
		    if (S_ISDIR(sb.st_mode)) {
	                lip->prpcs = prpcs ;
		    } else {
			rs = SR_NOTDIR ;
		    }
		} /* end if (stat) */
	    } else {
	        const int	tlen = MAXPATHLEN ;
	        cchar		*name = VARPRPCS ;
	        cchar		*dn = pip->domainname ;
	        char		tbuf[MAXPATHLEN+1] = { 0 } ;
	        if ((rs = mkpr(tbuf,tlen,name,dn)) >= 0) {
	            cchar	**vpp = &lip->prpcs ;
	            rs = locinfo_setentry(lip,vpp,tbuf,rs) ;
	        }
	    }
	} /* end if (mailcheck) */
	return rs ;
}
/* end subroutine (locinfo_prpcs) */


static int locinfo_utfname(LOCINFO *lip,cchar *utfn)
{
	int		rs = SR_OK ;
	if (lip->f.nusers && (lip->utfname == NULL)) {
	    cchar	**vpp = &lip->utfname ;
	    rs = locinfo_setentry(lip,vpp,utfn,-1) ;
	} /* end if (nusers) */
	return rs ;
}
/* end subroutine (locinfo_utfname) */


static int locinfo_termlines(LOCINFO *lip,SHIO *ofp)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (lip->lines == 0) {
	    if ((rs = shio_getlines(ofp)) == 0) {
	        cchar	*cp ;
	        if ((cp = getourenv(pip->envv,VARLINES)) != NULL) {
	            rs = optvalue(cp,-1) ;
	            lip->lines = rs ;
	        } else {
	            lip->lines = DEFLINES ;
	        }
	    } else if (rs > 0) {
	        lip->lines = rs ;
	    }
	} /* end if (needed) */
	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: lines=%u\n" ;
	    shio_printf(pip->efp,fmt,pn,lip->lines) ;
	}
	return rs ;
}
/* end subroutine (locinfo_termlines) */


static int locinfo_termflags(LOCINFO *lip)
{
	int		rs = SR_OK ;
	cchar		*termspec = lip->termspec ;

	lip->termflags = TCF_MDEFAULT ;
	if (termspec != NULL) {
	    int		i ;
	    int		f = FALSE ;
	    for (i = 0 ; terms[i].name != NULL ; i += 1) {
	        f = (strcmp(terms[i].name,termspec) == 0) ;
	        if (f) break ;
	    } /* end for */
	    if (f) {
		lip->termflags = terms[i].flags ;
	    }
	} /* end if (had a terminal type) */

	return rs ;
}
/* end subroutine (locinfo_termflags) */


