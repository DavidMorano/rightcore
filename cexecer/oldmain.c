/* main (CEXECER) */

/* program to return a user's home login directory */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-06-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ /usr/add-on/local/lib/cexece/cexecer


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<ascii.h>
#include	<vecstr.h>
#include	<stdorder.h>
#include	<sockaddress.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"filebuf.h"
#include	"dialopts.h"
#include	"dialcprogmsg.h"
#include	"config.h"
#include	"defs.h"
#include	"mkcexsync.h"



/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	PORTBUFLEN
#define	PORTBUFLEN	20
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif

#define	BUFLEN		(4 * VARLEN)
#define	HEXBUFLEN	70
#define	TO_READ		(5 * 60)

#define	DEBUGFNAME	"/tmp/cexecer.deb"


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;

extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */

struct targetinfo_flags {
	uint	errchan : 1 ;
} ;

struct targetinfo {
	char	*nodename ;
	char	*pwd ;
	char	*progfname ;
	char	**av ;
	char	**ev ;
	SOCKADDRESS	saout, saerr ;
	struct targetinfo_flags	f ;
	int	opts ;
	ushort	flags ;
} ;


/* forward references */

static int	usage(struct proginfo *) ;
static int	process_input(struct proginfo *,int,int,
			struct targetinfo *) ;
static int	process_record(struct proginfo *,FILEBUF *,int,int,char *,
			struct targetinfo *) ;

#if	CF_DEBUGS
extern int	mkhexstr(char *,int,void *,int) ;
#endif


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"of",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
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

static const char	*badenvs[] = {
	"_",
	"_A0",
	"A__z",
	"SYSNAME",
	"RELEASE",
	"MACHINE",
	"NODE",
	"ARCHITECTURE",
	"HZ",
	"HOSTNAME",
	"TERM",
	"TERMDEV",
	"AUDIODEV",
	"DISPLAY",
	"RANDOM",
	NULL
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct group	*gep = NULL ;

	struct proginfo	pi, *pip = &pi ;

	struct targetinfo	ti, *tip = &ti ;

	bfile		errfile ;

	time_t	daytime = 0 ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan, npa ;
	int	rs, rs1 ;
	int	len ;
	int	i, j, k ;
	int	sl, cl ;
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
	int	f ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	nodename[NODENAMELEN + 1] ;
	char	progfbuf[MAXPATHLEN + 1] ;
	char	*pr = NULL ;
	char	*errfname = NULL ;
	char	*progfname = NULL ;
	char	*xpath = NULL ;
	char	*varpath = VARPATH ;
	char	*varpwd = VARPWD ;
	char	*sp, *cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;

	cp = getenv(STDERRVAR) ;

	errfname = BFILE_STDERR ;
	if ((cp != NULL) && (cfdeci(cp,-1,&fd_error) >= 0))
	    errfname = (char *) fd_error ;

	if (bopen(&errfile,errfname,"dwca",0666) >= 0) {

	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;

	    if (errfname != BFILE_STDERR)
	        u_close(fd_error) ;

	}

	fd_error = FD_STDERR ;

/* initialize */

	pip->verboselevel = 1 ;

/* start parsing the arguments */

	rs = SR_OK ;
	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1) 
		argpresent[ai] = 0 ;

	npa = 0 ;			/* number of positional so far */
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
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                        }

	                        break ;

				case argopt_help:
					f_help = TRUE ;
					break ;

/* handle all keyword defaults */
	                    default:
	                        rs = SR_INVALID ;
	                        f_usage = TRUE ;
	                        bprintf(pip->efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {

				int	kc = (*akp & 0xff) ;
	                        switch (kc) {

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
	                            bprintf(pip->efp,
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                        } /* end switch */

	                        akp += 1 ;
				if (rs < 0)
				    break ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	        } /* end if (digits or options) */

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
	if (DEBUGLEVEL(1))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    usage(pip) ;

/* try to get our program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

/* program search name */

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

/* help */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* initialize more stuff */

	rs = vecstr_start(&pip->args,10,VECSTR_PORDERED) ;
	if (rs < 0)
	    goto badinit ;

	rs = vecstr_start(&pip->envs,10,VECSTR_PSORTED) ;
	if (rs < 0) {

	    vecstr_finish(&pip->args) ;

	    goto badinit ;
	}

/* get our node name */

	getnodedomain(nodename,NULL) ;

/* prepare the primary SYNC sequence */

	i = 0 ;
	for (j = 0 ; j < SYNCALTLEN ; j += 1)
	    buf[i++] = j & 1 ;

	for (j = 0 ; j < SYNCFINLEN ; j += 1)
	    buf[i++] = CH_SYNC ;

	pslen = i ;

/* prepare the secondary SYNC sequence */

	cl = strlen(nodename) ;

	stdorder_wshort((buf + i),cl) ;

	i += 2 ;
	strncpy(buf + i,nodename,cl) ;

	i += cl ;
	for (j = 0 ; j < SYNCFINLEN ; j += 1)
	    buf[i++] = CH_SYNC ;

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

	memset(&ti,0,sizeof(struct targetinfo)) ;

	rs = process_input(pip,fd_input,TO_READ,tip) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: process_input() rs=%d\n", rs) ;
#endif

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
	    if (rs1 < 0) {
	        sp = DEFPATH ;
	        rs = vecstr_envadd(&pip->envs,varpath,sp,-1) ;
	    }

	    xpath = (sp + 5) ;

	} /* end if (checking environment) */

/* does the directory given us exist? */

	if ((rs >= 0) && (tip->pwd != NULL) && (tip->pwd[0] != '\0')) {

	    rs = perm(tip->pwd,-1,-1,NULL,X_OK) ;

	    if (rs == SR_NOENT)
		rs = SR_NOTDIR ;

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

	    if (rs == SR_NOENT)
		rs = SR_NOTDIR ;

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

		    if (rs1 >= 0)
	                xpath = (cp + 5) ;

	        } /* end if */

	        rs = findfilepath(xpath,tip->progfname,X_OK,progfbuf) ;

	        if (rs == 0) {

	            progfname = progfbuf ;
	            mkpath2(progfbuf,tip->pwd,tip->progfname) ;

	            rs = perm(progfname,-1,-1,NULL,X_OK) ;

	        } else if (rs > 0)
	            progfname = progfbuf ;

	    } else
	        rs = perm(progfname,-1,-1,NULL,X_OK) ;

/* add the special environment variable '_' */

	    rs1 = vecstr_search(&pip->envs,"_", vstrkeycmp,&cp) ;

	    if (rs1 >= 0)
	        vecstr_del(&pip->envs,rs1) ;

	    vecstr_envadd(&pip->envs,"_",progfname,-1) ;

	} /* end if (program file) */

/* write the next answer */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: sending back rs=%d\n",rs) ;
#endif

	stdorder_wint(buf,rs) ;

	rs1 = uc_writen(fd_output,buf,4) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: ACK/NAK uc_writen() rs=%d\n", rs1) ;
#endif

	if ((rs >= 0) && (rs1 < 0)) {
	    rs = rs1 ;
	    goto badio ;
	}

	if (rs < 0)
	    goto badproto ;

/* do some stuff if we were instructed to go into light-wight mode */

	if ((rs >= 0) && (! (tip->opts & DIALOPTS_MNOLIGHT))) {

	    struct sockaddr_in	*sap ;

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

	    rs = dialtcp(tip->nodename,portbuf,AF_INET,20,0) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: dialtcp() rs=%d\n",rs) ;
#endif

	    fd_a1 = rs ;
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

	        rs = dialtcp(tip->nodename,portbuf,AF_INET,20,0) ;
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

	    rs = u_fork() ;
	    if (rs < 0)
	        goto badfork ;

	    if (rs > 0)
	        goto done ;

	    u_close(fd_input) ;

	    u_dup(fd_a1) ;

	    u_close(fd_output) ;

	    u_dup(fd_a1) ;

	    u_close(fd_error) ;

	    if (tip->f.errchan)
	        u_dup(fd_a2) ;

	    else
	        u_dup(fd_a1) ;

	    u_close(fd_a1) ;

	    u_close(fd_a2) ;

	} /* end if */

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
	    const char	**eav = (const char **) pip->args.va ;
	    const char	**eev = (const char **) pip->envs.va ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        rs = u_getgroups(0,NULL) ;
	        debugprintf("main: u_getgroups() rs=%d\n",rs) ;
	        debugprintf("main: u_execve()\n") ;
	    }
#endif /* CF_DEBUG */

	    rs = u_execve(progfname,eav,eev) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: u_execve() rs=%d\n",rs) ;
#endif

	} /* end if */

	ex = (rs >= 0) ? EX_OK : EX_NOEXEC ;

/* we are done */
done:
ret2:
	vecstr_finish(&pip->args) ;

	vecstr_finish(&pip->envs) ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

/* early return thing */
retearly:
ret1:
	bclose(pip->efp) ;

ret0:

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

badinit:
	ex = EX_TEMPFAIL ;
	goto retearly ;

badio:
	ex = EX_IOERR ;
	goto retearly ;

badproto:
	ex = EX_PROTOCOL ;
	goto retearly ;

badfork:
	ex = EX_TEMPFAIL ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s \n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,"%s: \t[-D] [-V]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int process_input(pip,fd,to,tip)
struct proginfo		*pip ;
int			fd ;
int			to ;
struct targetinfo	*tip ;
{
	struct dialcprogmsg_end		m0 ;

	struct dialcprogmsg_light	m5 ;

	FILEBUF	rd ;

	int	rs ;
	int	type ;
	int	cl, mlen ;
	int	fbo ;
	int	f_exit ;

	ushort	usw ;

	char	buf[BUFLEN + 1] ;


	memset(tip,0,sizeof(struct targetinfo)) ;

	fbo = FILEBUF_ONET ;
	rs = filebuf_start(&rd,fd,0L,BUFLEN,fbo) ;
	if (rs < 0)
	    goto ret0 ;

	f_exit = FALSE ;
	while ((! f_exit) && (rs >= 0) && 
	    ((rs = filebuf_read(&rd,buf,1,to)) > 0)) {

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

	        if (rs < 0)
	            break ;

	        rs = dialcprogmsg_end(buf,mlen,1,&m0) ;
	        f_exit = (rs >= 0) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process_input: type=exit rs=%d mlen=%d\n",
			rs,mlen) ;
#endif

	        tip->flags = m0.flags ;
	        tip->opts = m0.opts ;
	        break ;

	    case dialcprogmsgtype_light:
	        rs = filebuf_read(&rd,(buf + 1),2,to) ;

	        if (rs < 0)
	            break ;

	        stdorder_rushort((buf + 1),&usw) ;

	        rs = filebuf_read(&rd,(buf + 3),(int) usw,to) ;
	        mlen = (3 + rs) ;
	        if (rs < 0)
	            break ;

#if	CF_DEBUGS
	        {
	            char	hexbuf[100 + 1] ;

	            mkhexstr(hexbuf,100,buf,12) ;
	            debugprintf("process_input: m5> %s\n",hexbuf) ;
	        }
#endif

	        rs = dialcprogmsg_light(buf,mlen,1,&m5) ;

	        if (rs >= 0) {

	            memcpy(&tip->saout,&m5.saout,sizeof(SOCKADDRESS)) ;

	            memcpy(&tip->saerr,&m5.saerr,sizeof(SOCKADDRESS)) ;

	        }

	        break ;

	    case dialcprogmsgtype_nodename:
	    case dialcprogmsgtype_pwd:
	    case dialcprogmsgtype_fname:
	    case dialcprogmsgtype_arg:
	    case dialcprogmsgtype_env:
	        rs = process_record(pip,&rd,to,type,buf,tip) ;

	        break ;

	    default:

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process_record: unknown type\n") ;
#endif

	        rs = filebuf_read(&rd,(buf + 1),2,to) ;
	        cl = rs ;
	        if (rs < 0)
	            break ;

	        if (cl < 2) {
	            rs = SR_INVALID ;
	            break ;
	        }

	        stdorder_rushort((buf + 1),&usw) ;

	        rs = filebuf_read(&rd,(buf + 3),(int) usw,to) ;

	    } /* end switch */

	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process_record: after-loop rs=%d f_exit=%d\n",
		rs,f_exit) ;
#endif

	filebuf_finish(&rd) ;

	if (rs >= 0)
	    rs = (f_exit) ? SR_OK : SR_PROTO ;

ret0:
	return rs ;
}
/* end subroutine (process_input) */


static int process_record(pip,fbp,to,type,buf,tip)
struct proginfo		*pip ;
FILEBUF			*fbp ;
int			to ;
int			type ;
char			buf[] ;
struct targetinfo	*tip ;
{
	int	rs ;
	int	len ;

	ushort	usw ;

	char	*lenbuf = (buf + 1) ;
	char	*envbuf = (buf + 3) ;


	rs = filebuf_read(fbp,lenbuf,2,to) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process_record: filebuf_read() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	stdorder_rushort(lenbuf,&usw) ;

	len = usw ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process_record: len=%d\n",len) ;
#endif

	rs = filebuf_read(fbp,envbuf,len,to) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process_record: filebuf_read() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	switch (type) {

	case dialcprogmsgtype_nodename:
	    rs = uc_mallocstrw(envbuf,(len - 1),&tip->nodename) ;

	    break ;

	case dialcprogmsgtype_pwd:
	    rs = uc_mallocstrw(envbuf,(len - 1),&tip->pwd) ;

	    break ;

	case dialcprogmsgtype_fname:
	    rs = uc_mallocstrw(envbuf,(len - 1),&tip->progfname) ;

	    break ;

	case dialcprogmsgtype_arg:
	    rs = vecstr_add(&pip->args,envbuf,(len - 1)) ;

	    break ;

	case dialcprogmsgtype_env:
	    if (matkeystr(badenvs,envbuf,(len - 1)) < 0)
	        rs = vecstr_add(&pip->envs,envbuf,(len - 1)) ;

	    break ;

	default:
	    rs = SR_PROTONOSUPPORT ;

	} /* end switch */

ret0:
	return rs ;
}
/* end subroutine (process_record) */



