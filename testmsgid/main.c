/* main */

/* test template */
/* last modified %G% version %I% */


#define	CF_DEBUGS	1		/* non-switchable debug print-outs */
#define	CF_DEBUG	1		/* switchable at invocation */
#define	CF_GETEXECNAME	1		/* try to use 'getexecname(3c)' ? */
#define	CF_NORMALTEST	0		/* do normal tests */
#define	CF_CLEANCOUNT	1		/* cleanup counts */


/* revision history:

	= 1999-03-01, David A­D­ Morano
	This was created for testing purposes.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Sysopsis:

	$ testmsgid.x


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<vecstr.h>
#include	<realname.h>
#include	<mallocstuff.h>
#include	<localmisc.h>
#include	<exitcodes.h>

#include	"config.h"
#include	"defs.h"
#include	"msgid.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	O_FLAGS		(O_RDWR | O_CREAT)


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	matstr(const char **,char *,int) ;
extern int	cfdeci(const char *,int,int *) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const char *argopts[] = {
	            "VERSION",
	            "VERBOSE",
	            "HELP",
	            "of",
	            NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_of,
	argopt_overlast

} ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	struct ustat	sb ;

	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		nisfile, *nfp = &nisfile ;

	time_t	daytime ;

	uid_t	uid ;

	pid_t	pid ;

	int	argr, argl, aol, akl, avl ;
	int	argvalue = -1 ;
	int	maxai, pan, npa, kwi, ai, i, j, k ;
	int	rs = SR_OK ;
	int	len ;
	int	sl, ci ;
	int	ex = EX_INFO ;
	int	fd_debug = -1 ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_self = FALSE ;
	int	f_entok = FALSE ;
	int	f_name = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*pr = NULL ;
	const char	*ofname = NULL ;
	const char	*srfname = NULL ;
	const char	*svcname = NULL ;
	const char	*un = NULL ;
	const char	*cp, *cp2 ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* initialize */

	pip->verboselevel = 1 ;

/* start parsing the arguments */

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while (argr > 0) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                if (((argl - 1) > 0) && 
	                    (cfdeci(argp + 1,argl - 1,&argvalue) < 0))
	                    goto badarg ;

	            } else {

#if	CF_DEBUGS
	                debugprintf("main: got an option\n") ;
#endif

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key w/ a value\n") ;
#endif

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

#if	CF_DEBUGS
	                    debugprintf("main: aol=%d avp=\"%s\" avl=%d\n",
	                        aol,avp,avl) ;

	                    debugprintf("main: akl=%d akp=\"%s\"\n",
	                        akl,akp) ;
#endif

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

	                }

/* do we have a keyword match or should we assume only key letters ?
					    */

#if	CF_DEBUGS
	                debugprintf("main: about to check for a key word match\n") ;
#endif

	                if ((kwi = matstr(argopts,akp,akl)) >= 0) {

#if	CF_DEBUGS
	                    debugprintf("main: option keyword=%t kwi=%d\n",
	                        akp,akl,kwi) ;
#endif

	                    switch (kwi) {

/* version */
	                    case argopt_version:
	                        f_version = TRUE ;
	                        if (f_optequal)
	                            goto badargextra ;

	                        break ;

/* verbose mode */
	                    case argopt_verbose:
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }
	                        }

	                        break ;

	                    case argopt_help:
	                        f_help = TRUE ;
	                        break ;

/* output file name */
	                    case argopt_of:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                ofname = avp ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                ofname = argp ;

	                        }

	                        break ;

/* handle all keyword defaults */
	                    default:
	                        ex = EX_USAGE ;
	                        f_usage = TRUE ;
	                        bprintf(pip->efp,
	                            "%s: option (%s) not supported\n",
	                            pip->progname,akp) ;

	                        goto badarg ;

	                    } /* end switch */

	                } else {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key letter\n") ;
#endif

	                    while (akl--) {

#if	CF_DEBUGS
	                        debugprintf("main: option key letters\n") ;
#endif

	                        switch ((int) *akp) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,
	                                        &pip->debuglevel) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                }
	                            }

	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

	                        case 'n':
	                            f_name = TRUE ;
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
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,
	                                        &pip->verboselevel) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                }
	                            }

	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
	                            break ;

	                        default:
	                            ex = EX_USAGE ;
	                            f_usage = TRUE ;
	                            bprintf(pip->efp,
	                                "%s: unknown option - %c\n",
	                                pip->progname,*aop) ;

	                        } /* end switch */

	                        akp += 1 ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minux sign character alone on the command line */

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

#if	CF_DEBUGS
	            debugprintf("main: pos arg=>%s<\n",argv[i]) ;
#endif

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                ex = EX_USAGE ;
	                bprintf(pip->efp,"%s: extra arguments specified\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: finished parsing arguments\n") ;
#endif

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: special debugging turned on\n") ;
#endif


/* get the program root */

	if (pr == NULL) {

	    pr = getenv(VARPROGRAMROOT1) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT2) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT3) ;

/* try to see if a path was given at invocation */

	    if ((pr == NULL) && (pip->progdname != NULL))
	        proginfo_rootprogdname(pip) ;

/* do the special thing */

#if	CF_GETEXECNAME && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	    if ((pr == NULL) && (pip->pr == NULL)) {

	        const char	*pp ;


	        pp = getexecname() ;

	        if (pp != NULL)
	            proginfo_execname(pip,pp) ;

	    }
#endif /* SOLARIS */

	} /* end if (getting a program root) */

	if (pip->pr == NULL) {

	    if (pr == NULL)
	        pr = PROGRAMROOT ;

	    pip->pr = mallocstr(pr) ;

	}


/* program search name */

	proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;


/* help file */

	if (f_help)
	    goto help ;


/* other initialization */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv("TMPDIR") ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;


/* other intialization */

	nodename[0] = '\0' ;
	domainname[0] = '\0' ;
	ex = EX_OK ;


/* open the output file */

	if (ofname != NULL)
	    rs = bopen(ofp,ofname,"wct",0666) ;

	else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0)
	    goto badoutopen ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: done w/ opening output \n") ;
#endif


/* loop through the arguments processing them */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: checking for positional arguments\n") ;
#endif

	pan = 1 ;
	for (ai = 0 ; ai <= maxai ; ai += 1) {

	    if (BATST(argpresent,ai)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: positional ai=%d pan=%d arg=%s\n",
	                ai,pan,argv[ai]) ;
#endif

	        switch (pan) {

	        case 1:
	            srfname = argv[ai] ;
	            break ;

	        case 2:
	            svcname = argv[ai] ;
	            break ;

	        } /* end switch */

	        pan += 1 ;

	    } /* end if */

	} /* end for (getting positional arguments) */


/* apply some defaults */

	if ((srfname == NULL) || (srfname[0] == '\0'))
	    srfname = "test" ;


/* open a block to enumerate the server entries */

	{
	    struct ustat	sb ;

	    MSGID		db ;

	    MSGID_CUR	cur ;

	    MSGID_ENT		e ;

	    MSGID_KEY	key ;

	    int	tc, txid ;
	    int	f_done ;

	    char	tmpfname[MAXPATHLEN + 1] ;
	    char	messageid[MSGIDE_LMESSAGEID + 1] ;
	    char	recipient[MSGIDE_LRECIPIENT + 1] ;


	    rs = msgid_open(&db,srfname,O_FLAGS,0666,4) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: 1 msgid_open() rs=%d\n",rs) ;
#endif

		if (rs == SR_ACCESS)
	    rs = msgid_open(&db,srfname,O_RDONLY,0666,4) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("main: 2 msgid_open() rs=%d\n",rs) ;
	        mkfnamesuf1(tmpfname,srfname,"msgidb") ;
	        u_stat(tmpfname,&sb) ;
	        debugprintf("main: fsize=%lu\n",sb.st_size) ;
	    }
#endif

/* list all entries */

	    f_done = FALSE ;
	    for (tc = 0 ; (! f_done) && (tc < 20) ; tc += 1) {

	        rs = msgid_txbegin(&db) ;

	        txid = rs ;

	        msgid_curbegin(&db,&cur) ;

	        while (msgid_enum(&db,&cur,&e) >= 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                debugprintf("main: 1 enum e.count=%u e.mtime=%s\n",
	                    e.count,
	                    timestr_logz(e.mtime,timebuf)) ;
	                debugprintf("main: e.recip=%s e.mid=%s\n",
	                    e.recipient,
	                    e.messageid) ;
	            }
#endif /* CF_DEBUG */

	            if (e.messageid[0] != '\0') {

	                bprintf(ofp,"1 c=%u t=%s recip=%s mid=%s\n",
	                    e.count,
	                    timestr_logz(e.mtime,timebuf),
	                    e.recipient,
	                    e.messageid) ;

	            }

	        } /* end while */

	        msgid_curend(&db,&cur) ;

	        rs = msgid_txcommit(&db,txid) ;

	        f_done = (rs >= 0) ;

	    } /* end for */

#if	CF_NORMALTEST

/* create an entry to write */

	    memset(&e,0,sizeof(MSGID_ENT)) ;

	    strwcpy(recipient,"person",MSGIDE_LRECIPIENT) ;

	    strwcpy(messageid,"hello123@here.there.com",MSGIDE_LMESSAGEID) ;

	    key.mid = messageid ;
	    key.midlen = -1 ;
	    key.recip = recipient ;
	    key.reciplen = -1 ;

	    daytime = time(NULL) ;

/* write this entry */

	    rs = msgid_update(&db,daytime,&key,&e) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("main: msgid_update() rs=%d\n",rs) ;
	        u_stat(tmpfname,&sb) ;
	        debugprintf("main: fsize=%lu\n",sb.st_size) ;
	    }
#endif


/* list all entries */

	    msgid_curbegin(&db,&cur) ;

	    while (msgid_enum(&db,&cur,&e) >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("main: 2 enum e.count=%u e.mtime=%s\n",
	                e.count,
	                timestr_logz(e.mtime,timebuf)) ;
	            debugprintf("main: e.recip=%s e.mid=%s\n",
	                e.recipient,
	                e.messageid) ;
	        }
#endif /* CF_DEBUG */

	        if (e.messageid[0] != '\0') {

	            bprintf(ofp,"2 c=%u t=%s recip=%s mid=%s\n",
	                e.count,
	                timestr_logz(e.mtime,timebuf),
	                e.recipient,
	                e.messageid) ;

	        }

	    } /* end while */

	    msgid_curend(&db,&cur) ;

#endif /* CF_NORMALTEST */

	    msgid_close(&db) ;

	} /* end block */


	bclose(ofp) ;



/* we are done */
done:
ret2:

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: program finishing\n",
	        pip->progname) ;

	bclose(ofp) ;

/* early return thing */
retearly:
ret1:
	bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

	return ex ;

/* the information type thing */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [username|- [keyword(s) ...]]\n",
	    pip->progname,pip->progname) ;

	bprintf(pip->efp,"%s: \t[-?V] [-Dv]\n",
	    pip->progname) ;

	goto retearly ;

/* print out some help */
help:
	printhelp(NULL,pip->pr,SEARCHNAME,HELPFNAME) ;

	goto retearly ;

/* the bad things */
badargnum:
	bprintf(pip->efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badargextra:
	bprintf(pip->efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(pip->efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: bad argument(s) given\n",
	    pip->progname) ;

	goto retearly ;

/* not found */
baduser1:
	ex = EX_NOUSER ;
	goto retearly ;

baduser2:
	ex = EX_NOUSER ;

#ifdef	COMMENT
	if (! pip->f.quiet)
	    bprintf(pip->efp,"%s: could not get information for user=%s (%d)\n",
	        pip->progname,un,rs) ;
#endif /* COMMENT */

	goto done ;

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could not open the output file (rs %d)\n",
	    pip->progname,rs) ;

	goto badret ;

/* error types of returns */
badret:
	bclose(pip->efp) ;

	goto retearly ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



