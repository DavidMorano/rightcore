/* main */

/* test template */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_GETEXECNAME	1		/* try to use 'getexecname(3c)' ? */
#define	CF_MID		1		/* do new MID print-out */


/* revision history:

	= 2003-03-01, David A­D­ Morano
	I enhanced the program to also print out effective UID and
	effective GID.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ vcardadm


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<vecobj.h>
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

#define	O_FLAGS		O_RDONLY


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(struct proginfo *) ;
static int	cmp_utime(), cmp_ctime(), cmp_mtime() ;
static int	cmpr_utime(), cmpr_ctime(), cmpr_mtime() ;


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"HELP",
	"of",
	"a",
	"af",
	"db",
	"td",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_of,
	argopt_a,
	argopt_af,
	argopt_db,
	argopt_td,
	argopt_overlast
} ;

static const char *sortkeys[] = {
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


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct ustat	sb ;

	struct proginfo	pi, *pip = &pi ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;

	time_t	daytime ;

	pid_t	pid ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs = SR_OK ;
	int	rs1 ;
	int	len, i, j, k ;
	int	cl, ski, tdi ;
	int	nentries = DEFNENTRIES ;
	int	oflags ;
	int	(*cmpfunc)() ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_all = FALSE ;
	int	f_header = FALSE ;
	int	f_mid = FALSE ;
	int	f_sort = TRUE ;
	int	f_reverse = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*pr = NULL ;
	const char	*dbfname = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*recipient = NULL ;
	const char	*svcname = NULL ;
	const char	*sortkeyspec = NULL ;
	const char	*tdspec = NULL ;
	const char	*cp, *cp2 ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	msgidfnamebuf[MAXPATHLEN + 1] ;
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

	            if ((argl - 1) > 0)
	                rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

	        } else if (argp[1] == '-') {

	            ai_pos = ai ;
	            break ;

	            } else {

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

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
	                            if (avl) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                            }
	                        }

	                        break ;

	                    case argopt_help:
	                        f_help = TRUE ;
	                        break ;

	                    case argopt_a:
	                        f_all = TRUE ;
	                        break ;

/* database name (path) */
	                    case argopt_db:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                dbfname = avp ;

	                        } else {

	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                dbfname = argp ;

	                        }

	                        break ;

/* argument file name */
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

/* output file name */
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

	                    case argopt_td:
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            tdspec = argp ;

	                        break ;

/* handle all keyword defaults */
	                    default:
	                        rs = SR_INVALID ;
	                        bprintf(pip->efp,
	                            "%s: option (%s) not supported\n",
	                            pip->progname,akp) ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,
	                                        &pip->debuglevel) ;

	                                }
	                            }

	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

	                        case 'a':
	                            f_all = TRUE ;
	                            break ;

	                        case 'f':
	                            f_mid = FALSE ;
	                            break ;

	                        case 'h':
	                            f_header = TRUE ;
	                            break ;

	                        case 'm':
	                            f_mid = TRUE ;
	                            break ;

/* quiet mode */
	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

	                        case 'r':
	                            f_reverse = TRUE ;
	                            break ;

	                        case 's':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                sortkeyspec = argp ;

	                            break ;

/* verbose mode */
	                        case 'v':
	                            pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,
	                                        &pip->verboselevel) ;

	                                }
	                            }

	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
	                            break ;

	                        default:
	                            rs = SR_INVALID ;
	                            bprintf(pip->efp,
	                                "%s: unknown option - %c\n",
	                                pip->progname,*aop) ;

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

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
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


/* check arguments */

	if (! f_all) {

	    if (argvalue > 0)
	        nentries = argvalue ;

	} else
	    nentries = INT_MAX ;

/* sort key */

	ski = sortkey_utime ;
	if (sortkeyspec != NULL) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: sortkeyspec=%s f_rev=%u\n",
		sortkeyspec,f_reverse) ;
#endif

	    i = matostr(sortkeys,1,sortkeyspec,-1) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: matstr() i=%d\n",i) ;
#endif

	    if (i >= 0)
	        ski = i ;

	}

	switch (ski) {

	case sortkey_none:
	    break ;

	default:
	case sortkey_utime:
	case sortkey_utime2:
	    ski = sortkey_utime ;
	    cmpfunc = (f_reverse) ? cmpr_utime : cmp_utime ;
	    break ;

	case sortkey_mtime:
	case sortkey_mtime2:
	    ski = sortkey_mtime ;
	    cmpfunc = (f_reverse) ? cmpr_mtime : cmp_mtime ;
	    break ;

	case sortkey_ctime:
	case sortkey_ctime2:
	    ski = sortkey_ctime ;
	    cmpfunc = (f_reverse) ? cmpr_ctime : cmp_ctime ;
	    break ;

	} /* end switch */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: sortkey=%s(%u)\n",sortkeys[ski],ski) ;
#endif

/* time display (TD) */

	tdi = sortkey_mtime ;
	if (tdspec != NULL) {

	    i = matostr(sortkeys,1,tdspec,-1) ;

	    if (i >= 0) {

	        tdi = i ;
	        switch (tdi) {

	        case sortkey_utime2:
	            tdi = sortkey_utime ;
	            break ;

	        case sortkey_mtime2:
	            tdi = sortkey_mtime ;
	            break ;

	        case sortkey_ctime2:
	            tdi = sortkey_ctime ;
	            break ;

	        } /* end switch */

	    }
	}

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

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: positional ai=%d pan=%d arg=%s\n",
	                ai,pan,argv[ai]) ;
#endif

	        switch (pan) {

	        case 1:
	            recipient = argv[ai] ;
	            break ;

	        case 2:
	            svcname = argv[ai] ;
	            break ;

	        } /* end switch */

	        pan += 1 ;

	} /* end for (getting positional arguments) */


/* apply some defaults */

	if ((dbfname == NULL) || (dbfname[0] == '\0')) {

	    dbfname = msgidfnamebuf ;
	    mkpath2(msgidfnamebuf,pip->pr,VCARDADMDBNAME) ;

	}


/* open a block to enumerate the server entries */

	{
	    struct ustat	sb ;

	    MSGID		db ;

	    MSGID_CUR	cur ;

	    MSGID_ENT		e, *ep ;

	    MSGID_KEY	key ;

	    vecobj	entries ;

	    time_t	t ;

	    int	policy ;
	    int	c, tc, txid ;
	    int	ninsert = (f_all) ? -1 : nentries ;
	    int	f_done ;


/* prepare for the entries */

	    c = MIN(nentries,200) ;
	    policy = (ski != sortkey_none) ? VECOBJ_PSORTED : VECOBJ_PORDERED ;
	    rs = vecobj_start(&entries,sizeof(MSGID_ENT),
	        c,policy) ;

	    if (rs < 0) {

	        ex = EX_TEMPFAIL ;
	        bprintf(pip->efp,"%s: initialization failure (%d)\n",
	            pip->progname,rs) ;

	        goto done ;
	    }


/* open the database */

	    oflags = O_FLAGS ;
	    f = FALSE ;
	    f = f || ((oflags & O_RDWR) == O_RDWR) ;
	    rs = msgid_open(&db,dbfname,oflags,0666,4) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: 1 msgid_open() rs=%d\n",rs) ;
#endif

	    if ((rs == SR_ACCESS) && (! f))
	        rs = msgid_open(&db,dbfname,O_RDONLY,0666,4) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("main: 2 msgid_open() rs=%d\n",rs) ;
	        mkfnamesuf1(tmpfname,dbfname,"msgid") ;
	        u_stat(tmpfname,&sb) ;
	        debugprintf("main: fsize=%lu\n",sb.st_size) ;
	    }
#endif /* CF_DEBUG */

	    if (rs < 0) {

	        vecobj_finish(&entries) ;

	        ex = EX_OSFILE ;
	        bprintf(pip->efp,"%s: could not open MSGID database (%d)\n",
	            pip->progname,rs) ;

	        goto done ;
	    }


/* list all entries */

	    f_done = FALSE ;
	    for (tc = 0 ; (! f_done) && (tc < 20) ; tc += 1) {

#ifdef	COMMENT
	        rs = msgid_txbegin(&db) ;

	        txid = rs ;
#endif /* COMMENT */

	        msgid_curbegin(&db,&cur) ;

	        c = 0 ;
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

	                if (ski != sortkey_none) {

	                    rs1 = vecobj_inorder(&entries,&e,cmpfunc,ninsert) ;

	                    if ((rs1 >= 0) && (! f_all))
	                        vecobj_del(&entries,nentries) ;

	                } else
	                    rs1 = vecobj_add(&entries,&e) ;

	            }

	        } /* end while */

	        msgid_curend(&db,&cur) ;

#ifdef	COMMENT
	        rs = msgid_txcommit(&db,txid) ;

	        f_done = (rs >= 0) ;
#else
	        f_done = TRUE ;
#endif

	        if (! f_done) {

	            for (i = 0 ; vecobj_get(&entries,i,&ep) >= 0 ; i += 1) {

	                if (ep == NULL)
	                    continue ;

	                vecobj_del(&entries,i--) ;

	            }
	        }

	    } /* end for */

	    msgid_close(&db) ;

/* print out the extracted entries */

	    if (f_header) {

	        strcpyup(tmpfname,sortkeys[tdi]) ;

#if	CF_MID
	        bprintf(ofp,
	            "RECIP    %-14s COUNT %s\n",
	            tmpfname, "FROM") ;
#else /* CF_MID */
	        bprintf(ofp,
	            "RECIP    %-14s COUNT %s\n",
	            tmpfname,
	            ((f_mid) ? "MESSAGE-ID" : "FROM")) ;
#endif /* CF_MID */

	    }

	    for (i = 0 ; (i < nentries) && (vecobj_get(&entries,i,&ep) >= 0) ; 
	        i += 1) {

	        if (ep == NULL) continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            char	timebuf1[TIMEBUFLEN + 1] ;
	            char	timebuf2[TIMEBUFLEN + 1] ;
	            char	timebuf3[TIMEBUFLEN + 1] ;
	            debugprintf("main: tdi=%u ut=%s mt=%s ct=%s\n",
	                tdi,
	                timestr_log(ep->utime,timebuf1),
	                timestr_log(ep->mtime,timebuf2),
	                timestr_log(ep->ctime,timebuf3)) ;
	        }
#endif

	        switch (tdi) {

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
	        if (DEBUGLEVEL(4)) {
	            debugprintf("main: t=%s\n",
	                timestr_log(t,timebuf)) ;
	        }
#endif

	        bprintf(ofp,"%-8s %-14s %5u %s\n",
	            ep->recipient,
	            timestr_log(t,timebuf),
	            ep->count,
	            ep->from) ;

		if (f_mid)
	        	bprintf(ofp,"  %s\n",
			ep->messageid) ;

	    } /* end for (print-out loop) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: vecobj_count() rs=%d\n",
	            vecobj_count(&entries)) ;
#endif

	    vecobj_finish(&entries) ;

	} /* end block */

done:
	bclose(ofp) ;


	if (rs >= 0)
	    ex = EX_OK ;

	else if (ex != EX_INFO)
	    ex = EX_DATAERR ;


/* we are done */
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
	usage(pip) ;

	goto retearly ;

/* print out some help */
help:
	printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	goto retearly ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

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



static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;


	bprintf(pip->efp,
	    "%s: USAGE> %s [recipient(s) ...] [-a] [-s sortkey] [-m]\n",
	    pip->progname,pip->progname) ;

	rs = bprintf(pip->efp,
	    "%s: \t[-db dbname] [-td displaykey] [-r] [-?V] [-Dv]\n",
	    pip->progname) ;

	return rs ;
}
/* end subroutine (usage) */


static int cmp_utime(e1pp,e2pp)
void	**e1pp, **e2pp ;
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


static int cmpr_utime(e1pp,e2pp)
void	**e1pp, **e2pp ;
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


static int cmp_ctime(e1pp,e2pp)
void	**e1pp, **e2pp ;
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


static int cmpr_ctime(e1pp,e2pp)
void	**e1pp, **e2pp ;
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


static int cmp_mtime(e1pp,e2pp)
void	**e1pp, **e2pp ;
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


static int cmpr_mtime(e1pp,e2pp)
void	**e1pp, **e2pp ;
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
/* end subroutine (cmp_mtime) */



