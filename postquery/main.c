/* main */

/* main subroutine for the MKKEY program */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1999-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is the 'main' module for the 'mkkey' program. This module processes
        the program invocation arguments and performs some preprocessing steps
        before any actual input files are scanned.

        The real work of processing the input files (one at a time) is performed
        by the 'process()' subroutine.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/utsname.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<varsub.h>
#include	<field.h>
#include	<ascii.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"configfile.h"
#include	"eigendb.h"


/* local defines */

#define	MAXARGINDEX	(1000000 / 20)
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#ifndef	BUFLEN
#define	BUFLEN		((8 * 1024) + REALNAMELEN)
#endif

#undef	LINEBUFLEN
#define	LINEBUFLEN		BUFLEN


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isdigitlatin(int) ;

extern int	process(struct proginfo *,bfile *,EIGENDB *,uchar *,
			char *,char *,char *) ;

extern int	varsub_load(), varsub_addvec(VARSUB *,VECSTR *) ;
extern int	varsub_subbuf(), varsub_merge() ;
extern int	expander() ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strshrink(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* externals variables */

extern char	makedate[] ;


/* forward references */

static int	eigenfind(struct proginfo *,char *,char *) ;

static char	*filereadable() ;


/* local structures */


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"CONFIG",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"LOGFILE",
	"HELP",
	"af",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_config,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_logfile,
	argopt_help,
	argopt_af,
	argopt_overlast
} ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo		pi, *pip = &pi ;

	USERINFO	u ;

	EIGENDB		eigendb ;

	CONFIGFILE	cf ;

	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		argfile, *afp = &argfile ;
	bfile		eigenfile, *ifp = &eigenfile ;

	varsub	vsh_e, vsh_d ;

	time_t	daytime = time(NULL) ;

	int	argr, argl, aol, akl, avl ;
	int	maxai, pan, npa, kwi, i ;
	int	rs, len ;
	int	ex = EX_INFO ;
	int	argnum, c ;
	int	loglen = -1 ;
	int	len1, len2 ;
	int	fd_debug ;
	int	sl, cl ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_append = FALSE ;

	uchar	terms[32] ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*pr = NULL ;
	const char	*configfname = NULL ;
	const char	*logfname = NULL ;
	const char	*helpfname = NULL ;
	const char	*afname = NULL ;
	const char	*eigenfname = NULL ;
	const char	*ofname = NULL ;
	const char	*ignorechars = IGNORECHARS ;
	const char	*delimiter = " " ;
	const char	*sp, *cp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp, *linebuf = buf ;
	char	buf2[BUFLEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

	pip->tmpdname = getenv("TMPDIR") ;
	pip->verboselevel = 1 ;
	pip->minwordlen = -2 ;
	pip->maxwordlen = -2 ;
	pip->eigenwords = -2 ;
	pip->keys = -2 ;

	pip->f.stderror = TRUE ;
	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	} else
	    pip->f.stderror = FALSE ;

#ifdef	COMMENT
	bprintf(efp,"%s: started\n",pip->progname) ;

	bflush(efp) ;
#endif

	eigenfname = NULL ;

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
		const int	ach = MKCHAR(argp[1]) ;

	        if (argl > 1) {

	            if (isdigitlatin(ach)) {

	                if (((argl - 1) > 0) && 
	                    (cfdeci(argp + 1,argl - 1,&argnum) < 0))
	                    goto badargnum ;

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

	                if ((kwi = matstr(argopts,akp,akl)) >= 0) {

	                    switch (kwi) {

/* configuration file */
	                    case argopt_config:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                configfname = avp ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                configfname = argp ;

	                        }

	                        break ;

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                pr = avp ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pr = argp ;

	                        }

	                        break ;

	                    case argopt_tmpdir:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->tmpdname = avp ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pip->tmpdname = argp ;

	                        }

	                        break ;

/* version */
	                    case argopt_version:
#if	CF_DEBUGS
	                        debugprintf("main: version key-word\n") ;
#endif
	                        f_version = TRUE ;
	                        if (f_optequal)
	                            goto badargextra ;

	                        break ;

/* verbose mode */
	                    case argopt_verbose:
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

/* log file */
	                    case argopt_logfile:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                logfname = avp ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                logfname = argp ;

	                        }

	                        break ;

/* print out the help */
	                    case argopt_help:
	                        f_help = TRUE ;
	                        break ;

	                    case argopt_af:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                afname = avp ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                afname = argp ;

	                        }

	                        break ;

/* handle all keyword defaults */
	                    default:
	                        ex = EX_USAGE ;
	                        bprintf(efp,"%s: option (%s) not supported\n",
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
	                                if (avl) {

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

/* configuration file */
	                        case 'C':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                configfname = argp ;

	                            break ;

/* append to the key file */
	                        case 'a':
	                            f_append = TRUE ;
	                            break ;

/* common words file (eigenfile) */
	                        case 'c':
	                        case 'e':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                eigenfname = argp ;

	                            break ;

/* entry delimiter string */
	                        case 'd':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

/* we want to store the pointer to zero length string if given */
	                            delimiter = argp ;

	                            break ;

/* file names to process is in this named file */
	                        case 'f':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

/* we allow a zero length string as a valid argument */
	                            afname = argp ;

	                            break ;

	                        case 'i':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                ignorechars = argp ;

	                            break ;

/* maximum number of keys written out */
	                        case 'k':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdeci(argp,argl,&pip->keys) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }

	                            break ;

	                        case 'l':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdeci(argp,argl,
	                                    &pip->minwordlen) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }

	                            break ;

	                        case 'm':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdeci(argp,argl,
	                                    &pip->maxwordlen) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }

	                            break ;

/* number of eigenwords to consider */
	                        case 'n':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdeci(argp,argl,
	                                    &pip->eigenwords) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }

	                            break ;

/* output file name */
	                        case 'o':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                ofname = argp ;

	                            break ;

/* quiet mode */
	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* remove labels */
	                        case 's':
	                            pip->f.removelabel = TRUE ;
	                            break ;

/* index whole files */
	                        case 'w':
	                            pip->f.wholefile = TRUE ;
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
	                            bprintf(efp,"%s: unknown option - %c\n",
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

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                bprintf(efp,"%s: extra arguments ignored\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: finished parsing arguments\n") ;
#endif


	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	    if ((cp = strchr(makedate,C_RPAREN)) != NULL) {

	        cp += 1 ;
	        while (*cp && isspace(*cp))
	            cp += 1 ;

	    } else
	        cp = makedate ;

	    bprintf(efp,"%s: makedate %s\n",
	        pip->progname,cp) ;

	}

	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto retearly ;

	if (pip->debuglevel > 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 0)
	        debugprintf("main: debuglevel %d\n",pip->debuglevel) ;
#endif

	    bprintf(efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	    bflush(efp) ;

	}


/* get our program root */

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

		proginfo_setprogroot(pip,pr,-1) ;

	    }

/* program search name */

	proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: initial pr=%s\n",pip->pr) ;
#endif

/* help */

	if (f_help)
	    goto help ;


	ex = EX_OK ;

/* get some host/user information */

	if ((rs = userinfo(&u,userbuf,USERINFO_LEN,NULL)) < 0) {

	    if ((pip->debuglevel > 0) && (! pip->f.quiet))
	        bprintf(efp,"%s: could not get user information\n",
	            pip->progname) ;

	}

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;

/* find a configuration file if we have one */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf(
	        "main: checking for configuration file\n") ;
#endif

/* search locally */

	if ((configfname == NULL) || (configfname[0] == '\0'))
	    configfname = filereadable(tmpfname,
	        NULL,ETCDIR1,CONFIGFILE1) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        NULL,ETCDIR1,CONFIGFILE2) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        NULL,ETCDIR2,CONFIGFILE1) ;

	if (configfname == NULL) {

	    configfname = CONFIGFILE1 ;
	    if (u_access(configfname,R_OK) < 0)
	        configfname = NULL ;

	}

	if (configfname == NULL) {

	    configfname = CONFIGFILE2 ;
	    if (u_access(configfname,R_OK) < 0)
	        configfname = NULL ;

	}

/* search in our program root area */

	if ((configfname == NULL) || (configfname[0] == '\0'))
	    configfname = filereadable(tmpfname,
	        pip->pr,ETCDIR1,CONFIGFILE1) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        pip->pr,ETCDIR1,CONFIGFILE2) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        pip->pr,ETCDIR2,CONFIGFILE1) ;


/* read in the configuration file if we have one */

	if (u_access(configfname,R_OK) >= 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1) debugprintf(
	        "main: we have a configuration file \"%s\"\n",
	        configfname) ;
#endif

	    if ((rs = configfile_start(&cf,configfname)) < 0)
	        goto badconfig ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we have a good configuration file\n") ;
#endif

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: varsubinit d\n") ;
#endif

	    varsub_start(&vsh_d,VARSUB_MBADNOKEY) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: varsubinit e\n") ;
#endif

	    varsub_start(&vsh_e,0) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: var_init\n") ;
#endif

	    varsub_load(&vsh_e,envv) ;


	    if ((cf.root != NULL) && (pip->pr == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF pr=%s\n",
	                cf.root) ;
#endif

	        if (((len1 = varsub_subbuf(&vsh_d,&vsh_e,cf.root,
	            -1,buf,BUFLEN)) > 0) &&
	            ((len2 = expander(pip,buf,len1,buf2,BUFLEN)) > 0)) {

	            proginfo_setprogroot(pip,buf2,len2) ;

	        }

	        cf.root = NULL ;

	    } /* end if (programroot) */

	    if ((cf.logfname != NULL) && (logfname == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF logfilename=%s\n",cf.logfname) ;
#endif

	        if (((len1 = varsub_subbuf(&vsh_d,&vsh_e,cf.logfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((len2 = expander(pip,buf,len1,buf2,BUFLEN)) > 0)) {

	            logfname = mallocstrw(buf2,len2) ;

	        }

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processed CF logfilename=%s\n",
			logfname) ;
#endif

	        cf.logfname = NULL ;

	    } /* end if (configuration file log filename) */

	    if ((cf.helpfname != NULL) && (helpfname == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF helpfname=%s\n",cf.helpfname) ;
#endif

	        if (((len1 = varsub_subbuf(&vsh_d,&vsh_e,cf.helpfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((len2 = expander(pip,buf,len1,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: helpfname subed and expanded\n") ;
#endif

	            helpfname = mallocstrw(buf2,len2) ;

	        }

	        cf.helpfname = NULL ;

	    } /* end if (helpfname) */

	    if ((cf.eigenfname != NULL) && (eigenfname == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF eigenfile=%s\n",cf.eigenfname) ;
#endif

	        if (((len1 = varsub_subbuf(&vsh_d,&vsh_e,cf.eigenfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((len2 = expander(pip,buf,len1,buf2,BUFLEN)) > 0)) {

	            eigenfname = mallocstrw(buf2,len2) ;

	        }

	        cf.eigenfname = NULL ;

	    } /* end if (eigenfname) */

	    if ((cf.tmpdir != NULL) && (pip->tmpdname == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF tmpdir=%s\n",cf.tmpdir) ;
#endif

	        if (((len1 = varsub_subbuf(&vsh_d,&vsh_e,cf.tmpdir,
	            -1,buf,BUFLEN)) > 0) &&
	            ((len2 = expander(pip,buf,len1,buf2,BUFLEN)) > 0)) {

	            pip->tmpdname = mallocstrw(buf2,len2) ;

	        }

	        cf.tmpdir = NULL ;

	    } /* end if (tmpdir) */


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: processing CF loglen\n") ;
#endif

	    if ((cf.loglen >= 0) && (loglen < 0))
	        loglen = cf.loglen ;

	    if ((cf.maxwordlen >= 0) && (pip->maxwordlen < 0))
	        pip->maxwordlen = cf.loglen ;

	    if ((cf.minwordlen >= 0) && (pip->minwordlen < 0))
	        pip->minwordlen = cf.minwordlen ;

	    if ((cf.keys >= 0) && (pip->keys < 0))
	        pip->keys = cf.keys ;


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: processing CF freeing data structures\n") ;
#endif

	    varsub_finish(&vsh_d) ;

	    varsub_finish(&vsh_e) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: processing CF free\n") ;
#endif


	    configfile_finish(&cf) ;

	} /* end if (accessed the configuration file) */


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: finished with any configfile stuff\n") ;
#endif


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: final pr=%s\n",pip->pr) ;
#endif


/* check program parameters */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking program parameters\n") ;
#endif


	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0'))
	    pip->tmpdname = TMPDNAME ;


/* clean up the path to the eigen word file, if we have one */

	rs = eigenfind(pip,eigenfname,tmpfname) ;

	if (rs > 0)
	    eigenfname = mallocstrw(tmpfname,rs) ;

	if (rs < 0)
	    eigenfname = NULL ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: eigenfname=%s\n",eigenfname) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: eigenfname=%s\n", pip->progname,eigenfname) ;


/* do we have an activity log file ? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 0 logfname=%s\n",logfname) ;
#endif

	rs = BAD ;
	if ((logfname == NULL) || (logfname[0] == '\0'))
	    logfname = LOGFNAME ;

	if ((logfname[0] == '/') || (u_access(logfname,W_OK) >= 0))
	    rs = logfile_open(&pip->lh,logfname,0,0666,u.logid) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if ((rs < 0) && (logfname[0] != '/')) {

	    mkpath2(tmpfname, pip->pr,logfname) ;

	    rs = logfile_open(&pip->lh,tmpfname,0,0666,u.logid) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 2 logfname=%s rs=%d\n",tmpfname,rs) ;
#endif

	} /* end if (we tried to open another log file) */

	if (rs >= 0) {

	    struct utsname	un ;


	    pip->f.log = TRUE ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we opened a logfile\n") ;
#endif

/* we opened it, maintenance this log file if we have to */

	    if (loglen < 0)
	        loglen = LOGSIZE ;

	    logfile_checksize(&pip->lh,loglen) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we checked its length\n") ;
#endif

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: making log entry\n") ;
#endif

	    logfile_printf(&pip->lh,"%s %s\n",
	        timestr_log(daytime,timebuf),
	        BANNER) ;

	    logfile_printf(&pip->lh,"%-14s %s/%s\n",
	        pip->progname,
	        VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;


	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        sprintf(buf,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        sprintf(buf,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        sprintf(buf,"(%s)",u.fullname) ;

	    logfile_printf(&pip->lh,"%s!%s %s\n",
	        u.nodename,u.username,buf) ;

	    (void) u_uname(&un) ;

	    logfile_printf(&pip->lh,"ostype=%s os=%s(%s) pid=%d\n",
	        u.f.sysv_rt ? "SYSV" : "BSD",
	        un.sysname,un.release,
	        u.pid) ;


	    if (eigenfname != NULL)
	        logfile_printf(&pip->lh,"eigenfile=%s\n",
	            eigenfname) ;

	} /* end if (we have a log file or not) */


/* print out the help file if requested */

	if (f_help) {

	    if ((helpfname == NULL) || (helpfname[0] == '\0'))
	        helpfname = HELPFNAME ;

	    printhelp(NULL,pip->pr,SEARCHNAME,helpfname) ;

	    goto done ;

	} /* end if (helpfname) */


/* some final initialization */

	if (pip->minwordlen < -1)
	    pip->minwordlen = MINWORDLEN ;

	if (pip->maxwordlen < -1)
	    pip->maxwordlen = MAXWORDLEN ;

	if (pip->eigenwords < -1)
	    pip->eigenwords = EIGENWORDS ;

	if (pip->keys < -1)
	    pip->keys = NKEYS ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {

	    debugprintf( "main: delimiter=>%s<\n",delimiter) ;

	    debugprintf( "main: keys=%d\n",pip->keys) ;

	}
#endif /* CF_DEBUG */


/* field terminators */

	for (i = 0 ; i < 32 ; i += 1)
	    terms[i] = 0xFF ;

	BACLR(terms,'_') ;

	for (c = 'a' ; c <= 'z' ; c += 1)
	    BACLR(terms,c) ;

	for (c = 'A' ; c <= 'Z' ; c += 1)
	    BACLR(terms,c) ;

	for (c = '0' ; c <= '9' ; c += 1)
	    BACLR(terms,c) ;


/* open the output key file */

	strcpy(buf,"wc") ;

	if (f_append)
	    strcat(buf,"a") ;

	 else
	    strcat(buf,"t") ;

	if ((ofname == NULL) || (ofname[0] == '\0')) {

	    strcat(buf,"d") ;

	    rs = bopen(ofp,BFILE_STDOUT,buf,0666) ;

	} else
	    rs = bopen(ofp,ofname,buf,0666) ;

	if (rs < 0)
	    goto badoutopen ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf( "main: opened output file rs=%d\n",rs) ;
#endif

/* open the eigenwords file and read them in to a DB */

	if (pip->eigenwords == 0)
	    len2 = 10 ;

	else if (pip->eigenwords < 0)
	    len2 = EIGENWORDS ;

	else
	    len2 = pip->eigenwords ;

	rs = eigendb_open(&eigendb,len2) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf( "main: opened eigen DB rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto badeigen ;


	if ((eigenfname != NULL) && (pip->eigenwords != 0) &&
	    (bopen(ifp,eigenfname,"r",0666) >= 0)) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf( "main: opened eigenfile rs=%d\n",rs) ;
#endif

	    rs = SR_OK ;
	    i = 0 ;
	    while ((rs = breadline(ifp,linebuf,LINEBUFLEN)) > 0) {
	        int	fl ;
		len = rs ;

	        if (linebuf[len - 1] == '\n') len -= 1 ;

	        sl = sfshrink(linebuf,len,&sp) ;

	        if ((sl > 0) && (*sp != '#')) {

	            while ((cl = nextfield(sp,sl,&cp)) > 0) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(6))
	                    debugprintf( "main: eigendb_add() \n") ;
#endif

	                if (cl >= pip->minwordlen) {

	                    rs = eigendb_add(&eigendb,cp,cl) ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4) && (rs < 0))
	                        debugprintf( "main: eigendb_add() rs=%d\n",rs) ;
#endif

	                    if (rs < 0)
	                        break ;

	                    if (pip->eigenwords > 0) {

	                        i += 1 ;
	                        if (i >= pip->eigenwords)
	                            break ;

	                    } /* end if */

	                } /* end if (eigenword met minimum word length) */

	                sl -= ((cp + cl) - sp) ;
	                sp = (cp + cl) ;

	            } /* end while (words) */

	        } /* end if */

	        if (rs < 0)
	            break ;

	        if ((pip->eigenwords > 0) && (i >= pip->eigenwords))
	            break ;

	    } /* end while (lines) */

	    bclose(ifp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(6)) {
	        EIGENDB_CUR	cur ;


	        eigendb_curbegin(&eigendb,&cur) ;

	        while ((cl = eigendb_enum(&eigendb,&cur,&cp)) >= 0) {

	            debugprintf("main: eigen> %t\n",cp,cl) ;
	        }

	        eigendb_curend(&eigendb,&cur) ;

	    }
#endif /* CF_DEBUG */

	} else
	    pip->eigenwords = 0 ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf( "main: finished reading eigenfile rs=%d\n",rs) ;
#endif


/* process the positional arguments */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf( "main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i) && (strlen(argv[i]) > 0)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf( "main: i=%d pan=%d arg=%s\n",
	                i,pan,argv[i]) ;
#endif

	        if (pip->debuglevel > 0)
	            bprintf(efp,"%s: processing input file \"%s\"\n",
	                pip->progname,argv[i]) ;


	        rs = process(pip,ofp,
	            &eigendb,terms,delimiter,ignorechars,argv[i]) ;

	        if (rs < 0) {

	            cp = argv[i] ;
	            if (*cp == '-')
	                cp = "*stdinput*" ;

	            bprintf(efp,"%s: error processing input file (%d)\n",
	                pip->progname,rs) ;

	            bprintf(efp,"%s: errored file=%s\n",
	                pip->progname,cp) ;

	        }

	        pan += 1 ;

	    } /* end if (got a positional argument) */

	} /* end for (processing positional arguments) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: done processing argument files\n") ;
#endif


/* process any files in the argument filename list file */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {

	    if (strcmp(afname,"-") != 0)
	        rs = bopen(afp,afname,"r",0666) ;

	    else
	        rs = bopen(afp,BFILE_STDIN,"dr",0666) ;

	    if (rs >= 0) {

		char	linebuf[LINEBUFLEN + 1] ;


	        while ((rs = breadline(afp,linebuf,LINEBUFLEN)) > 0) {

		    len = rs ;
	            if (linebuf[len - 1] == '\n')
	                len -= 1 ;

	            linebuf[len] = '\0' ;
	            cp = strshrink(linebuf) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            pan += 1 ;
	            rs = process(pip,ofp,
	                &eigendb,terms,delimiter,ignorechars,cp) ;

	            if (rs < 0) {

	                if (*cp == '-')
	                    cp = "*stdinput*" ;

	                bprintf(efp,
	                    "%s: error processing input file, rs=%d\n",
	                    pip->progname,rs) ;

	                bprintf(efp,"%s: errored file=%s\n",
	                    pip->progname,cp) ;

	            } /* end if */

	        } /* end while (reading lines) */

	        bclose(afp) ;

	    } else {

	        if (! pip->f.quiet) {

	            bprintf(efp,
	                "%s: could not open the argument list file\n",
	                pip->progname) ;

	            bprintf(efp,"\trs=%d argfile=%s\n",rs,afname) ;

	        }

	    }

	} else if (pan == 0) {

	    pan += 1 ;
	    rs = process(pip,ofp,
	        &eigendb,terms,delimiter,ignorechars,"-") ;

	    if (rs < 0) {

	        cp = "*stdinput*" ;
	        bprintf(efp,"%s: error processing input file, rs=%d\n",
	            pip->progname,rs) ;

	        bprintf(efp,"%s: errored file=%s\n",
	            pip->progname,cp) ;

	    }

	} /* end if (file list arguments or not) */

/* close out the eigenwords database */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: closing eigenwords DB\n") ;
#endif

	eigendb_close(&eigendb) ;


/* close the output file */

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("main: files processed=%u\n",pan) ;
	    debugprintf("main: closing output file\n") ;
	}
#endif

	bclose(ofp) ;


	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: %d files processed\n",
	        pip->progname,pan) ;


/* we are done */
done:
ret3:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: program finishing\n") ;
#endif

retearly:
ret2:
	logfile_close(&pip->lh) ;

ret1:
	bclose(efp) ;

ret0:
	proginfo_finish(pip) ;

	return ex ;

/* USAGE> grope-dict [-C conf] [file(s) ...] [-V?] */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [-C conf] [-d delim] [file(s) ...] [-?vV]\n",
	    pip->progname,pip->progname) ;

	bprintf(efp,
		"%s: \t[-l minwordlen] [-m maxwordlen] [-HELP] [-D[=n]]\n",
		pip->progname) ;

	bprintf(efp,"%s: \t[-n eigenwords] [-asw] [-c eigenfile]\n",
		pip->progname) ;

	bprintf(efp,"%s: \t[-i ignorechars] [-k nkeys]\n",
		pip->progname) ;

	goto retearly ;

/* help */
help:
	    ex = EX_INFO ;
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	    goto retearly ;

/* bad argument usage */
badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto ret2 ;

badconfig:
	bprintf(efp,
	    "%s: error (%d) in configuration file starting at line %d\n",
	    pip->progname,rs,cf.badline) ;

	goto badret ;

badoutopen:
	bprintf(efp,"%s: could not open output (key) file (%d)\n",
	    pip->progname,rs) ;

	goto badret ;

badeigen:
	bclose(ofp) ;

	bprintf(efp,"%s: could not initialize the eigen word database\n",
	    pip->progname) ;

	goto badret ;

badret:
	ex = EX_DATAERR ;
	goto done ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



/* is a file readable */
static char *filereadable(tmpfname,dir1,dir2,fname)
char	tmpfname[], dir1[], dir2[], fname[] ;
{


	if ((dir1 != NULL) && (dir2 != NULL))
	    sprintf(tmpfname,"%s/%s/%s",
	        dir1,dir2,fname) ;

	else if (dir1 != NULL)
	    sprintf(tmpfname,"%s/%s",
	        dir1,fname) ;

	else if (dir2 != NULL)
	    sprintf(tmpfname,"%s/%s",
	        dir2,fname) ;

	else
	    strcpy(tmpfname,fname) ;

	if (u_access(tmpfname,R_OK) >= 0)
	    return tmpfname ;

	return NULL ;
}
/* end subroutine (filereadable) */


/* find an eigen file if we can */
static int eigenfind(pip,eigenfname,tmpfname)
struct proginfo	*pip ;
char		eigenfname[] ;
char		tmpfname[] ;
{
	int	rs = SR_NOENT, i ;

	char	*fn ;


	tmpfname[0] = '\0' ;
	if ((eigenfname != NULL) && (eigenfname[0] != '\0')) {

	    rs = u_access(eigenfname,R_OK) ;

	    if (rs < 0) {

	        mkpath2(tmpfname, pip->pr,eigenfname) ;

	        rs = u_access(tmpfname,R_OK) ;

	    }

	} /* end if (given a filename) */

	for (i = 0 ; (rs < 0) && (i < 2) ; i += 1) {

	    fn = (i == 0) ? EIGENFNAME1 : EIGENFNAME2 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("eigenfind: fn=%s\n",fn) ;
#endif

	    tmpfname[0] = '\0' ;
	    rs = u_access(fn,R_OK) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("eigenfind: 1 u_access() rs=%d\n",rs) ;
#endif

	    if (rs >= 0)
	        strwcpy(tmpfname,fn,(MAXPATHLEN - 1)) ;

	    if ((rs < 0) && (fn[0] != '/')) {

	        mkpath2(tmpfname, pip->pr,fn) ;

	        rs = u_access(tmpfname,R_OK) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("eigenfind: 2 u_access() rs=%d\n",rs) ;
#endif

	    } /* end if */

	} /* end for */

	if (rs < 0) {

	    fn = EIGENFNAME ;
	    mkpath2(tmpfname, pip->pr,fn) ;

	    rs = u_access(tmpfname,R_OK) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("eigenfind: 3 u_access() rs=%d\n",rs) ;
#endif

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("eigenfind: ret rs=%d tmpfname=%s\n",rs,tmpfname) ;
#endif

	return (rs >= 0) ? strlen(tmpfname) : rs ;
}
/* end subroutine (eigenfind) */



