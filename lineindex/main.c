/* main */

/* main subroutine for the MKINDEX program */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

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
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>

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


/* local defines */

#define	MAXARGINDEX	(1000000 / 20)
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#ifndef	BUFLEN
#define	BUFLEN		((8 * 1024) + REALNAMELEN)
#endif

#ifndef	CMDBUFLEN
#define	CMDBUFLEN	(2 * 1024)
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX(BUFLEN,2048)
#endif


/* external subroutines */

extern uint	nextpowtwo(uint) ;

extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;

extern int	varsub_addvec(VARSUB *,VECSTR *) ;
extern int	varsub_subbuf(), varsub_merge() ;
extern int	expander() ;
extern int	proginfo_setpiv(PROGINFO *,const char *,
			const struct pivars *) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,
			const char *,const char *) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	progname(PROGINFO *, const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* externals variables */

extern char	makedate[] ;


/* forward references */

static int	usage(PROGINFO *) ;

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
	"of",
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
	argopt_of,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	PROGINFO	pi, *pip = &pi ;

	USERINFO	u ;

	CONFIGFILE	cf ;

	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	varsub	vsh_e, vsh_d ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, rs1 ;
	int	i, len ;
	int	len1, len2 ;
	int	sl, cl ;
	int	c ;
	int	loglen = -1 ;
	int	c_files = 0 ;
	int	c_proc = 0 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_makedate = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_append = FALSE ;
	int	f_noinput = FALSE ;
	int	f ;

	uchar	terms[32] ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1] ;
	char	buf2[BUFLEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*configfname = NULL ;
	const char	*logfname = NULL ;
	const char	*helpfname = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*indexname = NULL ;
	const char	*sp, *cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	} 

/* initialize */

	pip->verboselevel = 1 ;

	pip->tmpdname = getenv(VARTMPDNAME) ;

/* start parsing the arguments */

	rs = 0 ;
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

/* configuration file */
	                    case argopt_config:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                configfname = avp ;

	                        } else {

	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
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

/* version */
	                    case argopt_version:
				f_makedate = f_version ;
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

/* log file */
	                    case argopt_logfile:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                logfname = avp ;

	                        } else {

	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
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

/* handle all keyword defaults */
	                    default:
	                        rs = SR_INVALID ;

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

/* version */
	                        case 'V':
				f_makedate = f_version ;
	                            f_version = TRUE ;
	                            break ;

/* configuration file */
	                        case 'C':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                configfname = argp ;

	                            break ;

/* append to the key file */
	                        case 'a':
	                            f_append = TRUE ;
	                            break ;

/* file names to process is in this named file */
	                        case 'f':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

/* we allow a zero length string as a valid argument */
	                            afname = argp ;

	                            break ;

				case 'n':
					f_noinput = TRUE ;
					break ;

/* output file name */
	                        case 'o':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                ofname = argp ;

	                            break ;

				case 'q':
				    pip->verboselevel = 0 ;
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
	                            f_usage = TRUE ;

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
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	    bflush(pip->efp) ;

	}

	if (f_version) {

	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	    if (f_makedate) {

	    if ((cp = strchr(makedate,CH_RPAREN)) != NULL) {

	        cp += 1 ;
	        while (*cp && isspace(*cp))
	            cp += 1 ;

	    } else
	        cp = makedate ;

	    bprintf(pip->efp,"%s: makedate %s\n",
	        pip->progname,cp) ;

	    }
	}

	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto retearly ;

/* get our program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

/* program search name */

	proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

/* help */

	if (f_help)
	    goto help ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: initial programroot=%s\n",pip->pr) ;
#endif

	ex = EX_OK ;

/* get some host/user information */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

	if (rs < 0) {
		ex = EX_NOUSER ;
		goto retearly ;
	}

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->logid = u.logid ;

/* find a configuration file if we have one */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf( "main: checking for configuration file\n") ;
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

	    varsub_addva(&vsh_e,(const char **) envv) ;


	    if ((cf.root != NULL) && (pip->pr == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF programroot=%s\n",
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
	    debugprintf("main: final programroot=%s\n",pip->pr) ;
#endif

/* check program parameters */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking program parameters\n") ;
#endif


	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0'))
	    pip->tmpdname = TMPDNAME ;


/* do we have an activity log file? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 0 logfname=%s\n",logfname) ;
#endif

	rs1 = BAD ;
	if ((logfname == NULL) || (logfname[0] == '\0'))
	    logfname = LOGFNAME ;

	if ((logfname[0] == '/') || (u_access(logfname,W_OK) >= 0))
	    rs1 = logfile_open(&pip->lh,logfname,0,0666,u.logid) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if ((rs1 < 0) && (logfname[0] != '/')) {

	    mkpath2(tmpfname, pip->pr,logfname) ;

	    rs1 = logfile_open(&pip->lh,tmpfname,0,0666,u.logid) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 2 logfname=%s rs=%d\n",tmpfname,rs) ;
#endif

	} /* end if (we tried to open another log file) */

	if (rs1 >= 0) {

	    pip->open.log = TRUE ;
	    if (pip->daytime == 0)
		pip->daytime = time(NULL) ;

/* we opened it, maintenance this log file if we have to */

	    if (loglen < 0)
	        loglen = LOGSIZE ;

	    logfile_checksize(&pip->lh,loglen) ;

	    logfile_userinfo(&pip->lh,&u,
		pip->daytime,pip->progname,pip->version) ;

	} /* end if (we have a log file or not) */

/* print out the help file if requested */

	if (f_help) {

	    if ((helpfname == NULL) || (helpfname[0] == '\0'))
	        helpfname = HELPFNAME ;

	    printhelp(NULL,pip->pr,SEARCHNAME,helpfname) ;

	    goto done ;

	} /* end if (helpfname) */

/* some final initialization */


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

/* process the positional arguments */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf( "main: checking for positional arguments\n") ;
#endif

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
			c_files += 1 ;
			rs = progname(pip,cp) ;

	            if (rs >= 0)
			c_proc += 1 ;

	} /* end for (processing positional arguments) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: done processing argument files\n") ;
#endif

/* process any files in the argument filename list file */

	if ((afname != NULL) && (afname[0] != '\0')) {

	    bfile	afile ;


	    if (strcmp(afname,"-") != 0)
	        rs = bopen(&afile,afname,"r",0666) ;

	    else
	        rs = bopen(&afile,BFILE_STDIN,"dr",0666) ;

	    if (rs >= 0) {

		char	linebuf[LINEBUFLEN + 1] ;


	        while ((rs = breadline(&afile,linebuf,LINEBUFLEN)) > 0) {

		    len = rs ;
	            if (linebuf[len - 1] == '\n')
	                len -= 1 ;

	            linebuf[len] = '\0' ;
		    cp = linebuf ;
		    cl = len ;
	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

		    c_files += 1 ;
		    rs = progname(pip,cp) ;

	            if (rs >= 0)
			c_proc += 1 ;

	            if (rs < 0) {

	                if (*cp == '-')
	                    cp = "*stdinput*" ;

	                bprintf(pip->efp,
	                    "%s: error processing input file, rs=%d\n",
	                    pip->progname,rs) ;

	                bprintf(pip->efp,"%s: errored file=%s\n",
	                    pip->progname,cp) ;

	            } /* end if */

	        } /* end while (reading lines) */

	        bclose(&afile) ;

	    } else {

	        if (! pip->f.quiet) {

	            bprintf(pip->efp,
	                "%s: could not open the argument list file\n",
	                pip->progname) ;

	            bprintf(pip->efp,"%s: rs=%d argfile=%s\n",
			pip->progname,rs,afname) ;

	        }

	    }

	} /* end if (afname) */

/* process standard input */

#ifdef	COMMENT
	if ((c_files <= 0) && (! f_noinput)) {

			c_files += 1 ;
			rs = progname(pip,"-") ;

	            if (rs >= 0)
			c_proc += 1 ;

	    if (rs < 0) {

	        cp = "*stdinput*" ;
	        bprintf(pip->efp,"%s: error processing input file (%d)\n",
	            pip->progname,rs) ;

	        bprintf(pip->efp,"%s: errored file=%s\n",
	            pip->progname,cp) ;

	    }

	} /* end if (standard input) */
#endif /* COMMENT */

/* close the output file */

	bclose(ofp) ;

done:
badopenout:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: files=%u processed=%u\n",
	        pip->progname,c_files,c_proc) ;

/* we are done */
retearly:
ret3:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
		pip->progname,ex,rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

ret2:
	if (pip->open.log)
	    logfile_close(&pip->lh) ;

ret1:
	bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* USAGE> grope-dict [-C conf] [file(s) ...] [-V?] */
usage:
	usage(pip) ;

	goto retearly ;

/* help */
help:
	    ex = EX_INFO ;
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	    goto retearly ;

/* bad argument usage */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

badconfig:
	ex = EX_SOFTWARE ;
	bprintf(pip->efp,
	    "%s: error (%d) in configuration file starting at line %d\n",
	    pip->progname,rs,cf.badline) ;

	goto retearly ;

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could not open output (key) file (%d)\n",
	    pip->progname,rs) ;

	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [-C <conf>] <file>=<indexfile> [...] \n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
		"%s: \t[-af <argfile>] [-v[=n]] [-HELP] [-D[=n]] [-V]\n",
		pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


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



