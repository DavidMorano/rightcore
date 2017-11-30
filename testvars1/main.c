/* main */

/* set the "system" variables at boot-up time */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocations */
#define	CF_HDBSTR	0		/* HDBSTR instead of VARMKS */


/* revision history:

	= 2004-01-10, David A­D­ Morano

	This is a complete rewrite of the previous code that performed
	this function.


*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This subroutine is the front-end for the program that sets the
	"system" varialbes at machine boot-up time.

	Synopsis:

	$ testvars [-s] [file(s)]


	This code can be a built-in command to the KSH shell.  But it
	really needs to be SUID to either user or group to really work
	properly, so it is always made to be stand-alone for the time
	being.


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<hdb.h>
#include	<hdbstr.h>
#include	<filebuf.h>
#include	<field.h>
#include	<char.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"varmks.h"
#include	"vars.h"


/* local defines */

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	USERNAMELEN
#define	USERNAMELEN	32
#endif

#ifndef	GNAMELEN
#define	GNAMELEN	80		/* GECOS name length */
#endif

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	120		/* key-buffer length */
#endif

#ifndef	NOFILE
#define	NOFILE		20
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	BUFLEN		MAX((4 * MAXPATHLEN),LINEBUFLEN)

#define	WORDEXPORT	"export"

#define	TO_OPEN		5
#define	TO_READ		5

#ifndef	DEFNVARS
#define	DEFNVARS	(1 * 1000 * 1000)
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	strnnlen(const char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;

#ifdef	COMMENT
extern int	vecstr_envfile(vecstr *,const char *) ;
#endif

extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */

#if	(! CF_SFIO)
extern char	**environ ;
#endif


/* local structures */

struct locinfo_flags {
	uint	set : 1 ;
	uint	test : 1 ;
	uint	list: 1 ;
	uint	query : 1 ;
	uint	dump : 1 ;
	uint	audit : 1 ;
	uint	uniq : 1 ;
	uint	vars_init : 1 ;
} ;

struct locinfo {
	struct proginfo	*pip ;
	struct locinfo_flags	have, f, changed, final ;
	char		*dumpfname ;
	HDBSTR		vars ;
	VECOBJ		files ;
	VARMKS		mk ;
	HDB		mvars ;
	vecstr		queries ;
	int		tsize ;
} ;

struct filemap {
	char		*fname ;
	caddr_t		fmap ;
	size_t		fsize ;
} ;


/* forward references */

static int	usage(struct proginfo *) ;
static int	procopts(struct proginfo *,KEYOPT *) ;
static int	locinfo_start(struct locinfo *,struct proginfo *) ;
static int	locinfo_name(struct locinfo *,const char *) ;
static int	locinfo_fileload(struct locinfo *,struct filemap *) ;
static int	locinfo_fileproc(struct locinfo *,struct filemap *) ;
static int	locinfo_varcount(struct locinfo *) ;
static int	locinfo_qkey(struct locinfo *,const char *,int) ;
static int	locinfo_addvar(struct locinfo *,const char *,int) ;
static int	locinfo_finish(struct locinfo *) ;

static int	procname(struct proginfo *,const char *) ;

#ifdef	COMMENT
static int	procfile(struct proginfo *,const char *) ;
static int	procsysdefs(struct proginfo *) ;
static int	procsysdef(struct proginfo *,const char *) ;
#endif

static int	procvarfile(struct proginfo *,const char *) ;
static int	procset(struct proginfo *,const char *) ;
static int	procload(struct proginfo *,const char *) ;
static int	procaudit(struct proginfo *,VARS *) ;
static int	process(struct proginfo *,const char *,void *,const char *) ;
static int	procgetfile(struct proginfo *,void *,VARS *,const char *) ;
static int	procqkey(struct proginfo *,void *,VARS *,const char *,int) ;

static int	filemap_free(struct filemap *) ;

static int	hasweird(const char *,int) ;

static void	sighand_int(int) ;


/* local variables */

static volatile int	if_int ;

static const int	sigblocks[] = {
	SIGUSR1,
	SIGUSR2,
	SIGHUP,
	SIGQUIT,
	SIGCHLD,
	0
} ;

static const int	sigignores[] = {
	SIGPIPE,
	SIGPOLL,
	0
} ;

static const int	sigints[] = {
	SIGINT,
	SIGTERM,
	0
} ;

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"LOGFILE",
	"af",
	"gf",
	"of",
	"db",
	"dump",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_logfile,
	argopt_af,
	argopt_gf,
	argopt_of,
	argopt_db,
	argopt_dump,
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

static const char *akonames[] = {
	"set",
	"test",
	"list",
	"audit",
	"uniq",
	"dump",
	NULL
} ;

enum akonames {
	akoname_set,
	akoname_test,
	akoname_list,
	akoname_audit,
	akoname_uniq,
	akoname_dump,
	akoname_overlast
} ;

static const uchar	qterms[] = {
	0x00, 0x3A, 0x00, 0x00,
	0x01, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const uchar	fterms[32] = {
	0x00, 0x3A, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x20,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const char	*wstrs[] = {
	"TZ",
	"LANG",
	"UMASK",
	"PATH",
	NULL
} ;

static const char	*pstrs[] = {
	"LC_",
	NULL
} ;


/* exported subroutines */


int main(argc,argv,contextp)
int	argc ;
char	*argv[] ;
void	*contextp ;
{
	struct proginfo	pi, *pip = &pi ;

	struct locinfo	li, *lip = &li ;

	struct sigaction	san ;
	struct sigaction	sao[nelem(sigints) + nelem(sigignores)] ;

	KEYOPT		akopts ;

	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	sigset_t	oldsigmask, newsigmask ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, rs1 ;
	int	n, i, j ;
	int	size ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	tmpdbfname[MAXPATHLEN + 1] ;
	const char	*pr = NULL ;
	const char	*afname = NULL ;
	const char	*getfname = NULL ;
	const char	*ofname = NULL ;
	const char	*dbname = NULL ;
	const char	*cp ;


	if_int = 0 ;

	n = nelem(sigints) + nelem(sigignores) ;
	size = n * sizeof(struct sigaction) ;
	memset(sao,0,size) ;

/* block some signals and catch the others */

	uc_sigsetempty(&newsigmask) ;

	for (i = 0 ; sigblocks[i] != 0 ; i += 1)
	    uc_sigsetadd(&newsigmask,sigblocks[i]) ;

	u_sigprocmask(SIG_BLOCK,&newsigmask,&oldsigmask) ;

	uc_sigsetempty(&newsigmask) ;

/* ignore these signals */

	j = 0 ;
	for (i = 0 ; sigignores[i] != 0 ; i += 1) {

	    memset(&san,0,sizeof(struct sigaction)) ;

	    san.sa_handler = SIG_IGN ;
	    san.sa_mask = newsigmask ;
	    san.sa_flags = 0 ;
	    u_sigaction(sigignores[i],&san,(sao + j)) ;

	    j += 1 ;

	} /* end for */

/* interrupt on these signals */

	for (i = 0 ; sigints[i] != 0 ; i += 1) {

	    memset(&san,0,sizeof(struct sigaction)) ;

	    san.sa_handler = sighand_int ;
	    san.sa_mask = newsigmask ;
	    san.sa_flags = 0 ;
	    u_sigaction(sigints[i],&san,(sao + j)) ;

	    j += 1 ;

	} /* end for */


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFNAME)) == NULL) {
	    if ((cp = getenv(VARDEBUGFD1)) == NULL)
	        cp = getenv(VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
#endif /* CF_DEBUGS */

#if	CF_DEBUGMALL && (CF_DEBUGS || CF_DEBUG)
	uc_mallset(1) ;
#endif

	proginfo_start(pip,environ,argv[0],VERSION) ;

	if ((cp = getenv(VARBANNER)) == NULL)
	    cp = BANNER ;

	proginfo_setbanner(pip,cp) ;


	if ((cp = getenv(VARERRORFNAME)) == NULL)
	    cp = STDERRFNAME ;

	if (strcmp(cp,STDERRFNAME) == 0)
	    rs1 = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	else
	    rs1 = bopen(&errfile,cp,"wca",0666) ;

	if (rs1 >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	}

/* initialize */

	pip->verboselevel = 1 ;

	pip->lip = &li ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0)
	    goto ret1 ;

/* start parsing the arguments */

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

	            rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

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
	                        if (avl)
	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

	                    }

	                    break ;

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

	                case argopt_help:
	                    f_help = TRUE ;
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

/* get-file */
	                case argopt_gf:
	                    lip->f.query = TRUE ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            getfname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            getfname = argp ;

	                    }

	                    break ;

/* output name */
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

/* DB name */
	                case argopt_db:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            dbname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            dbname = argp ;

	                    }

	                    break ;

/* dump file */
	                case argopt_dump:
	                    lip->have.dump = TRUE ;
	                    lip->final.dump = TRUE ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            lip->dumpfname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            lip->dumpfname = argp ;

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

	                    switch ((int) *akp) {

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

/* quiet mode */
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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* get a variable by name */
	                    case 'g':
	                        lip->f.query = TRUE ;
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            rs = locinfo_qkey(lip,argp,argl) ;

	                        break ;

/* list mode */
	                    case 'l':
	                        lip->have.list = TRUE ;
	                        lip->final.list = TRUE ;
	                        lip->f.list = TRUE ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.list = (rs > 0) ;
	                            }

	                        }

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
	                            rs = keyopt_loads(&akopts,argp,argl) ;

	                        break ;

/* set mode */
	                    case 's':
	                        lip->have.set = TRUE ;
	                        lip->final.set = TRUE ;
	                        lip->f.set = TRUE ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.set = (rs > 0) ;
	                            }

	                        }

	                        break ;

/* unique mode */
	                    case 'u':
	                        lip->have.uniq = TRUE ;
	                        lip->final.uniq = TRUE ;
	                        lip->f.uniq = TRUE ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.uniq = (rs > 0) ;
	                            }

	                        }

	                        break ;

/* verbose mode */
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

	                    } /* end switch */

	                    akp += 1 ;
	                    if (rs < 0)
	                        break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits as argument or not) */

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (rs < 0) {
	    ex = EX_USAGE ;
	    bprintf(pip->efp,
	        "%s: invalid argument specified (%d)\n",
	        pip->progname,rs) ;

	    usage(pip) ;

	    goto retearly ;
	}

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

/* program search name */

	proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    bprintf(pip->efp,"%s: sn=%s\n",
	        pip->progname,pip->searchname) ;

	}

/* help file */

	if (f_usage)
	    usage(pip) ;

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_usage || f_help)
	    goto retearly ;


	ex = EX_OK ;

/* load up the environment options */

	rs = procopts(pip,&akopts) ;

	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

	if (dbname == NULL)
	    dbname = DBNAME ;

	if (strchr(dbname,'/') == NULL) {
	    mkpath2(tmpdbfname,DBDNAME,dbname) ;
	    dbname = tmpdbfname ;
	}

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: dbname=%s\n",pip->progname,dbname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: f_flist=%u f_list=%u f_set=%u f_query=%u\n",
	        lip->final.list,lip->f.list,lip->f.set,lip->f.query) ;
#endif

	if ((! lip->final.list) && (! lip->f.set) && (! lip->f.query))
	    lip->f.list = TRUE ;

	if (lip->f.set && (! lip->final.test))
		lip->f.test = TRUE ;

	if (pip->debuglevel > 0) {
		bprintf(pip->efp,"%s: f_list=%u\n",
			pip->progname,lip->f.list) ;
		bprintf(pip->efp,"%s: f_set=%u\n",
			pip->progname,lip->f.set) ;
		bprintf(pip->efp,"%s: f_query=%u\n",
			pip->progname,lip->f.query) ;
	}

/* process any files on invocation */

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    if (cp[0] != '\0')
	        rs = procname(pip,cp) ;

	    if (rs < 0)
	        break ;

	} /* end for */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {

	    bfile	argfile, *afp = &argfile ;


	    if (strcmp(afname,"-") == 0)
	        afname = STDINFNAME ;

	    if (strcmp(afname,STDINFNAME) == 0)
	        rs = bopen(afp,BFILE_STDIN,"dr",0666) ;
	    else
	        rs = bopen(afp,afname,"r",0666) ;

	    if (rs >= 0) {

	        int	len ;

	        char	linebuf[LINEBUFLEN + 1] ;


	        while ((rs = breadline(afp,linebuf,LINEBUFLEN)) > 0) {

	            len = rs ;
	            if (linebuf[len - 1] == '\n')
	                len -= 1 ;

	            linebuf[len] = '\0' ;
	            cp = linebuf ;
	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            pan += 1 ;
	            rs = procname(pip,cp) ;
	            if (rs < 0)
	                break ;

	        } /* end while (reading lines) */

	        bclose(afp) ;

	    } else {

	        if (! pip->f.quiet) {

	            bprintf(pip->efp,
	                "%s: could not open the argument list file\n",
	                pip->progname) ;

	            bprintf(pip->efp,"%s: \trs=%d argfile=%s\n",
	                pip->progname,rs,afname) ;

	        }

	    }

	} /* end if (processing file argument file list) */

	if (rs < 0)
	    goto done ;

#ifdef	COMMENT

/* load the system default variables if we are in "set" mode */

	if ((pan > 0) || lip->f.set) {

	    lip->f.set = TRUE ;

#ifdef	COMMENT
	    rs = procsysdefs(pip) ;
#endif

	    if (rs >= 0)
	        rs = procset(pip,dbname) ;

	    if (pip->debuglevel > 0) {
	        if (rs >= 0)
	            bprintf(pip->efp,"%s: loaded variables=%u\n",
	                pip->progname,rs) ;
	        else
	            bprintf(pip->efp,"%s: load failure (%d)\n",
	                pip->progname,rs) ;
	    }

	} /* end if (set mode) */

#else

	if ((pan > 0) || lip->f.set) {

	    lip->f.set = TRUE ;
	    if (rs >= 0)
	        rs = procload(pip,dbname) ;

	} /* end if */

#endif /* COMMENT */

	if ((rs >= 0) && if_int)
	    rs = SR_INTR ;

	if (rs < 0)
	    goto done ;

	if (lip->f.set && 
	    (! lip->f.list) && (! lip->f.query) && (! lip->f.audit))
	    goto done ;

/* open output file */

	if ((ofname == NULL) || (ofname[0] == '\0'))
	    ofname = STDOUTFNAME ;

	if (strcmp(ofname,STDOUTFNAME) == 0)
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;
	else
	    rs = bopen(ofp,ofname,"wct",0666) ;
	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: output unavailable (%d)\n",
	        pip->progname,rs) ;

	    goto badoutopen ;
	}

/* do the processing */

	rs = process(pip,dbname,ofp,getfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: process() rs=%d\n",rs) ;
#endif

/* finish and close up */

	bclose(ofp) ;

/* we're done */
badoutopen:
done:
	if ((rs < 0) && (! pip->f.quiet))
	    bprintf(pip->efp,
	        "%s: could not perform function (%d)\n",
	        pip->progname,rs) ;

	if ((rs < 0) && (ex == EX_OK)) {

	    ex = mapex(mapexs,rs) ;

	} else if (if_int)
	    ex = EX_INTR ;

/* early return thing */
retearly:
ret3:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

ret2:
	if (pip->open.akopts)
	    keyopt_finish(&akopts) ;

	locinfo_finish(lip) ;

ret1:
	bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

#if	CF_DEBUGMALL && (CF_DEBUGS || CF_DEBUG)
	{
	    int	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: mallout=%u\n",mo) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

/* restore and get out */

	j = 0 ;
	for (i = 0 ; sigints[i] != 0 ; i += 1)
	    u_sigaction(sigints[i],(sao + j++),NULL) ;

	for (i = 0 ; sigignores[i] != 0 ; i += 1)
	    u_sigaction(sigignores[i],(sao + j++),NULL) ;

	u_sigprocmask(SIG_SETMASK,&oldsigmask,NULL) ;

	return ex ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static void sighand_int(sn)
int	sn ;
{


	if_int = TRUE ;
}
/* end subroutine (sighand_int) */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen ;


	wlen = 0 ;
	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [-s] [<file(s)> ...]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s: \t[-Q] [-D] [-v[=n]] [-HELP] [-V]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int locinfo_start(lip,pip)
struct locinfo	*lip ;
struct proginfo	*pip ;
{
	int	rs = SR_OK ;
	int	size ;


	memset(lip,0,sizeof(struct locinfo)) ;

	lip->pip = pip ;

#if	CF_HDBSTR
	rs = hdbstr_start(&lip->vars,DEFNVARS) ;
	lip->f.vars_init (rs > 0) ;
	if (rs < 0)
	    goto bad0 ;
#endif

	rs = vecstr_start(&lip->queries,10,0) ;
	if (rs < 0)
	    goto bad1 ;

	size = sizeof(struct filemap) ;
	rs = vecobj_start(&lip->files,size,10,0) ;
	if (rs < 0)
	    goto bad2 ;

	rs = hdb_start(&lip->mvars,DEFNVARS,TRUE,NULL,NULL) ;
	if (rs < 0)
	    goto bad3 ;

	lip->f.uniq = TRUE ;

ret0:
	return rs ;

bad3:
	vecobj_finish(&lip->files) ;

bad2:
	vecstr_finish(&lip->queries) ;

bad1:

#if	CF_HDBSTR
	if (pip->f.vars_init) {
	     pip->f.vars_init = FALSE ;
	     hdbstr_finish(&lip->vars) ;
	}
#endif

bad0:
	goto ret0 ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(lip)
struct locinfo	*lip ;
{
	struct filemap	*fmp ;

	int	rs = SR_OK ;
	int	i ;


	hdb_finish(&lip->mvars) ;

	for (i = 0 ; vecobj_get(&lip->files,i,&fmp) >= 0 ; i += 1) {
	    if (fmp == NULL) continue ;
	    filemap_free(fmp) ;
	} /* end for */

	vecobj_finish(&lip->files) ;

	vecstr_finish(&lip->queries) ;

#if	CF_HDBSTR
	if (lip->f.vars_init) {
	    lip->f.vars_init = FALSE ;
	    hdbstr_finish(&lip->vars) ;
	}
#endif /* CF_HDBSTR */

	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_name(lip,fname)
struct locinfo	*lip ;
const char	fname[] ;
{
	struct proginfo	*pip = lip->pip ;

	struct filemap	fm ;

	struct ustat	sb ;

	int	rs = SR_OK ;


	if (fname == NULL)
	    return SR_FAULT ;

	rs = u_stat(fname,&sb) ;
	if (rs < 0)
	    goto ret0 ;

	lip->tsize += sb.st_size ;
	memset(&fm,0,sizeof(struct filemap)) ;

	rs = uc_mallocstrw(fname,-1,&fm.fname) ;
	if (rs < 0)
	    goto bad0 ;

	rs = vecobj_add(&lip->files,&fm) ;
	if (rs < 0)
	    goto bad1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("locinfo_name: vecobj_add() rs=%d\n",rs) ;
#endif

ret0:
	return rs ;

bad1:
	uc_free(fm.fname) ;

bad0:
	goto ret0 ;
}
/* end subroutine (locinfo_name) */


static int locinfo_fileload(lip,fmp)
struct locinfo	*lip ;
struct filemap	*fmp ;
{
	struct ustat	sb ;

	size_t	fsize = 0 ;

	int	rs = SR_OK ;
	int	mprot ;
	int	mflags ;
	int	fd = -1 ;
	int	oflags = (O_RDONLY) ;

	caddr_t	fmap = NULL ;

	const char	*mp ;


	rs = u_open(fmp->fname,oflags,0666) ;
	fd = rs ;
	if (rs < 0)
	    goto ret0 ;

	rs = u_fstat(fd,&sb) ;
	if (rs < 0)
	    goto ret1 ;

	if (sb.st_size == 0)
	    goto ret1 ;

	fsize = sb.st_size ;
	mprot = PROT_READ ;
	mflags = MAP_SHARED ;
	rs = u_mmap(NULL,(size_t) fsize,mprot,mflags,fd,0L,&fmap) ;
	if (rs < 0)
	    goto ret1 ;

	fmp->fmap = fmap ;
	fmp->fsize = fsize ;
	rs = locinfo_fileproc(lip,fmp) ;

	if ((rs < 0) && (fmp->fmap != NULL)) {
	    u_munmap(fmp->fmap,fmp->fsize) ;
	    fmp->fmap = NULL ;
	    fmp->fsize = 0 ;
	}

ret1:
	if (fd >= 0) {
	    u_close(fd) ;
	    fd = -1 ;
	}

ret0:
	return rs ;
}
/* end subroutine (locinfo_fileload) */


static int locinfo_fileproc(lip,fmp)
struct locinfo	*lip ;
struct filemap	*fmp ;
{
	struct proginfo	*pip = lip->pip ;

	HDB_DATUM	key, val ;

	FIELD	fsb ;

	int	rs = SR_OK ;
	int	ml ;
	int	ll ;
	int	len ;
	int	kl ,vl, fl ;
	int	fileoff = 0 ;
	int	c = 0 ;

	const char	*mp ;
	const char	*tp ;
	const char	*lp ;

	char	keybuf[KEYBUFLEN + 1] ;
	char	*kp, *vp, *fp ;


	mp = (const char *) fmp->fmap ;
	ml = fmp->fsize ;
	while ((tp = strnchr(mp,ml,'\n')) != NULL) {

	    len = ((tp + 1) - mp) ;
	    lp = mp ;
	    ll = (len - 1) ;

#if	CF_DEBUG 
	    if (DEBUGLEVEL(3))
	        debugprintf("locinfo_fileproc: l=>%t<\n",
	            lp,strnnlen(lp,ll,40)) ;
#endif

	    if ((ll > 0) && (lp[0] != '#')) {

	        if ((tp = strnchr(lp,ll,'#')) != NULL)
	            ll = (tp - lp) ;

	        if ((rs = field_start(&fsb,lp,ll)) >= 0) {

	            kl = field_get(&fsb,fterms,&kp) ;

	            if ((kl > 0) && (! hasweird(kp,kl))) {

	                    if (kl > KEYBUFLEN)
	                        kl = KEYBUFLEN ;

	                vp = (kp + kl) ;
	                vl = 0 ;
	                if (fsb.term != '#') {
	                    fl = field_get(&fsb,fterms,&fp) ;
	                    if (fl >= 0) {
				vp = fp ;
	                        vl = fl ;
			    }
	                }

	                    c += 1 ;
			if (lip->f.test) {
	                    key.buf = kp ;
	                    key.len = kl ;
	                    val.buf = vp ;
	                    val.len = vl ;
	                    rs = hdb_store(&lip->mvars,key,val) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("locinfo_fileproc: hdb_store() rs=%d\n",rs) ;
	        debugprintf("locinfo_fileproc: k=%t v=>%t<\n",
			kp,kl,
	            vp,strnnlen(vp,vl,40)) ;
	}
#endif

			    } /* end if (test will come later) */

#if	CF_HDBSTR
	                    if (rs >= 0) {
	                        rs = hdbstr_add(&lip->vars,kp,kl,vp,vl) ;
	                    }
#else
	                    if (rs >= 0) {
	                        strwcpy(keybuf,kp,kl) ;
	                        rs = varmks_addvar(&lip->mk,keybuf,vp,vl) ;
	                    }
#endif /* CF_HSBSTR */

	            } /* end if (have a key and is not weirdo) */

	            field_finish(&fsb) ;
	        } /* end if (field) */

	    } /* end if (non-zero-length line) */

	    if ((rs >= 0) && if_int)
	        rs = SR_INTR ;

	    if (rs < 0)
	        break ;

	    fileoff += len ;
	    mp += len ;
	    ml -= len ;

	} /* end while (readling lines) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("locinfo_fileproc: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_fileproc) */


static int locinfo_varcount(lip)
struct locinfo	*lip ;
{
	int	rs = SR_OK ;


#if	CF_HDBSTR
	rs = hdbstr_count(&lip->vars) ;
#endif

ret0:
	return rs ;
}
/* end subroutine (locinfo_varcount) */


static int locinfo_qkey(lip,qp,ql)
struct locinfo	*lip ;
const char	*qp ;
int		ql ;
{
	FIELD	fsb ;

	int	rs ;
	int	kl ;

	char	*kp ;


	rs = field_start(&fsb,qp,ql) ;
	if (rs < 0)
	    goto ret0 ;

	while ((kl = field_get(&fsb,qterms,&kp)) > 0) {

	    if (kl == 0) continue ;

	    rs = vecstr_add(&lip->queries,kp,kl) ;
	    if (rs < 0)
	        break ;

	} /* end while */

	field_finish(&fsb) ;

ret0:
	return rs ;
}
/* end subroutine (locinfo_qkey) */


static int locinfo_addvar(lip,cp,cl)
struct locinfo	*lip ;
const char	*cp ;
int		cl ;
{
	HDBSTR	*varp = &lip->vars ;

	int	rs = SR_OK ;
	int	rs1 = SR_OK ;
	int	kl, vl ;
	int	c = 0 ;

	const char	*tp ;
	const char	*kp, *vp ;


	kp = cp ;
	kl = cl ;
	vp = NULL ;
	vl = 0 ;
	if ((tp = strnchr(cp,cl,'=')) != NULL) {
	    vp = (tp + 1) ;
	    vl = -1 ;
	    kl = (tp - cp) ;
	}

	rs1 = SR_NOTFOUND ;
	if (lip->f.uniq)
	    rs1 = hdbstr_fetch(varp,kp,kl,NULL,NULL) ;

	if ((rs1 == SR_NOTFOUND) || (! lip->f.uniq)) {
	    c += 1 ;
	    rs = hdbstr_add(varp,kp,kl,vp,vl) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_addvar) */


/* process the program ako-options */
static int procopts(pip,kop)
struct proginfo	*pip ;
KEYOPT		*kop ;
{
	struct locinfo	*lip = pip->lip ;

	KEYOPT_CUR	kcur ;

	int	rs = SR_OK ;
	int	oi ;
	int	kl, vl ;
	int	c = 0 ;

	const char	*kp, *vp ;

	char	*cp ;


	if ((cp = getenv(VAROPTS)) != NULL)
	    rs = keyopt_loads(kop,cp,-1) ;

	if (rs < 0)
	    goto ret0 ;

/* process program options */

	keyopt_curbegin(kop,&kcur) ;

	while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

/* get the first value for this key */

	    vl = keyopt_fetch(kop,kp,NULL,&vp) ;

/* do we support this option? */

	    if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	        switch (oi) {

	        case akoname_set:
	            if (! lip->final.set) {

	                lip->have.set = TRUE ;
	                lip->final.set = TRUE ;
	                lip->f.set = TRUE ;
	                if ((vl > 0) && ((rs = optbool(vp,vl)) >= 0))
	                    lip->f.set = (rs > 0) ;

	            }

	            break ;

	        case akoname_test:
	            if (! lip->final.test) {

	                lip->have.test = TRUE ;
	                lip->final.test = TRUE ;
	                lip->f.test = TRUE ;
	                if ((vl > 0) && ((rs = optbool(vp,vl)) >= 0))
	                    lip->f.test = (rs > 0) ;

	            }

	            break ;

	        case akoname_list:
	            if (! lip->final.list) {

	                lip->have.list = TRUE ;
	                lip->final.list = TRUE ;
	                lip->f.list = TRUE ;
	                if ((vl > 0) && ((rs = optbool(vp,vl)) >= 0))
	                    lip->f.list = (rs > 0) ;

	            }

	            break ;

	        case akoname_audit:
	            if (! lip->final.audit) {

	                lip->have.audit = TRUE ;
	                lip->final.audit = TRUE ;
	                lip->f.audit = TRUE ;
	                if ((vl > 0) && ((rs = optbool(vp,vl)) >= 0))
	                    lip->f.audit = (rs > 0) ;

	            }

	            break ;

	        case akoname_uniq:
	            if (! lip->final.uniq) {

	                lip->have.uniq = TRUE ;
	                lip->final.uniq = TRUE ;
	                lip->f.uniq = TRUE ;
	                if ((vl > 0) && ((rs = optbool(vp,vl)) >= 0))
	                    lip->f.uniq = (rs > 0) ;

	            }

	            break ;

	        case akoname_dump:
	            if (! lip->final.dump) {

	                lip->have.dump = TRUE ;
	                lip->final.dump = TRUE ;
	                if (vl > 0)
	                    lip->dumpfname = (char *) vp ;

	            }

	            break ;

	        } /* end switch */

	        c += 1 ;

	    } /* end if (valid option) */

	} /* end while (looping through key options) */

	keyopt_curend(kop,&kcur) ;

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procname(pip,fname)
struct proginfo	*pip ;
const char	fname[] ;
{
	struct locinfo	*lip = pip->lip ;

	int	rs ;


	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procname: fname=%s\n",fname) ;
#endif

	rs = locinfo_name(lip,fname) ;

ret0:
	return rs ;
}
/* end subroutine (procname) */


static int procload(pip,dbname)
struct proginfo	*pip ;
const char	dbname[] ;
{
	struct locinfo	*lip = pip->lip ;

	struct filemap	*fmp ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	oflags ;
	int	operms ;
	int	i ;
	int	n ;
	int	c = 0 ;


	if (dbname == NULL)
	    return SR_FAULT ;

	if (dbname[0] == '\0')
	    goto ret0 ;

	oflags = O_CREAT ;
	operms = 0666 ;
	n = ((lip->tsize / 4) + 1) ;
	rs = varmks_open(&lip->mk,dbname,oflags,operms,n) ;
	if (rs < 0)
	    goto ret0 ;

	for (i = 0 ; vecobj_get(&lip->files,i,&fmp) >= 0 ; i += 1) {
	    if (fmp == NULL) continue ;
	    rs = locinfo_fileload(lip,fmp) ;
	    c += rs ;
	    if ((rs >= 0) && if_int)
	        rs = SR_INTR ;
	    if (rs < 0)
	        break ;
	} /* end for */

#if	CF_DEBUG || CF_DEBUGS
	if (DEBUGLEVEL(3))
	    debugprintf("main/procload: loop-after rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    varmks_abort(&lip->mk) ;

ret1:
	rs1 = varmks_close(&lip->mk) ;
	if (rs >= 0)
	    rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procload: varmks_close() \n") ;
#endif

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procload) */


static int process(pip,dbname,ofp,getfname)
struct proginfo	*pip ;
const char	dbname[] ;
void		*ofp ;
const char	getfname[] ;
{
	struct locinfo	*lip ;

	VARS		sv ;

	VARS_CUR	cur ;

	int	rs ;
	int	vl ;
	int	c = 0 ;

	char	kbuf[KBUFLEN + 1] ;
	char	vbuf[VBUFLEN + 1] ;


	lip = pip->lip ;

	rs = vars_open(&sv,dbname) ;
	if (rs < 0)
	    goto ret0 ;

	if (lip->f.audit) {

	    rs = vars_audit(&sv) ;

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: DB audit (%d)\n",
	            pip->progname,rs) ;

	} /* end if */

	if ((rs >= 0) && lip->f.set && lip->f.test) {
	    rs = procaudit(pip,&sv) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: procaudit() rs=%d\n",rs) ;
#endif

	}

	if ((rs >= 0) && lip->f.list) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: listing\n") ;
#endif

	    vars_curbegin(&sv,&cur) ;

	    while ((vl = vars_enum(&sv,&cur,kbuf,KBUFLEN,vbuf,VBUFLEN)) >= 0) {

	        c += 1 ;
	        rs = bprintf(ofp,"%s=%t\n",kbuf,vbuf,vl) ;

	        if ((rs >= 0) && if_int)
	            rs = SR_INTR ;

	        if (rs < 0)
	            break ;

	    } /* end while */

	    vars_curend(&sv,&cur) ;

	} /* end if (list mode) */

	if ((rs >= 0) && lip->f.query) {

	    int		i ;

	    char	*kp ;


	    for (i = 0 ; vecstr_get(&lip->queries,i,&kp) >= 0 ; i += 1) {

	        if (kp == NULL) continue ;

	        c += 1 ;
	        rs = procqkey(pip,ofp,&sv,kp,-1) ;
	        if (rs < 0)
	            break ;

	    } /* end for */

	    if ((rs >= 0) && (getfname != NULL))
	        rs = procgetfile(pip,ofp,&sv,getfname) ;

	} /* end if (query mode) */

	vars_close(&sv) ;

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (process) */


static int procaudit(pip,vlp)
struct proginfo	*pip ;
VARS		*vlp ;
{
	struct locinfo	*lip = pip->lip ;

	struct filemap	*fmp ;

	HDB_CUR	cur ;

	HDB_DATUM	key, val ;

	VARS_CUR	vcur ;

	int	rs = SR_OK ;
	int	kl, vl ;
	int	vlt = 0 ;
	int	f_debug = FALSE ;

	const char	*kp, *vp ;

	char	vbuf[VBUFLEN + 1] ;


	vbuf[0] = '\0' ;
	hdb_curbegin(&lip->mvars,&cur) ;

	while (hdb_enum(&lip->mvars,&cur,&key,&val) >= 0) {

		kp = (const char *) key.buf ;
		kl = key.len ;
		vp = (const char *) val.buf ;
		vl = val.len ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
		const char	*cp = "addability" ;
		int	cl ;
		cl = strlen(cp) ;
	f_debug = ((cl == kl) && (strncmp(cp,kp,kl) == 0)) ;
	}
#endif

#if	CF_DEBUG && 0
	if (DEBUGLEVEL(4))
	debugprintf("main/procaudit: sk=%t sv=%t\n",kp,kl,vp,vl) ;
#endif

	    vars_curbegin(vlp,&vcur) ;

	    while ((rs = vars_fetch(vlp,kp,kl,&vcur,vbuf,VBUFLEN)) >= 0) {

		vlt = rs ;

#if	CF_DEBUG 
	if (DEBUGLEVEL(4) && f_debug)
	debugprintf("main/procaudit: tv=>%t<\n",vbuf,vlt) ;
#endif

		if ((vl == vlt) && (strncmp(vbuf,vp,vl) == 0))
			break ;

		if (if_int)
		    rs = SR_INTR ;

	    } /* end while */

	    if (rs == SR_NOTFOUND) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4) && f_debug)
		debugprintf("main/procaudit: notfound k=%t v=%t vt=%t\n",
		kp,kl,
		vp,strnnlen(vp,vl,40),
		vbuf,strnnlen(vbuf,vlt,40)) ;
#endif

		bprintf(pip->efp,"%s: notfound k=%t v=%t\n",
		pip->progname,
		kp,kl,
		vp,strnnlen(vp,vl,40)) ;

	    } /* end if */

	    vars_curend(vlp,&vcur) ;

	    if ((rs >= 0) && if_int)
		rs = SR_INTR ;

	    if (rs < 0)
		break ;

	} /* end while */

	hdb_curend(&lip->mvars,&cur) ;

	return rs ;
}
/* end subroutine (procaudit) */


static int procgetfile(pip,ofp,vfp,getfname)
struct proginfo	*pip ;
void		*ofp ;
VARS		*vfp ;
const char	getfname[] ;
{
	FIELD	fsb ;

	bfile	getfile, *gfp = &getfile ;

	int	rs = SR_OK ;
	int	len ;
	int	cl, kl ;
	int	n = 0 ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	*cp, *kp ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procgetfile: getfname=%s\n",getfname) ;
#endif

	if (getfname == NULL)
	    goto ret0 ;

	if (getfname[0] == '\0')
	    goto ret0 ;

	if (strcmp(getfname,"-") == 0)
	    getfname = STDINFNAME ;

	rs = bopen(gfp,getfname,"r",0666) ;
	if (rs < 0)
	    goto ret0 ;

	while ((rs = breadline(gfp,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    if (linebuf[len - 1] == '\n')
	        len -= 1 ;

	    cp = linebuf ;
	    cl = len ;

	    if ((rs = field_start(&fsb,cp,cl)) >= 0) {

	        while ((kl = field_get(&fsb,qterms,&kp)) > 0) {

	            rs = procqkey(pip,ofp,vfp,kp,kl) ;
	            n += rs ;
	            if (rs < 0)
	                break ;

	            if (fsb.term == '#')
	                break ;

	        } /* end while (reading lines) */

	        field_finish(&fsb) ;
	    } /* end if (field) */

	    if (rs < 0) break ;
	} /* end while */

	bclose(gfp) ;

ret0:
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (procgetfile) */


static int procqkey(pip,ofp,vfp,kp,kl)
struct proginfo	*pip ;
void		*ofp ;
VARS		*vfp ;
const char	*kp ;
int		kl ;
{
	VARS_CUR	cur ;

	int	rs = SR_OK ;
	int	vl ;
	int	n = 0 ;

	char	vbuf[VBUFLEN + 1] ;


	if (kl < 0)
	    kl = strlen(kp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procqkey: k=%t\n",kp,kl) ;
#endif

	vars_curbegin(vfp,&cur) ;

	while ((vl = vars_fetch(vfp,kp,kl,&cur,vbuf,VBUFLEN)) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procqkey: v=>%t<\n",vbuf,vl) ;
#endif

	    if (n++ > 0)
	        rs = bprintf(ofp," ") ;

	    if (rs >= 0)
	        rs = bwrite(ofp,vbuf,vl) ;

	    if ((rs >= 0) && if_int)
	        rs = SR_INTR ;

	    if (rs < 0)
	        break ;

	} /* end while */

	vars_curend(vfp,&cur) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procqkey: fetch_loop rs=%d n=%u\n",rs,n) ;
#endif

	if (rs >= 0) {

	    if ((n == 0) && (pip->debuglevel > 0))
	        bprintf(pip->efp,"%s: notfound=%t\n",
	            pip->progname,kp,kl) ;

	    rs = bprintf(ofp,"%s\n",
	        ((n == 0) ? "*" : "")) ;

	} /* end if */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (procqkey) */


static int hasweird(sp,sl)
const char	*sp ;
int		sl ;
{
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; (i != sl) && (sp[i] != '\0') ; i += 1) {
	    const int	ch = MKCHAR(sp[i]) ;
	    f = ((! isalnumlatin(ch)) && (ch != '_')) ;
	    if (f) break ;
	} /* end if */

	return f ;
}
/* end subroutine (hasweird) */


static int filemap_free(fmp)
struct filemap	*fmp ;
{

	if (fmp->fmap != NULL) {
	    u_munmap(fmp->fmap,fmp->fsize) ;
	    fmp->fmap = NULL ;
	    fmp->fsize = 0 ;
	}

	if (fmp->fname != NULL) {
	    uc_free(fmp->fname) ;
	    fmp->fname = NULL ;
	}

	return SR_OK ;
}
/* end subroutine (filemap_free) */


#ifdef	COMMENT

static int procfile(pip,fname)
struct proginfo	*pip ;
const char	fname[] ;
{
	struct locinfo	*lip = pip->lip ;

	int	rs ;


	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

	rs = procvarfile(pip,fname) ;

ret0:
	return rs ;
}
/* end subroutine (procfile) */


static int procsysdefs(pip)
struct proginfo	*pip ;
{
	int	rs = SR_OK ;


	if (if_int)
	    rs = SR_INTR ;

	if (rs >= 0)
	    rs = procsysdef(pip,DEFINITFNAME) ;

	if (rs >= 0)
	    rs = procsysdef(pip,DEFLOGFNAME) ;

	return rs ;
}
/* end subroutine (procsysdefs) */


static int procsysdef(pip,fname)
struct proginfo	*pip ;
const char	fname[] ;
{
	struct locinfo	*lip = pip->lip ;

	vecstr	lvars ;

	int	rs ;
	int	i ;
	int	f ;

	char	*tp, *cp ;


	rs = vecstr_start(&lvars,10,0) ;
	if (rs < 0)
	    goto ret0 ;

	rs = vecstr_envfile(&lvars,fname) ;
	if (rs < 0)
	    goto ret1 ;

	for (i = 0 ; vecstr_get(&lvars,i,&cp) >= 0 ; i += 1) {

	    if (cp == NULL) continue ;

	    if ((tp = strchr(cp,'=')) == NULL) continue ;

	    f = (matstr(wstrs,cp,(tp - cp)) >= 0) ;
	    f = f || (matpstr(pstrs,10,cp,(tp - cp)) >= 0) ;
	    if (f) {

	        rs = locinfo_addvar(lip,cp,-1) ;
	        if (rs < 0)
	            break ;

	    } /* end if */

	} /* end for */

ret1:
	vecstr_finish(&lvars) ;

ret0:
	return rs ;
}
/* end subroutine (procsysdef) */


static int procset(pip,dbname)
struct proginfo	*pip ;
const char	dbname[] ;
{
	struct locinfo	*lip = pip->lip ;

	HDBSTR_CUR	cur ;

	VARMKS	svars ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	oflags = O_CREAT ;
	int	operms = 0644 ;
	int	vl = 0 ;
	int	c = 0 ;

	char	*kp, *vp ;


	if (dbname == NULL)
	    return SR_FAULT ;

	rs = varmks_open(&svars,dbname,oflags,operms,0) ;
	if (rs < 0)
	    goto ret0 ;

	hdbstr_curbegin(&lip->vars,&cur) ;

	while (hdbstr_enum(&lip->vars,&cur,&kp,&vp,&vl) >= 0) {

#if	CF_DEBUG
	    debugprintf("main/procset: k=%s v=>%t<\n",
	        kp,vp,vl) ;
#endif

	    c += 1 ;
	    rs = varmks_addvar(&svars,kp,vp,vl) ;

	    if ((rs >= 0) && if_int)
	        rs = SR_INTR ;

	    if (rs < 0)
	        break ;

	} /* end for */

	hdbstr_curend(&lip->vars,&cur) ;

/* done */

	if (rs < 0)
	    varmks_abort(&svars) ;

	rs1 = varmks_close(&svars) ;
	if (rs >= 0)
	    rs = rs1 ;

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procset) */


static int procvarfile(pip,fname)
struct proginfo	*pip ;
const char	fname[] ;
{
	struct locinfo	*lip = pip->lip ;

	HDBSTR	*varp ;

	FIELD	fsb ;

	FILEBUF	dfile, *dfp = &dfile ;

	int	rs ;
	int	rs1 = SR_OK ;
	int	len, cl ;
	int	fd ;
	int	kl, vl ;
	int	to = TO_READ ;
	int	c = 0 ;

	const char	*vp ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	vbuf[VBUFLEN + 1] ;
	char	*cp, *kp ;


	if (pip == NULL)
	    return SR_FAULT ;

	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

	varp = &lip->vars ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procvarfile: fname=%s\n",fname) ;
#endif

	rs = u_open(fname,O_RDONLY,0666) ;
	fd = rs ;
	if (rs < 0)
	    goto ret0 ;

	rs = filebuf_start(dfp,fd,0L,BUFLEN,0) ;
	if (rs < 0)
	    goto ret1 ;

	while ((rs = filebuf_readline(dfp,linebuf,LINEBUFLEN,to)) > 0) {

	    len = rs ;
	    if (linebuf[len - 1] == '\n')
	        len -= 1 ;

	    linebuf[len] = '\0' ;
	    cp = linebuf ;
	    cl = len ;
	    while ((cl > 0) && CHAR_ISWHITE(*cp)) {
	        cp += 1 ;
	        cl -= 1 ;
	    }

	    if ((cp[0] == '\0') || (cp[0] == '#'))
	        continue ;

	    if ((rs = field_start(&fsb,cp,cl)) >= 0) {

	        kl = field_get(&fsb,fterms,&kp) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("procvarfile: k=>%t<\n",kp,kl) ;
#endif

	        if ((kl == 6) && (strncasecmp(WORDEXPORT,kp,kl) == 0)) {

	            kl = field_get(&fsb,fterms,&kp) ;

	        } /* end if (elimination of 'export') */

	        if ((kl > 0) && (! hasweird(kp,kl))) {

	            rs1 = SR_OK ;
	            if (lip->f.uniq)
	                rs1 = hdbstr_fetch(varp,kp,kl,NULL,NULL) ;

	            if ((rs1 == SR_NOTFOUND) || (! lip->f.uniq)) {

	                vp = vbuf ;
	                vl = 0 ;
	                if (fsb.term != '#') {
	                    rs1 = field_sharg(&fsb,fterms,vbuf,VBUFLEN) ;
	                    if (rs1 >= 0)
	                        vl = rs1 ;
	                }

#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("procvarfile: v=>%t<\n",vp,vl) ;
#endif

	                    c += 1 ;
	                    rs = hdbstr_add(varp,kp,kl,vp,vl) ;

	            } /* end if (didn't have it already) */

	        } /* end if (have a variable keyname) */

	        field_finish(&fsb) ;
	    } /* end if (fields) */

	    if ((rs >= 0) && if_int) rs = SR_INTR ;
	    if (rs < 0) break ;
	} /* end while (reading lines) */

	filebuf_finish(dfp) ;

ret1:
	u_close(fd) ;

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procvarfile) */

#endif /* COMMENT */



