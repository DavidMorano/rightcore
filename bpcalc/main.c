/* main */

/* branch-prediction calculator */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1999-03-01, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ bpcalc


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<realname.h>
#include	<getxusername.h>
#include	<pwfile.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	PWENTRY_BUFLEN
#define	PWENTRY_BUFLEN		256
#endif


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	lastlogin(char *,uid_t,time_t *,char *,char *) ;
extern int	mkgecosname(char *,int,const char *) ;
extern int	isdigitlatin(int) ;

extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* forward references */


/* local global variables */


/* local structures */

/* define command option words */

static char *const argopts[] = {
	"VERSION",
	"VERBOSE",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_overlast
} ;

/* define the configuration keywords */

static char *const qopts[] = {
	    "logname",
	    "username",
	    "uid",
	    "gid",
	    "groupname",
	    "gecos",		/* 5 */
	    "gecosname",	/* 6 */
	    "mailname",
	    "homedir",
	    "shell",
	    "name",
	    "fullname",
	    "organization",
	    "nodename",
	    "hostname",
	    "domainname",
	    "nisdomain",
	    "password",
	    "passwd",
	    "euid",
	    "egid",
	    "realname",
	    "account",
	    "bin",
	    "office",
	    "wphone",
	    "hphone",
	    "printer",
	    "lstchg",
	    "lastlog",
	    NULL
} ;

enum qopts {
	qopt_logname,
	qopt_username,
	qopt_uid,
	qopt_gid,
	qopt_groupname,
	qopt_gecos,
	qopt_gecosname,
	qopt_mailname,
	qopt_homedir,
	qopt_shell,
	qopt_name,
	qopt_fullname,
	qopt_organization,
	qopt_nodename,
	qopt_hostname,
	qopt_domain,
	qopt_nisdomain,
	qopt_password,
	qopt_passwd,
	qopt_euid,
	qopt_egid,
	qopt_realname,
	qopt_account,
	qopt_bin,
	qopt_office,
	qopt_wphone,
	qopt_hphone,
	qopt_printer,
	qopt_lstchg,
	qopt_lastlog,
	qopt_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct passwd	*pep ;

	struct group	*gep = NULL ;

	struct proginfo	pi, *pip = &pi ;

	PWENTRY		entry ;

	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		nisfile, *nfp = &nisfile ;

	time_t	daytime = 0 ;

	uid_t	uid_cur ;

	int	argr, argl, aol, akl, avl ;
	int	argvalue = -1 ;
	int	maxai, pan, npa, kwi, i, j, k ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	len, rs ;
	int	sl, ci ;
	int	uid ;
	int	ex = EX_INFO ;
	int	fd_debug ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_self = FALSE ;
	int	f_entok = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;
	char	entrybuf[PWENTRY_BUFLEN + 1] ;
	const char	*ofname = NULL ;
	const char	*pwfname = NULL ;
	const char	*un = NULL ;
	const char	*cp, *cp2 ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	memset(pip,0,sizeof(struct proginfo)) ;

	pip->version = VERSION ;
	pip->progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    bcontrol(efp,BC_LINEBUF,0) ;
	}

/* initialize */

	pip->f.quiet = FALSE ;

	pip->debuglevel = 0 ;
	pip->verboselevel = 0 ;

	pip->programroot = NULL ;


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
	                    (cfdeci(argp + 1,argl - 1,&argvalue) < 0))
	                    goto badarg ;

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

/* do we have a keyword match or should we assume only key letters? */

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

#if	CF_DEBUGS
	                    debugprintf("main: option keyword=%W kwi=%d\n",
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
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }
	                        }

	                        break ;

/* handle all keyword defaults */
	                    default:
	                        ex = EX_USAGE ;
	                        f_usage = TRUE ;
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

/* alternate passwd file */
	                        case 'p':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pwfname = argp ;

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
			ex = EX_USAGE ;
	                bprintf(efp,"%s: extra arguments specified\n",
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
	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto earlyret ;

	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: special debugging turned on\n") ;
#endif


/* open the output file */

	if (ofname != NULL)
	    rs = bopen(ofp,ofname,"wct",0666) ;

	else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0)
	    goto badoutopen ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: done w/ opening output \n") ;
#endif


/* get the first positional argument as the username to key on */

	for (i = 0 ; i <= maxai ; i += 1) {
	    if (BATST(argpresent,i)) break ;
	} /* end for */

	if (i <= maxai)
	    BACLR(argpresent,i) ;

	if ((i > maxai) || (strcmp(argv[i],"-") == 0)) {

	    if (getusername(buf,BUFLEN,-1) >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("main: getusername() username=%s\n",buf) ;
#endif

	        f_self = TRUE ;
	        un = mallocstr(buf) ;

	    }

	} else
	    un = argv[i] ;


	uid_cur = u_getuid() ;

	f_entok = FALSE ;
	if (pwfname == NULL) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: about to get PW entry, un=%s\n",un) ;
#endif

	    if (un != NULL) {

	        rs = getpwentry_name(&entry,entrybuf,PWENTRY_BUFLEN,un) ;

	        if ((rs < 0) && isdigit(un[0])) {

	            if ((rs = cfdeci(un,-1,&uid)) >= 0)
	                rs = getpwentry_uid(&entry,
	                    entrybuf,PWENTRY_BUFLEN,uid) ;

	        }

	    } else
	        rs = getpwentry_uid(&entry,entrybuf,PWENTRY_BUFLEN,uid_cur) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: getpwentry() rs=%d\n",rs) ;
#endif

	} else {
	    PWFILE	pf ;

	    if ((rs = pwfile_open(&pf,pwfname)) >= 0) {

	        rs = SR_NOENT ;
	        if (un != NULL)
	            rs = pwfile_fetchuser(&pf,un,NULL,
	                &entry,entrybuf,PWENTRY_BUFLEN) ;

	        pwfile_close(&pf) ;
	    } /* end if (opened PWFILE DB) */

	} /* end if (system or file) */

	if (rs >= 0)
	    f_entok = TRUE ;

/* perform the default actions */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: default actions\n") ;
#endif

	if (strcmp(pip->progname,"logdir") == 0) {

	    if (rs >= 0) {

	        if (npa <= 1)
	            bprintf(ofp,"%s\n",entry.dir) ;

	    } else if (! pip->f.quiet)
	        bprintf(efp,"%s: user \"%s\" not found\n",
	            pip->progname,un) ;

	} else {

	    if (rs >= 0) {

	        if (npa <= 1)
	            bprintf(ofp,"%s\n",un) ;

	    } else if (! pip->f.quiet)
	        bprintf(efp,"%s: user \"%s\" not found\n",
	            pip->progname,un) ;

	} /* end if ('logdir' or 'logname') */


	ex = EX_NOUSER ;
	if (rs < 0)
	    goto baduser ;


/* initialize some stuff */

	nodename[0] = '\0' ;
	domainname[0] = '\0' ;
	ex = EX_OK ;


/* loop through the arguments processing them */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking for positional arguments\n") ;
#endif

	pan = 1 ;
	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: argument i=%d pan=%d arg=%s\n",
	                i,pan,argv[i]) ;
#endif

	        if ((ci = matostr(qopts,2,argv[i],-1)) >= 0) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: config keyword=%s i=%d\n",
	                    argv[i],ci) ;
#endif

	            switch (ci) {

	            case qopt_logname:
			case qopt_username:
	                cp = entry.username ;
	                bprintf(ofp,"%s\n",((cp != NULL) ? cp : "")) ;

	                break ;

	            case qopt_uid:
	                if (f_entok)
	                    bprintf(ofp,"%d\n",entry.uid) ;

	                    else
	                    bprintf(ofp,"\n") ;

	                break ;

	            case qopt_gid:
	                if (f_entok)
	                    bprintf(ofp,"%d\n",entry.gid) ;

	                    else
	                    bprintf(ofp,"\n") ;

	                break ;

	            case qopt_groupname:
	                if (f_self) {

	                    gid_t	gid_cur = u_getgid() ;


	                    gep = getgrgid(gid_cur) ;

	                } else if (f_entok)
	                    gep = getgrgid(entry.gid) ;

	                if (gep != NULL)
	                    bprintf(ofp,"%s\n",gep->gr_name) ;

	                else
	                    bprintf(ofp,"\n") ;

	                break ;

	            case qopt_gecos:
	                if (f_entok)
	                    bprintf(ofp,"%s\n", entry.gecos) ;

	                else
	                    bprintf(ofp,"\n") ;

	                break ;

	            case qopt_gecosname:
	                if (f_entok) {

	                    mkgecosname( buf,BUFLEN, entry.gecos) ;

	                    bprintf(ofp,"%s\n", buf) ;

	                } else
	                    bprintf(ofp,"\n") ;

	                break ;

	            case qopt_homedir:
	                if (f_entok)
	                    bprintf(ofp,"%s\n",entry.dir) ;

	                else
	                    bprintf(ofp,"\n") ;

	                break ;

	            case qopt_shell:
	                if (f_entok)
	                    bprintf(ofp,"%s\n",entry.shell) ;

	                else
	                    bprintf(ofp,"\n") ;

	                break ;

	            case qopt_name:
	                cp = NULL ;
	                if (f_self)
	                    cp = getenv("NAME") ;

	                if ((cp == NULL) && f_entok)
	                    cp = entry.realname ;

	                if (cp != NULL) {

	                    realname	rn = obj_realname(cp,-1) ;


	                    sl = realname_name(&rn,buf,BUFLEN) ;

	                    bprintf(ofp,"%s\n",buf) ;

	                    realname_free(&rn) ;

	                } else
	                    bputc(ofp,'\n') ;

	                break ;

	            case qopt_mailname:
	                cp = NULL ;
	                if (f_self)
	                    cp = getenv("NAME") ;

	                if ((cp == NULL) && f_entok)
	                    cp = entry.realname ;

	                if (cp != NULL) {

	                    realname	rn = obj_realname(cp,-1) ;


	                    sl = realname_mailname(&rn,buf,BUFLEN) ;

	                    bprintf(ofp,"%s\n",buf) ;

	                    realname_free(&rn) ;

	                } else
	                    bputc(ofp,'\n') ;

	                break ;

	            case qopt_fullname:
	                cp = NULL ;
	                if (f_self)
	                    cp = getenv("FULLNAME") ;

	                if ((cp == NULL) && f_entok)
	                    cp = entry.realname ;

	                if (cp != NULL) {

	                    realname	rn = obj_realname(cp,-1) ;


	                    sl = realname_fullname(&rn,buf,BUFLEN) ;

	                    bprintf(ofp,"%s\n",buf) ;

	                    realname_free(&rn) ;

	                } else
	                    bputc(ofp,'\n') ;

	                break ;

	            case qopt_organization:
	                cp = NULL ;
	                if (f_self)
	                    cp = getenv("ORGANIZATION") ;

	                if ((cp == NULL) && f_entok)
	                    cp = entry.organization ;

	                bprintf(ofp,"%s\n",((cp != NULL) ? cp : "")) ;

	                break ;

	            case qopt_nodename:
	                if (nodename[0] == '\0')
	                    getnodedomain(nodename,domainname) ;

	                bprintf(ofp,"%s\n", nodename) ;

	                break ;

#ifdef	COMMENT

	            case qopt_domain:
	                if (nodename[0] == '\0')
	                    getnodedomain(nodename,domainname) ;

	                bprintf(ofp,"%s\n", domainname) ;

	                break ;

#endif /* COMMENT */

	            case qopt_hostname:
	                if (nodename[0] == '\0')
	                    getnodedomain(nodename,domainname) ;

	                cp = "" ;
	                if (nodename[0] != '\0') {

	                    cp = nodename ;
	                    if (domainname[0] != '\0') {

	                        cp = buf ;
	                        bufprintf(buf,BUFLEN,
	                            "%s.%s",
	                            nodename,domainname) ;

	                    }

	                } /* end if (nodename) */

	                bprintf(ofp,"%s\n",cp) ;

	                break ;

	            case qopt_nisdomain:
	                cp = "" ;
	                sl = 1 ;
	                if (bopen(nfp,NISDOMAINNAME,"r",0666) >= 0) {

	                    if ((len = breadline(nfp,buf,BUFLEN)) > 0)
	                        sl = sfshrink(buf,len,&cp) ;

	                    bclose(nfp) ;

	                } /* end if (reading NIS file) */

/* remove all trailing dots from the NIS domain name */

	                while ((sl > 0) && (cp[sl - 1] == '.'))
	                    sl -= 1 ;

	                cp[sl] = '\0' ;
	                bprintf(ofp,"%W\n",cp,sl) ;

	                break ;

	            case qopt_password:
	            case qopt_passwd:
	                if (f_entok && (entry.password != NULL))
	                    bprintf(ofp,"%s\n",entry.password) ;

	                else
	                    bprintf(ofp,"\n") ;

	                break ;

	            case qopt_euid:
	                bprintf(ofp,"%d\n",u_geteuid()) ;

	                break ;

	            case qopt_egid:
	                bprintf(ofp,"%d\n",u_getegid()) ;

	                break ;

	            case qopt_realname:
	                cp = "" ;
	                if (f_entok && (entry.realname != NULL))
	                    cp = entry.realname ;

	                bprintf(ofp,"%s\n",cp) ;

	                break ;

	            case qopt_account:
	                cp = "" ;
	                if (f_entok && (entry.account != NULL))
	                    cp = entry.account ;

	                bprintf(ofp,"%s\n",cp) ;

	                break ;

	            case qopt_bin:
	                cp = "" ;
	                if (f_entok && (entry.bin != NULL))
	                    cp = entry.bin ;

	                bprintf(ofp,"%s\n",cp) ;

	                break ;

	            case qopt_office:
	                cp = "" ;
	                if (f_entok && (entry.office != NULL))
	                    cp = entry.office ;

	                bprintf(ofp,"%s\n",cp) ;

	                break ;

	            case qopt_wphone:
	                cp = "" ;
	                if (f_entok && (entry.wphone != NULL))
	                    cp = entry.wphone ;

	                bprintf(ofp,"%s\n",cp) ;

	                break ;

	            case qopt_hphone:
	                cp = "" ;
	                if (f_entok && (entry.hphone != NULL))
	                    cp = entry.hphone ;

	                bprintf(ofp,"%s\n",cp) ;

	                break ;

	            case qopt_printer:
	                cp = NULL ;
	                if (f_self)
	                    cp = getenv("PRINTER") ;

	                if ((cp == NULL) && f_entok)
	                    cp = entry.printer ;

	                bprintf(ofp,"%s\n",((cp != NULL) ? cp : "")) ;

	                break ;

	            case qopt_lstchg:
	                if (f_entok)
	                    bprintf(ofp,"%ld\n",entry.lstchg) ;

	                else
	                        bputc(ofp,'\n') ;

	                break ;

	            case qopt_lastlog:
	                if (f_entok) {

	                    time_t	t ;

	                    char	hostname[16 + 1], line[8 + 1] ;
	                    char	timebuf[TIMEBUFLEN + 1] ;


	                    rs = lastlogin(NULL,entry.uid,
	                        &t,hostname,line) ;

	                    if (rs >= 0) {

	                        bprintf(ofp,"%s %-8s %16s",
	                            timestr_log(t,timebuf),
	                            line,hostname) ;

	                        if (pip->verboselevel >= 1) {

	                            if (daytime <= 0)
	                                daytime = time(NULL) ;

	                            bprintf(ofp," (%17s)",
	                                timestr_elapsed((daytime - t),
	                                timebuf)) ;

	                        }

	                        bputc(ofp,'\n') ;

	                    } else
	                        bputc(ofp,'\n') ;

	                } else
	                        bputc(ofp,'\n') ;

	                break ;

			case qopt_domain:
	                	if (nodename[0] == '\0')
	                    		getnodedomain(nodename,domainname) ;
				bprintf(ofp,"%s\n",domainname) ;
	                break ;

	            default:
	                bprintf(efp,
	                    "%s: extra positional arguments ignored\n",
	                    pip->progname) ;

	            } /* end switch */

	        } else {

	            ex = EX_DATAERR ;
	            bprintf(ofp,"*\n") ;

	        } /* end if */

	    } /* end if (got a positional argument) */

	} /* end for (handling positional arguments) */

/* we are done */
done:
ret2:

#if	CF_DEBUG
	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: program finishing\n",
	        pip->progname) ;
#endif

	bclose(ofp) ;

/* early return thing */
earlyret:
ret1:
	bclose(efp) ;

ret0:
	return ex ;

/* the information type thing */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [username|- [keyword(s) ...]]\n",
	    pip->progname,pip->progname) ;

	bprintf(efp,"%s: \t[-?V] [-Dv]\n",
	    pip->progname) ;

	bprintf(efp,"%s:    possible configuration keywords are :\n",
	    pip->progname) ;

	for (i = 0 ; qopts[i] != NULL ; i += 1) {

	    if ((i % USAGECOLS) == 0)
	        bprintf(efp,"%s: \t",
	            pip->progname) ;

	    bprintf(efp,"%-16s",qopts[i]) ;

	    if ((i % USAGECOLS) == 3)
	        bprintf(efp,"\n") ;

	} /* end for */

	if ((i % USAGECOLS) != 0)
	    bprintf(efp,"\n") ;

	goto earlyret ;

/* the bad things */
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
	bprintf(efp,"%s: bad argument(s) given\n",
	    pip->progname) ;

	goto earlyret ;

/* not found */
baduser:
	ex = EX_NOUSER ;
	if (! pip->f.quiet)
	    bprintf(efp,"%s: could not get information for \"%s\" (rs %d)\n",
	        pip->progname,un,rs) ;

	goto earlyret ;

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(efp,"%s: could not open the output file (rs %d)\n",
	    pip->progname,rs) ;

	goto badret ;

/* error types of returns */
badret:
	bclose(efp) ;

	goto earlyret ;
}
/* end subroutine (main) */


