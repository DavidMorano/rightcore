/* main (CEXECER) */

/* program to return a user's home login directory */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 1998-06-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ /usr/add-on/local/lib/cexece/cexecer


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/systeminfo.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<bits.h>
#include	<ascii.h>
#include	<vecstr.h>
#include	<stdorder.h>
#include	<sockaddress.h>
#include	<filebuf.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"dialopts.h"
#include	"dialcprogmsg.h"
#include	"config.h"
#include	"defs.h"
#include	"mkcexsync.h"


/* local defines */

#define	TARGETINFO	struct targetinfo
#define	TARGETINFO_FL	struct targetinfo_flags

#ifndef	PORTBUFLEN
#define	PORTBUFLEN	20
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif

#ifndef	VARNODE
#define	VARNODE		"NODE"
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define	BUFLEN		(4 * VARLEN)

#ifndef	VBUFLEN
#define	VBUFLEN		(MAXNAMELEN+DIGBUFLEN)
#endif

#define	HEXBUFLEN	70

#define	TO_READ		(5 * 60)

#define	NDEBFNAME	"/tmp/cexecer.deb"


/* external subroutines */

extern int	snses(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	ctdeci(char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getnodename(char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	findfilepath(const char *,char *,const char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	nprintf(const char *,const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */

struct targetinfo_flags {
	uint		errchan:1 ;
} ;

struct targetinfo {
	const char	*nodename ;
	const char	*pwd ;
	const char	*progfname ;
	const char	**av ;
	const char	**ev ;
	SOCKADDRESS	saout, saerr ;
	TARGETINFO_FL	f ;
	int		opts ;
	ushort		flags ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procenvsys(PROGINFO *) ;
static int	process_input(PROGINFO *,int,int,
			TARGETINFO *) ;
static int	process_record(PROGINFO *,FILEBUF *,int,int,char *,
			int,TARGETINFO *) ;

#if	CF_DEBUGS
extern int	mkhexstr(char *,int,void *,int) ;
#endif


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"ef",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_ef,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;

static const char	*varnode = VARNODE ;

static const char	*envbads[] = {
	"_",
	"_A0",
	"_EF",
	"A__z",
	"SYSNAME",
	"RELEASE",
	"VERSION",
	"MACHINE",
	"ARCHITECTURE",
	"HZ",
	"NCPU",
	"HOSTNAME",
	"TERM",
	"TERMDEV",
	"AUDIODEV",
	"RANDOM",
	NULL
} ;

static const char	*envsys[] = {
	"SYSNAME",
	"RELEASE",
	"VERSION",
	"MACHINE",
	"ARCHITECTURE",
	"HZ",
	"NCPU",
	NULL
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	TARGETINFO	ti, *tip = &ti ;
	BITS		pargs ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	rs, rs1 ;
	int	i, j ;
	int	cl ;
	int	v ;
	int	pslen, sslen ;
	int	fd_input = FD_STDIN ;
	int	fd_output = FD_STDOUT ;
	int	fd_error = FD_STDERR ;
	int	fd_a1, fd_a2 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*efname = NULL ;
	const char	*progfname = NULL ;
	const char	*xpath = NULL ;
	const char	*varpath = VARPATH ;
	const char	*varpwd = VARPWD ;
	const char	*sp, *cp ;
	char		buf[BUFLEN + 1] ;
	char		nodename[NODENAMELEN + 1] ;
	char		progfbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
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

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
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

	            argval = (argp + 1) ;

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

/* program-root */
	                case argopt_root:
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
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sn = argp ;
	                    }
	                    break ;

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            efname = argp ;
	                    }
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    f_usage = TRUE ;
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

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
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
	                        f_usage = TRUE ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits or options) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(1))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

/* try to get our program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

	if (f_usage)
	    usage(pip) ;

/* help */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* initialize more stuff */

	rs = vecstr_start(&pip->args,10,VECSTR_PORDERED) ;

	if (rs >= 0) {
	    rs = vecstr_start(&pip->envs,10,VECSTR_PSORTED) ;
	    if (rs < 0) vecstr_finish(&pip->args) ;
	}

/* get our node name */

	if (rs >= 0)
	    rs = getnodename(nodename,NODENAMELEN) ;

	if (rs < 0) {
	    ex = EX_TEMPFAIL ;
	    goto badinit ;
	}

/* prepare the primary SYNC sequence */

	pslen = mkcexsync(buf,MKCEXSYNC_MKLEN) ;

/* prepare the secondary SYNC sequence */

	i = pslen ;
	cl = strlen(nodename) ;

	stdorder_wshort((buf + i),cl) ;

	i += 2 ;
	strncpy((buf + i),nodename,cl) ;

	i += cl ;
	for (j = 0 ; j < MKCEXSYNC_FINLEN ; j += 1) {
	    buf[i++] = CH_SYNC ;
	}
	sslen = i ;

/* send the SYNC sequence */

	rs = uc_writen(fd_output,buf,sslen) ;
	if (rs < 0) {
	    ex = EX_IOERR ;
	    goto badio ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: SYNC uc_writen() rs=%d\n", rs) ;
#endif

/* read the data */

	memset(&ti,0,sizeof(TARGETINFO)) ;

	rs = process_input(pip,fd_input,TO_READ,tip) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: process_input() rs=%d\n", rs) ;
#endif

/* put our the nodename given us into the target environment */

#ifdef	COMMENT /* this is the *wrong* (source) nodename; we needed dst */
	if (rs >= 0) {
	    const char	*nn = tip->nodename ;
	    if ((nn != NULL) && (nn[0] != '\0'))
	        rs = vecstr_envadd(&pip->envs,varnode,nn,-1) ;
	}
#endif /* COMMENT */

	if (rs >= 0)
	    rs = procenvsys(pip) ;

/* if the error channel is different than the output channel, pop it */

	if ((rs >= 0) && (tip->flags & DIALCPROGMSG_FERRCHAN)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: different FDs\n") ;
#endif

	    rs = uc_writen(fd_error,buf,pslen) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: uc_writen() rs=%d\n", rs) ;
#endif

	} /* end if (standard error diffferent than standard output) */

/* check the passed environment for some minimal things */

	if (rs >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: ENV check path \n") ;
#endif

	    rs1 = vecstr_search(&pip->envs,varpath,vstrkeycmp,&sp) ;
	    if (rs1 == SR_NOTFOUND) {
	        sp = DEFPATH ;
	        rs = vecstr_envadd(&pip->envs,varpath,sp,-1) ;
	    }

	    xpath = (sp + 5) ;

	} /* end if (checking environment) */

/* does the directory given us exist? */

	if ((rs >= 0) && (tip->pwd != NULL) && (tip->pwd[0] != '\0')) {
	    rs = perm(tip->pwd,-1,-1,NULL,X_OK) ;
	    if (rs == SR_NOENT) rs = SR_NOTDIR ;
	}

	if ((rs >= 0) && (tip->pwd != NULL) && (tip->pwd[0] != '\0')) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: ENV check PWD \n") ;
#endif

	    rs1 = vecstr_search(&pip->envs,varpwd,vstrkeycmp,&sp) ;
	    if (rs1 < 0)
	        rs = vecstr_envadd(&pip->envs,varpwd,tip->pwd,-1) ;

	} /* end if */

/* change into that directory */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: ENV chdir()\n") ;
#endif

	if ((rs >= 0) && (tip->pwd != NULL) && (tip->pwd[0] != '\0')) {
	    rs = u_chdir(tip->pwd) ;
	    if (rs == SR_NOENT) rs = SR_NOTDIR ;
	}

/* does the program file exist (and is it at least executable)? */

	if ((rs >= 0) && 
	    ((tip->progfname == NULL) || (tip->progfname[0] == '\0')))
	    rs = SR_NOENT ;

	if (rs >= 0) {

	    progfname = tip->progfname ;
	    if (tip->progfname[0] != '/') {

	        if (xpath == NULL) {
	            rs1 = vecstr_search(&pip->envs,varpath, vstrkeycmp,&cp) ;
	            if (rs1 >= 0) xpath = (cp + 5) ;
	        } /* end if */

	        rs = findfilepath(xpath,progfbuf,tip->progfname,X_OK) ;
	        if (rs == 0) {
	            progfname = progfbuf ;
	            rs = mkpath2(progfbuf,tip->pwd,tip->progfname) ;
	            if (rs >= 0)
	                rs = perm(progfname,-1,-1,NULL,X_OK) ;
	        } else if (rs > 0)
	            progfname = progfbuf ;

	    } else
	        rs = perm(progfname,-1,-1,NULL,X_OK) ;

/* add the special environment variable '_' */

	    if (rs >= 0) {
	        rs1 = vecstr_search(&pip->envs,"_", vstrkeycmp,&cp) ;
	        if (rs1 >= 0) vecstr_del(&pip->envs,rs1) ;
	        rs = vecstr_envadd(&pip->envs,"_",progfname,-1) ;
	    }

	} /* end if (program file) */

/* write the next answer */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: sending back rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    stdorder_wint(buf,rs) ;
	    rs1 = uc_writen(fd_output,buf,4) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: ACK/NAK uc_writen() rs=%d\n", rs1) ;
#endif

	if ((rs >= 0) && (rs1 < 0)) {
	    rs = rs1 ;
	    goto badio ;
	}

	if (rs < 0) {
	    ex = EX_PROTOCOL ;
	    goto badproto ;
	}

/* do some stuff if we were not instructed to go into light-wight mode */

	if ((rs >= 0) && (! (tip->opts & DIALOPTS_MNOLIGHT))) {
	    struct sockaddr_in	*sap ;
	    const int	af = AF_INET ;
	    int		port ;
	    ushort	usw ;
	    char	portbuf[PORTBUFLEN + 1] ;

	    sap = (struct sockaddr_in *) &tip->saout ;
	    usw = ntohs(sap->sin_port) ;

	    port = usw ;
	    ctdeci(portbuf,18,port) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: dialtcp() node=%s port=%s\n",
	            tip->nodename,portbuf) ;
#endif

	    rs = dialtcp(tip->nodename,portbuf,af,20,0) ;
	    fd_a1 = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: dialtcp() rs=%d\n",rs) ;
#endif

	    sap = (struct sockaddr_in *) &tip->saerr ;
	    usw = ntohs(sap->sin_port) ;

	    port = usw ;
	    if ((rs >= 0) && (port > 0)) {

	        tip->f.errchan = TRUE ;
	        ctdeci(portbuf,18,port) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: dialtcp() node=%s port=%s\n",
	                tip->nodename,portbuf) ;
#endif

	        rs = dialtcp(tip->nodename,portbuf,af,20,0) ;
	        fd_a2 = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: dialtcp() rs=%d\n",rs) ;
#endif

	    } /* end if */

	} /* end if (light-weight) */

/* continue with heavy-weight work for light-weight mode! :-) */

	bflush(pip->efp) ;

	if ((rs >= 0) && (! (tip->opts & DIALOPTS_MNOLIGHT))) {

	    if ((rs = uc_fork()) == 0) {

	        u_close(fd_input) ;

	        u_dup(fd_a1) ;

	        u_close(fd_output) ;

	        u_dup(fd_a1) ;

	        u_close(fd_error) ;

	        if (tip->f.errchan) {
	            u_dup(fd_a2) ;
	        } else
	            u_dup(fd_a1) ;

	        u_close(fd_a1) ;

	        u_close(fd_a2) ;

	    } else if (rs > 0)
	        goto done ;

	} /* end if (not light-weight mode) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: so far 1 rs=%d\n",rs) ;
#endif

/* sort the environment variables */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: vecstr_i=%d\n",pip->envs.i) ;
	    debugprintf("main: vecstr_n=%d\n",pip->envs.n) ;
	    debugprintf("main: vecstr_c=%d\n",pip->envs.c) ;
	    for (i = 0 ; vecstr_get(&pip->envs,i,&cp) >= 0 ; i += 1)
	        debugprintf("main: env[%d]=>%s< (%p)\n",i,cp,cp) ;
	}
	debugprintf("main: env[OV]=%08x\n",pip->envs.va[i]) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: so far 2 rs=%d\n",rs) ;
#endif

	if (rs >= 0)
	    rs = vecstr_sort(&pip->envs,vstrkeycmp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: so far 3 rs=%d\n",rs) ;
#endif

/* do the 'exec(2)' */

	if (rs >= 0) {
	    const char	**cpp ;
	    const char	**av ;
	    const char	**ev ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        rs = u_getgroups(0,NULL) ;
	        debugprintf("main: u_getgroups() rs=%d\n",rs) ;
	        debugprintf("main: u_execve()\n") ;
	    }
#endif /* CF_DEBUG */

	    if (rs >= 0) {
	        rs = vecstr_getvec(&pip->args,&cpp) ;
	        av = (const char **) cpp ;
	    }

	    if (rs >= 0) {
	        rs = vecstr_getvec(&pip->envs,&cpp) ;
	        ev = (const char **) cpp ;
	    }

	    if (rs >= 0)
	        rs = u_execve(progfname,av,ev) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: u_execve() rs=%d\n",rs) ;
#endif

	} /* end if */

/* we are done */
done:
	ex = (rs >= 0) ? EX_OK : EX_NOEXEC ;

ret3:
badio:
	vecstr_finish(&pip->args) ;

	vecstr_finish(&pip->envs) ;

/* early return thing */
badproto:
badinit:
retearly:
ret2:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
PROGINFO	*pip ;
{
	int	rs ;
	int	wlen = 0 ;

	const char	*pn = pip->progname ;
	const char	*fmt ;


	fmt = "%s: USAGE> %s\n",
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procenvsys(pip)
PROGINFO	*pip ;
{
	struct utsname	un ;

	int	rs ;
	int	rs1 ;
	int	i ;
	int	n = 0 ;

	const char	**envs = envsys ;
	const char	*tp ;
	const char	*ep ;

	char	vbuf[VBUFLEN+1] = { 0 } ;


	if ((rs = u_uname(&un)) >= 0) {

	    if ((tp = strchr(un.nodename,'.')) != NULL)
	        un.nodename[tp-un.nodename] = '\0' ;

	    for (i = 0 ; (rs >= 0) && (envs[i] != NULL) ; i += 1) {
	        int	sc = (envs[i][0] & 0xff) ;

	        tp = NULL ;
	        switch (sc) {

	        case 'S':
	            tp = un.sysname ;
	            break ;

	        case 'V':
	            tp = un.version ;
	            break ;

	        case 'R':
	            tp = un.release ;
	            break ;

	        case 'M':
	            tp = un.machine ;
	            break ;

	        case 'A':
#ifdef	SI_ARCHITECTURE
	            rs1 = u_sysinfo(SI_ARCHITECTURE,vbuf,VBUFLEN) ;
	            if (rs1 >= 0) tp = vbuf ;
#endif /* SI_ARCHITECTURE */
	            break ;

	        case 'H':
#ifdef	_SC_CLK_TCK
	            {
	                long	v ;
	                rs1 = uc_sysconf(_SC_CLK_TCK,&v) ;
	                if (rs1 >= 0) {
	                    rs1 = ctdecl(vbuf,VBUFLEN,v) ;
	                    if (rs1 >= 0) tp = vbuf ;
	                }
	            }
#endif /* _SC_CLK_TCK */
	            break ;

	        case 'N':
#ifdef	_SC_NPROCESSORS_ONLN
	            {
	                long	v ;
	                rs1 = uc_sysconf(_SC_NPROCESSORS_ONLN,&v) ;
	                if (rs1 >= 0) {
	                    rs1 = ctdecl(vbuf,VBUFLEN,v) ;
	                    if (rs1 >= 0) tp = vbuf ;
	                }
	            }
#endif /* _SC_NPROCESSORS_ONLN */
	            break ;

	        } /* end switch */

	        if ((rs >= 0) && (tp != NULL)) {
	            n += 1 ;
	            rs = vecstr_envadd(&pip->envs,envs[i],tp,-1) ;
	        } /* end if */

	    } /* end for */

	} /* end if (uname) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (procenvsys) */


static int process_input(pip,fd,to,tip)
PROGINFO		*pip ;
int			fd ;
int			to ;
TARGETINFO	*tip ;
{
	struct dialcprogmsg_end		m0 ;
	struct dialcprogmsg_light	m5 ;

	FILEBUF		rd ;

	const int	salen = sizeof(SOCKADDRESS) ;
	const int	fbo = FILEBUF_ONET ;

	int	rs ;
	int	rs1 ;
	int	type ;
	int	cl, mlen ;
	int	f_exit = FALSE ;

	ushort	usw ;

	char	buf[BUFLEN + 1] ;


	memset(tip,0,sizeof(TARGETINFO)) ;

	if ((rs = filebuf_start(&rd,fd,0L,BUFLEN,fbo)) >= 0) {
	    f_exit = FALSE ;

	    while ((! f_exit) && ((rs = filebuf_read(&rd,buf,1,to)) > 0)) {

	        type = buf[0] & 0xff ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process_input: filebuf_read() rs=%d type=%d\n",
	                rs,type) ;
#endif

	        switch (type) {

	        case dialcprogmsgtype_end:
	            mlen = DIALCPROGMSG_END ;
	            rs = filebuf_read(&rd,(buf + 1),(mlen - 1),to) ;
	            if (rs >= 0) {
	                rs = dialcprogmsg_end(buf,mlen,1,&m0) ;
	                f_exit = (rs >= 0) ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    debugprintf("process_input: rs=%d mlen=%d\n",
	                        rs,mlen) ;
	                }
#endif
	                tip->flags = m0.flags ;
	                tip->opts = m0.opts ;
	            }
	            break ;

	        case dialcprogmsgtype_light:
	            rs = filebuf_read(&rd,(buf + 1),2,to) ;
	            if (rs >= 0) {
	                stdorder_rushort((buf + 1),&usw) ;
	                rs = filebuf_read(&rd,(buf + 3),(int) usw,to) ;
	                mlen = (3 + rs) ;
	                if (rs >= 0) {
#if	CF_DEBUGS
	                    {
	                        char	hexbuf[100 + 1] ;
	                        mkhexstr(hexbuf,100,buf,12) ;
	                        debugprintf("process_input: m5> %s\n",hexbuf) ;
	                    }
#endif
	                    rs = dialcprogmsg_light(buf,mlen,1,&m5) ;
	                    if (rs >= 0) {
	                        memcpy(&tip->saout,&m5.saout,salen) ;
	                        memcpy(&tip->saerr,&m5.saerr,salen) ;
	                    } /* end if */
	                }
	            }
	            break ;

	        case dialcprogmsgtype_nodename:
	        case dialcprogmsgtype_pwd:
	        case dialcprogmsgtype_fname:
	        case dialcprogmsgtype_arg:
	        case dialcprogmsgtype_env:
	            rs = process_record(pip,&rd,to,type,buf,BUFLEN,tip) ;
	            break ;

	        default:
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("process_input: unknown type\n") ;
#endif
	            rs = filebuf_read(&rd,(buf + 1),2,to) ;
	            cl = rs ;
	            if (rs >= 0) {
	                if (cl < 2) rs = SR_INVALID ;
	                stdorder_rushort((buf + 1),&usw) ;
	                rs = filebuf_read(&rd,(buf + 3),(int) usw,to) ;
	            }
	            break ;

	        } /* end switch */

	        if (rs < 0) break ;
	    } /* end while */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("process_input: after-loop rs=%d f_exit=%d\n",
	            rs,f_exit) ;
#endif

	    rs1 = filebuf_finish(&rd) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */

	if (rs >= 0)
	    rs = (f_exit) ? SR_OK : SR_PROTO ;

	return rs ;
}
/* end subroutine (process_input) */


static int process_record(pip,fbp,to,type,rbuf,rlen,tip)
PROGINFO		*pip ;
FILEBUF			*fbp ;
int			to ;
int			type ;
char			rbuf[] ;
int			rlen ;
TARGETINFO	*tip ;
{
	int	rs ;
	int	len ;
	int	envlen ;

	ushort	usw ;

	char	*lenbuf = (rbuf + 1) ;
	char	*envbuf = (rbuf + 3) ;

	if ((rs = filebuf_read(fbp,lenbuf,2,to)) >= 0) {
	    stdorder_rushort(lenbuf,&usw) ;
	    len = usw & USHORT_MAX ;
	    if (len <= (rlen-3)) {
	        if ((rs = filebuf_read(fbp,envbuf,len,to)) >= 0) {
	            const char	**vpp = NULL ;
	            envlen = (len-1) ;
	            switch (type) {
	            case dialcprogmsgtype_nodename:
	                vpp = &tip->nodename ;
	                break ;
	            case dialcprogmsgtype_pwd:
	                vpp = &tip->pwd ;
	                break ;
	            case dialcprogmsgtype_fname:
	                vpp = &tip->progfname ;
	                break ;
	            case dialcprogmsgtype_arg:
	                rs = vecstr_add(&pip->args,envbuf,(len - 1)) ;
	                break ;
	            case dialcprogmsgtype_env:
	                if (matkeystr(envbads,envbuf,envlen) < 0) {
	                    rs = vecstr_add(&pip->envs,envbuf,envlen) ;
	                }
	                break ;
		    default:
	    		rs = SR_PROTONOSUPPORT ;
	    		break ;
		    } /* end switch */
		    if ((rs >= 0) && (vpp != NULL)) {
	    	        rs = uc_mallocstrw(envbuf,envlen,vpp) ;
		    }
		} /* end if (read) */
	    } else
	        rs = SR_PROTO ;
	} /* end if (read) */

	return rs ;
}
/* end subroutine (process_record) */



