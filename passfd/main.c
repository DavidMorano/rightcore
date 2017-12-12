/* main */

/* non-generic (?) front-end */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ passfd <passfile>


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<stropts.h>
#include	<poll.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	POLLMULT
#define	POLLMULT	1000
#endif

#define	DEBUGFILE	"/tmp/passfd.deb"

#define	TO_FDPASS	15		/* seconds */


/* external subroutines */

extern int	sfshrink(const char *,int,char **) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecmfui(const char *,int,int *) ;
extern int	cfnumi(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	acceptpass(int,struct strrecvfd *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	printhelp(void *,cchar *,cchar *,cchar *) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;

static int	procpassfd(PROGINFO *,const char *,int) ;
static int	procserve(PROGINFO *,const char *,const char *,mode_t) ;


/* local variables */

static volatile int	if_exit ;
static volatile int	if_int ;

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"srvfile",
	"mntfile",
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
	argopt_srvfile,
	argopt_mntfile,
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
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	BITS		pargs ;
	KEYOPT		akopts ;
	bfile		errfile ;
	mode_t		srvperm = 0666 ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs ;
	int		rs1 ;
	int		fd_input = FD_STDIN ;
	int		v ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_help = FALSE ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*pfname = NULL ;
	const char	*sfname = NULL ;
	const char	*cp ;


	if_int = FALSE ;
	if_exit = FALSE ;

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
	rs = proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

/* start parsing the arguments */

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

/* help */
	                case argopt_help:
	                    f_help = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl > 0)
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

/* argument-list name */
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

	                case argopt_srvfile:
	                case argopt_mntfile:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl > 0)
	                            sfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                sfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
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

	                    case 'q':
	                        pip->verboselevel = 0 ;
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

	                    case 'm':
	                    case 'p':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfnumi(argp,argl,&v) ;
					srvperm = v ;
	                                srvperm &= 0666 ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
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
	} else if (! isNotPresent(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

/* get the program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pr=%s\n",pip->pr) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* do the thing */

	for (ai = 1 ; ai < argc ; ai += 1) {
	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    f = f || (ai > ai_pos) ;
	    if (f) {
	        pfname = argv[ai] ;
	        bits_clear(&pargs,ai) ;
	        break ;
	    }
	} /* end for */

	if (pfname == NULL) {
	    if ((cp = getenv(VARPASSFILE)) != NULL) {
	        pfname = cp ;
	    }
	}

	if (pfname == NULL) {
	    ex = EX_USAGE ;
	    bprintf(pip->efp,"%s: no pass file was specified\n",
	        pip->progname) ;
	}

	if (rs >= 0) {
	    if (sfname != NULL) {
	        rs = procserve(pip,pfname,sfname,srvperm) ;
	    } else {
	        if (argval != NULL) {
		    if ((rs = cfdeci(argval,-1,&v)) >= 0) {
		        fd_input = v ;
		    }
	        }
	        if (rs >= 0) {
	            rs = procpassfd(pip,pfname,fd_input) ;
	        }
	    } /* end if */
	} /* end if (ok) */

/* done */
	if ((ex == EX_OK) && (rs < 0)) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: could not complete pass operation (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    ex = mapex(mapexs,rs) ;
	} else if (if_exit) {
	    ex = EX_TERM ;
	} else if (if_int) {
	    ex = EX_INTR ;
	}

/* early return thing */
retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

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
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
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

	fmt = "%s: USAGE> %s <passfile> [-mnt <mntfile>] [-m <filemode>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procpassfd(PROGINFO *pip,cchar *pfname,int fd_input)
{
	struct ustat	sb ;
	int		rs ;

	if (pfname == NULL) return SR_FAULT ;

	if (pfname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/passfd: pfname=%s\n",pfname) ;
#endif

	if (fd_input < 0) return SR_BADF ;

	if ((rs = u_fstat(fd_input,&sb)) >= 0) {
	    const int	of = (O_WRONLY | O_NDELAY) ;
	    if ((rs = u_open(pfname,of,0666)) >= 0) {
	        const int	fd_pass = rs ;
	        if ((rs = u_fstat(fd_pass,&sb)) >= 0) {
	            if (S_ISCHR(sb.st_mode) || S_ISFIFO(sb.st_mode)) {
	                int	i ;
	                for (i = 0 ; (rs >= 0) && (i < TO_FDPASS) ; i += 1) {
	                    if (i > 0) sleep(1) ;
	                    rs = u_ioctl(fd_pass,I_SENDFD,fd_input) ;
	                } /* end for */
		    }
	        } else {
	            rs = SR_NOSTR ;
	        }
	        u_close(fd_pass) ;
	    } /* end if (open) */
	} /* end if (stat-input) */

	return rs ;
}
/* end subroutine (procpassfd) */


static int procserve(PROGINFO *pip,cchar *pfname,cchar *sfname,mode_t om)
{
	struct pollfd	fds[2] ;
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		to = POLLINT ;
	int		pto ;
	int		i ;
	int		sfd, pfd ;
	int		pipes[2] ;

	if (sfname[0] == '\0') return SR_INVALID ;

	rs = u_stat(sfname,&sb) ;
	if (rs == SR_NOENT) {
	    rs = u_creat(sfname,om) ;
	    sfd = rs ;
	    if (rs >= 0)
	        u_close(sfd) ;
	}

	if (rs < 0)
	    goto bad0 ;

	rs = u_pipe(pipes) ;
	sfd = pipes[0] ;		/* server-side */
	if (rs < 0)
	    goto bad0 ;

	rs = u_ioctl(pipes[1],I_PUSH,"connld") ;
	if (rs < 0)
	    goto bad1 ;

/* attach the client end to the file created above */

	rs = uc_fattach(pipes[1],sfname) ;
	if (rs < 0)
	    goto bad1 ;

	u_close(pipes[1]) ;
	pipes[1] = -1 ;

	i = 0 ;
	fds[i].fd = sfd ;
	fds[i].events = (POLLIN | POLLPRI) ;
	i += 1 ;
	fds[i].fd = -1 ;
	fds[i].events = 0 ;

	pto = (to * POLLMULT) ;
	while (rs >= 0) {

	    if ((rs = u_poll(fds,1,pto)) > 0) {
	        const int	re = fds[0].revents ;

	        if ((re & POLLIN) || (re & POLLPRI)) {
	            struct strrecvfd	passer ;
	            if ((rs = acceptpass(sfd,&passer,-1)) >= 0) {
	                pfd = rs ;
	                rs = procpassfd(pip,pfname,pfd) ;
	                u_close(pfd) ;
	            } /* end if */
	        } else if (re & POLLHUP) {
	            rs = SR_HANGUP ;
	        } else if (re & POLLERR) {
	            rs = SR_POLLERR ;
	        } else if (re & POLLNVAL) {
	            rs = SR_NOTOPEN ;
	        } /* end if (poll returned) */

	    } else if (rs == SR_INTR) {
	        rs = SR_OK ;
	    }

	    if ((rs >= 0) && if_exit) {
	        rs = SR_INTR ;
	        break ;
	    }

	} /* end while */

	u_close(sfd) ;

	uc_fdetach(sfname) ;

ret0:
	return rs ;

/* bad stuff */
bad1:
	for (i = 0 ; i < 2 ; i += 1) {
	    if (pipes[i] >= 0)
	        u_close(pipes[i]) ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (procserve) */


