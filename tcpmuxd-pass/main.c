/* main */

/* non-generic (?) front-end */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-04-10, David A­D­ Morano

	I really have no idea where this file has been before now.  It is now
	being modified for use for the PASSFD program.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ tcpmuxd-pass <passfile>


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/conf.h>
#include	<stropts.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<sockaddress.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"muximsg.h"


/* local defines */

#define	TO_RECVMSG	3

#define	NIOVECS		1

#ifndef	IPCBUFLEN
#define	IPCBUFLEN	MAXPATHLEN
#endif

#ifndef	CMSGBUFLEN
#define	CMSGBUFLEN	(sizeof(struct cmsghdr) + (2 * MAXPATHLEN))
#endif

#define	DEBUGFILE	"/tmp/passfd.deb"


/* external subroutines */

extern int	snddd(char *,int,uint,uint) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecmfui(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	dupup(int,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* global variables */


/* local structures */

union conmsg {
	struct cmsghdr	cm ;
	char		cmbuf[CMSGBUFLEN + 1] ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procpass(PROGINFO *,const char *,int) ;

static int	cmsg_passfd(struct cmsghdr *,int) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_af,
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


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	struct ustat	sb ;
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	bfile		errfile, *efp = &errfile ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		pan = 0 ;
	int		rs, rs1 ;
	int		v ;
	int		fd_input = FD_STDIN ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*pfname = NULL ;
	cchar		*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

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
	    argr -= 1 ;
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

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }

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

/* argument file */
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

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    f_usage = TRUE ;

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
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
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

	        } /* end if (digits as argument or not) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

	if (f_usage)
	    usage(pip) ;

/* get the program root */

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

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* OK, do some initialization */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* check some arguments */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    ARGINFO	*aip = &ainfo ;
	    BITS	*bop = &pargs ;
	    for (ai = 1 ; ai < argc ; ai += 1) {
	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	        if (f) {
	            cp = argv[ai] ;
		    if (cp[0] != '\0') {
			pfname = cp ;
	                pan += 1 ;
	                break ;
		    }
	        }
	    } /* end for */
	} /* end if (ok) */

	if (pfname == NULL)
	    pfname = REQFNAME ;

/* check the input */

	if (argval != NULL) {
	    if ((rs = cfdeci(argval,-1,&v)) >= 0) {
	        fd_input = v ;
	    }
	}

	if (rs >= 0) {
	    rs = u_fstat(fd_input,&sb) ;
	}

	if (rs < 0) {
	    ex = EX_NOINPUT ;
	    bprintf(efp,"%s: input file was not open (FD=%d)\n",
	        pip->progname,fd_input) ;
	    goto badnoinput ;
	}

#ifdef	COMMENT
	rs = u_stat(pfname,&sb) ;

	if ((rs < 0) || (! S_ISSOCK(sb.st_mode)))
	    goto badnopass ;
#endif /* COMMENT */

	rs = procpass(pip,pfname,fd_input) ;
	if (rs < 0)
	    rs = uc_writen(fd_input,"-\r\n",3) ;

	u_close(fd_input) ;
	fd_input = -1 ;

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	    bprintf(efp,"%s: could not complete pass operation (%d)\n",
	        pip->progname,rs) ;
	}

badnopass:
badnoinput:
ret3:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

/* early return thing */
retearly:
	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s <passfile>\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=n]] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procpass(PROGINFO *pip,cchar pfname[],int fd_input)
{
	struct muximsg_response	m0 ;
	struct muximsg_passfd	m2 ;
	struct msghdr	ipcmsg ;
	struct iovec	vecs[NIOVECS] ;
	union conmsg	conbuf ;	/* aligned for bad architectures */
	struct cmsghdr	*cmp ;
	SOCKADDRESS	toaddr ;
	mode_t		om ;
	const int	fdlen = sizeof(int) ;
	const int	to = TO_RECVMSG ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		of ;
	int		fd_pass ;
	int		s ;
	int		len ;
	int		tolen ;
	int		blen ;
	int		size ;
	int		cmsize ;
	int		conbufsize = 0 ;
	char		msgfname[MAXPATHLEN + 1] ;
	char		template[MAXPATHLEN + 1], *ipcbuf = template ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: pfname=%s\n",pfname) ;
#endif

	if (pfname == NULL) return SR_FAULT ;

	if (pfname[0] == '\0') return SR_INVALID ;

/* open the filename to pass to  */

	of = O_RDWR ;
	om = 0666 ;
	rs = uc_opene(pfname,of,om,to) ;
	fd_pass = rs ;
	if (rs < 0)
	    goto ret0 ;

/* create a local end-point */

	mkpath2(template,pip->tmpdname,"ipcXXXXXXXXXXX") ;

	om = (0600 | S_IFSOCK) ;
	rs = opentmpfile(template,O_RDWR,om,msgfname) ;
	s = rs ;
	if (rs < 0)
	    goto ret1 ;

/* continue */

	size = NIOVECS * sizeof(struct iovec) ;
	memset(&vecs,0,size) ;

	vecs[0].iov_base = ipcbuf ;
	vecs[0].iov_len = IPCBUFLEN ;

	memset(&ipcmsg,0,sizeof(struct msghdr)) ;

	ipcmsg.msg_name = NULL ;
	ipcmsg.msg_namelen = 0 ;
	ipcmsg.msg_iov = vecs ;
	ipcmsg.msg_iovlen = NIOVECS ;
	ipcmsg.msg_control = &conbuf ;
	ipcmsg.msg_controllen = CMSG_SPACE(fdlen) ;

/* create the address to send to */

	rs = sockaddress_start(&toaddr,AF_UNIX,pfname,0,0) ;
	tolen = rs ;
	if (rs < 0)
	    goto ret2 ;

	ipcmsg.msg_name = &toaddr ;
	ipcmsg.msg_namelen = tolen ;

/* send the message */

	m2.tag = 0 ;
	rs = muximsg_passfd(&m2,0,ipcbuf,IPCBUFLEN) ;
	blen = rs ;
	if (rs < 0)
	    goto ret3 ;

	vecs[0].iov_len = blen ;

/* formulate the content of our message */

	memset(&m2,0,sizeof(struct muximsg_passfd)) ;

	if ((pip->svcpass != NULL) && (pip->svcpass[0] != '\0'))
	    strwcpy(m2.svc,pip->svcpass,MUXIMSG_SVCLEN) ;

	rs = muximsg_passfd(&m2,0,ipcbuf,IPCBUFLEN) ;
	blen = rs ;
	if (rs < 0)
	    goto ret2 ;

	vecs[0].iov_len = blen ;

/* build the control message */

	cmp = CMSG_FIRSTHDR(&ipcmsg) ;
	cmsize = cmsg_passfd(cmp,fd_pass) ;

	conbufsize += cmsize ;
	ipcmsg.msg_controllen = conbufsize ;

/* send it */

	rs = u_sendmsg(s,&ipcmsg,0) ;

/* get the response */

	if (rs >= 0) {

	    vecs[0].iov_len = IPCBUFLEN ;

	    ipcmsg.msg_control = NULL ;
	    ipcmsg.msg_controllen = 0 ;

	    rs = uc_recvmsge(s,&ipcmsg,0,TO_RECVMSG,0) ;
	    len = rs ;
	    if ((rs >= 0) && (len > 0)) {

	        rs1 = muximsg_response(&m0,1,ipcbuf,len) ;

	        rs = ((rs1 >= 0) && (m0.rc == 0)) ? SR_OK : SR_PROTO ;

	    } else
	        rs = SR_PROTO ;

	} /* end if (receiving the reply) */

/* we're done */
ret3:
	sockaddress_finish(&toaddr) ;

	if (msgfname[0] != '\0')
	    u_unlink(msgfname) ;

ret2:
	u_close(s) ;

ret1:
	u_close(fd_pass) ;

ret0:
	return rs ;
}
/* end subroutine (passfd) */


static int cmsg_passfd(struct cmsghdr *cmp,int fd)
{
	const int	fdlen = sizeof(int) ;
	int		cmsize ;
	uchar		*up ;

	cmsize = CMSG_SPACE(fdlen) ;

	memset(cmp,0,sizeof(struct cmsghdr)) ;
	cmp->cmsg_level = SOL_SOCKET ;
	cmp->cmsg_type = SCM_RIGHTS ;
	cmp->cmsg_len = CMSG_LEN(fdlen) ;

	up = CMSG_DATA(cmp) ;
	memcpy(up,&fd,fdlen) ;

	return cmsize ;
}
/* end subroutine (cmsg_passfd) */


