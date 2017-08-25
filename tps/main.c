/* main */

/* program to return process status */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_EPRINTF	0
#define	CF_PRINTHELP	0
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */


/* revision history:

	= 1989-03-01, David A­D­ Morano
	This subroutine was originally written.  This whole program, LOGDIR, is
	needed for use on the Sun CAD machines because Sun doesn't support
	LOGDIR or LOGNAME at this time.  There was a previous program but it is
	lost and not as good as this one anyway.  This one handles NIS+ also.
	(The previous one didn't.)

	= 1998-06-01, David A­D­ Morano
	I enhanced the program a little to print out some other user
	information besides the user's name and login home directory.

	= 1999-03-01, David A­D­ Morano
	I enhanced the program to also print out effective UID and effective
	GID.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ tps 

*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/sysctl.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>
#include	<kvm.h>
#include	<stdio.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<sbuf.h>
#include	<realname.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	PWBUFLEN
#define	PWBUFLEN	2048
#endif

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	COMBUFLEN
#define	COMBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	cfdeci(const char *,int,int *) ;

#if	CF_PRINTHELP
extern int	printhelp(bfile *,const char *,const char *,const char *) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
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
	    "ROOT",
	    "of",
	    "pg",
	    NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_root,
	argopt_of,
	argopt_pg,
	argopt_overlast

} ;

/* define the configuration keywords */
static const char *qopts[] = {
	    "sysname",
	    "release",
	    "version",
	    "machine",
	    "logname",
	    "username",
	    NULL
} ;

enum qopts {
	qopt_sysname,
	qopt_release,
	qopt_version,
	qopt_machine,
	qopt_logname,
	qopt_username,
	qopt_overlast

} ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	PROGINFO	pi, *pip = &pi ;

	int	argr, argl, aol, akl, avl ;
	int	argvalue = -1 ;
	int	maxai, pan, npa, kwi, ai, i, j, k ;
	int	rs, rs1, len, c ;
	int	sl, cl, ci ;
	int	arg_uid = -1 ;
	int	arg_sid = -1 ;
	int	arg_pid = -1 ;
	int	arg_pgid = -1 ;
	int	ex = EX_INFO ;
	int	fd_debug ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_all = FALSE ;
	int	f_every = FALSE ;
	int	f_full = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*arg_username = NULL ;
	const char	*pr = NULL ;
	const char	*ofname = NULL ;
	const char	*sp, *cp, *cp2 ;
	void		*ofp ;
	char		argpresent[MAXARGGROUPS] ;
	char		buf[BUFLEN + 1], *bp ;
	char		pwbuf[PWBUFLEN + 1] ;

#if	CF_EPRINTF
	if ((cp = getenv(VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

	pip->efp = stderr ;

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
	                    goto badargval ;

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

/* keyword match or only key letters ? */

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

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
	                                rs = SR_NOENT ;
	                                break ;
	                            }

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pr = argp ;

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

	                            if (argr <= 0) {
	                                rs = SR_NOENT ;
	                                break ;
	                            }

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                ofname = argp ;

	                        }

	                        break ;

/* process group ID */
	                    case argopt_pg:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl,&arg_pgid) ;

	                        } else {

	                            if (argr <= 0) {
	                                rs = SR_NOENT ;
	                                break ;
	                            }

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                rs = cfdeci(argp,argl,&arg_pgid) ;

	                        }

	                        break ;

/* handle all keyword defaults */
	                    default:
	                        f_usage = TRUE ;
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

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

	                        case 'a':
	                            f_all = TRUE ;
	                            break ;

	                        case 'e':
	                            f_every = TRUE ;
	                            break ;

	                        case 'f':
	                            f_full = TRUE ;
	                            break ;

/* PID */
	                        case 'p':
	                            if (argr <= 0) {
	                                rs = SR_NOENT ;
	                                break ;
	                            }

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                rs = cfdeci(argp,argl,&arg_pid) ;

	                            break ;

/* SID */
	                        case 's':
	                            if (argr <= 0) {
	                                rs = SR_NOENT ;
	                                break ;
	                            }

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                rs = cfdeci(argp,argl,&arg_sid) ;

	                            break ;

/* username or UID */
	                        case 'u':
	                            if (argr <= 0) {
	                                rs = SR_NOENT ;
	                                break ;
	                            }

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                arg_username = argp ;

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
	                                if (avl)
	                                    rs = cfdeci(avp,avl,
	                                        &pip->verboselevel) ;

	                            }

	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
	                            break ;

	                        default:
	                            f_usage = TRUE ;
	                            rs = SR_INVALID ;

	                        } /* end switch */

	                        akp += 1 ;
				if (rs < 0)
				    break ;

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
	                fprintf(pip->efp,"%s: extra arguments specified\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0)
	    fprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    fprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;

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

	    proginfo_setprogroot(pip,pr,-1) ;

	}

/* program search name */

	proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

	if (pip->debuglevel > 0) {

	    fprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    fprintf(pip->efp,"%s: sn=%s\n",
	        pip->progname,pip->searchname) ;

	}

/* help file */

	if (f_help)
	    goto help ;


	ex = EX_OK ;

/* initialize some stuff */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv(VARTMPDNAME) ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;

/* open the output file */

	rs = SR_OK ;
	if (ofname != NULL) {

	    ofp = fopen(ofname,"w") ;

	    if (ofp == NULL)
	        rs = SR_NOENT ;

	} else
	    ofp = stdout ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    fprintf(pip->efp,
	        "%s: could not open the output file (%d)\n",
	        pip->progname,rs) ;

	    goto ret3 ;
	}

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        fprintf(stderr,"main: sid=%d\n",getsid(0)) ;
#endif

/* get the processes and print them out */

	{
	    struct passwd	pw, *pp ;

	    kvm_t		*kp ;

	    struct kinfo_proc	*kpp ;

	    int	kop, karg, kcount ;


	    if (arg_username != NULL) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            fprintf(stderr,"main: username=%s\n",arg_username) ;
#endif

	        if (! isdigit(arg_username[0])) {

	            rs = uc_getpwnam(arg_username,&pw,pwbuf,PWBUFLEN) ;

	            if (rs >= 0)
	                arg_uid = pw.pw_uid ;

	        } else
	            rs = cfdeci(arg_username,-1,&arg_uid) ;

	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        fprintf(stderr,"main: arg_uid=%d\n",arg_uid) ;
#endif

	    kop = KERN_PROC_ALL ;
	    karg = 0 ;
	    if (arg_pid >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            fprintf(stderr,"main: choice 1\n") ;
#endif

	        kop = KERN_PROC_PID ;
	        karg = arg_pid ;

	    } else if (arg_uid >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            fprintf(stderr,"main: choice 2\n") ;
#endif

	        kop = KERN_PROC_UID ;
	        karg = arg_uid ;

	    } else if (! (f_all || f_every || (arg_sid >= 0))) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            fprintf(stderr,"main: choice 3\n") ;
#endif

	        arg_uid = getuid() ;

	        kop = KERN_PROC_UID ;
	        karg = arg_uid ;

	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        fprintf(stderr,"main: kop=%d karg=%d\n", kop,karg) ;
#endif

	    if (rs >= 0) {

	        if ((kp = kvm_open(NULL,NULL,NULL,O_RDONLY,NULL)) >= 0) {

	            kpp = kvm_getprocs(kp,kop,karg,&kcount) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                fprintf(stderr,"main: kvm_getprocs() kpp=%p\n",kpp) ;
#endif

	            if (kpp != NULL) {

	                int	pe_pid ;
	                int	pe_sid ;
	                int	pe_pgid ;
	                int	pe_ppid ;
	                int	pe_uid ;
	                int	pe_gid ;
			int	pe_stat ;
			int	pe_flags ;
	                int	f ;

	                char	combuf[COMBUFLEN + 1] ;
	                char	*pe_comm ;


	                for (i = 0 ; i < kcount ; i += 1) {

	                    pe_comm = kpp->kp_proc.p_comm ;
	                    pe_pid = kpp->kp_proc.p_pid ;
			    pe_stat = kpp->kp_proc.p_stat ;
			    pe_flags = kpp->kp_proc.p_flag ;

	                    pe_uid = kpp->kp_eproc.e_pcred.p_ruid ;
	                    pe_gid = kpp->kp_eproc.e_pcred.p_rgid ;
	                    pe_ppid = kpp->kp_eproc.e_ppid ;
	                    pe_pgid = kpp->kp_eproc.e_pgid ;
			    pe_sid = 0 ;
			    if ((kpp->kp_proc.p_pgrp != NULL) &&
			    (kpp->kp_proc.p_pgrp->pg_session != NULL))
			    pe_sid = kpp->kp_proc.p_pgrp->pg_session->s_sid ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(3)) {
				char	loginbuf[MAXLOGNAME + 1] ;
				strwcpy(loginbuf,
					kpp->kp_eproc.e_login,
					MAXLOGNAME) ;
	                        fprintf(stderr,"main: login=%s\n",
					loginbuf) ;
			}
#endif

#if	CF_DEBUG
	                    if (DEBUGLEVEL(3)) {
	                        fprintf(stderr,"main: tpgid=%d\n",
	                    		kpp->kp_eproc.e_tpgid) ;
	                        fprintf(stderr,"main: e_sess=%p\n",
					kpp->kp_eproc.e_sess) ;
	                        fprintf(stderr,"main: e_tsess=%p\n",
					kpp->kp_eproc.e_tsess) ;
			}
#endif

#if	CF_DEBUG
	                    if (DEBUGLEVEL(3)) {
				char	loginbuf[MAXLOGNAME + 1] ;
	                        fprintf(stderr,"main: pid=%d uid=%d sid=%d\n",
	                            pe_pid,pe_uid,pe_sid) ;
			    if (kpp->kp_eproc.e_tsess != NULL) {
				strwcpy(loginbuf,
					kpp->kp_eproc.e_tsess->s_login,
					MAXLOGNAME) ;
	                        fprintf(stderr,"main: login=%s\n",
					loginbuf) ;
				}
			}
#endif

	                    f = TRUE ;
	                    if (f && (arg_sid >= 0) && (arg_sid != pe_sid))
	                        f = FALSE ;

	                    if (f && (arg_pgid >= 0) && (arg_pgid != pe_pgid))
	                        f = FALSE ;

	                    if (f && (arg_uid >= 0) && (arg_uid != pe_uid))
	                        f = FALSE ;

	                    if (f) {

	                        sncpy1(combuf,COMBUFLEN,pe_comm) ;

	                        combuf[40] = '\0' ;

	                        fprintf(stdout,"%6d %6d %6d %s\n",
	                            pe_pid,
	                            pe_uid,
				    pe_sid,
	                            combuf) ;

	                    } /* end if (selected) */

	                    kpp += 1 ;

	                } /* end for */

	            } /* end if */

	            kvm_close(kp) ;
	        } /* end if (opened KVM) */

	    } /* end if */

	} /* end block */

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

ret3:
	fclose(ofp) ;

/* we are done */
done:
ret2:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    fprintf(stderr,"main: exiting ex=%d\n",ex) ;
#endif


/* early return thing */
retearly:
ret1:
	fclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

	return ex ;

/* the information type thing */
usage:
	fprintf(pip->efp,
	    "%s: USAGE> %s [-u username] [-s sid] [-p pid]\n",
	    pip->progname,pip->progname) ;

	fprintf(pip->efp,"%s: \t[-?V] [-Dv]\n",
	    pip->progname) ;

	goto retearly ;

/* print out some help */
help:

#if	CF_PRINTHELP
	printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif

	goto retearly ;

/* the bad things */
badargnum:
	fprintf(pip->efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badargextra:
	fprintf(pip->efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	fprintf(pip->efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

/* bad arguments come here */
badarg:
	ex = EX_USAGE ;

	goto retearly ;

}
/* end subroutine (main) */


