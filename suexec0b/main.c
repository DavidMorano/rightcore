/* main */

/* main module for the SUEXEC program */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */
#define	CF_NPRINTF	1


/* revision history:

	= 1996-03-01, David A­D­ Morano

	This is a pretty much standard module for the start-up code
	of a program.


*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is the front-end (all of it I think) of a little program
	to allow users to execute SUID SHELL programs.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<field.h>
#include	<pwfile.h>
#include	<paramopt.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	NPRINTFILE		"/tmp/suexec.deb"

#ifndef	DEBUGLEVLE
#define	DEBUGLEVEL(n)		(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	matostr(const char **,int,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	findfilepath(const char *,char *,const char *,int) ;
extern int	execer(struct proginfo *,struct ustat *,char *,char *,char *,
			int,char **,char **) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(struct proginfo *) ;


/* local variables */

static const char	*argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"option",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_option,
	argopt_overlast
} ;

static const char	*procopts[] = {
	"seven",
	NULL
} ;

enum procopts {
	procopt_seven,
	procopt_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct ustat	pwd_sb, sb ;

	struct proginfo	pi, *pip = &pi ;

	PARAMOPT	aparams ;

	PWFILE		pf, *pfp ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;

	uid_t	euid = -1 ;

	gid_t	egid = -1 ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs ;
	int	rs1 ;
	int	i ;
	int	pai = -1 ;
	int	size, len ;
	int	fl, pfd ;
	int	pargc ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_failed = FALSE ;
	int	f_dash = FALSE ;
	int	f_exitargs = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS + 1] ;
	const char	*afname = NULL ;
	const char	*pwfname = NULL ;
	const char	*euser = NULL, *egroup = NULL ;
	char	buf[BUFLEN + 1] ;
	char	filebuf[MAXNAMELEN + 1], *pp ;
	char	pwdbuf[MAXPATHLEN + 1], *pwd = NULL ;
	char	progfname[MAXPATHLEN + 1] ;
	const char	*pr ;
	const char	*program = NULL ;
	const char	**pargv ;
	const char	*arg0 = NULL ;
	const char	*interpreter, *interpretarg ;
	const char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;

	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {
		pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* early things to initialize */

	pip->ofp = ofp ;

	pip->verboselevel = 1 ;

	pip->f.quiet = FALSE ;
	pip->f.sevenbit = FALSE ;


	pip->uid = getuid() ;

	pip->euid = geteuid() ;

	if (pip->uid != pip->euid)
	    pip->f.setuid = TRUE ;

	pip->gid = getgid() ;

	pip->egid = getegid() ;

	if (pip->gid != pip->egid)
	    pip->f.setgid = TRUE ;

#if	CF_DEBUGS
	debugprintf("main: initial EUID=%d EGID=%d\n",
	    pip->euid,pip->egid) ;
#endif

/* process program arguments */

	rs = paramopt_start(&aparams) ;
	pip->open.aparams = (rs >= 0) ;

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1)
		argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc - 1 ;
	while ((! f_exitargs) && (argr > 0)) {

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
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	                f_optequal = TRUE ;

	            } else {

	                avl = 0 ;
	                akl = aol ;

	            }

/* do we have a keyword or only key letters ? */

	            if ((kwi = matostr(argopts,2,aop,aol)) >= 0) {

	                switch (kwi) {

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        goto badargextra ;

	                    break ;

/* verbose */
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

/* temporary directory */
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

/* the user specified some options */
	                case argopt_option:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            paramopt_loads(&aparams,PO_OPTION,
	                                avp,avl) ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
				}

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            rs = paramopt_loads(&aparams,PO_OPTION,
	                                argp,argl) ;

	                    }

	                    break ;


/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
			    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) {

	                                rs = cfdeci(avp,avl, &pip->debuglevel) ;

	                            }
	                        }

	                        break ;

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* alternate effective group */
	                    case 'g':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
				}

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            egroup = argp ;

	                        break ;

/* alternate PASSWD file */
	                    case 'p':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
				}

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pwfname = argp ;

	                        break ;

/* alternate effective user */
	                    case 'u':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
				}

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            euser = argp ;

	                        break ;

/* verbose output */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) {

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
	                        bprintf(pip->efp,"%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

	                    } /* end switch */

	                    akp += 1 ;
				if (rs < 0)
					break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

		}

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

/* check arguments */

#if	CF_DEBUGS
	debugprintf("main: finished parsing command line arguments\n") ;
#endif

	if (pip->debuglevel >= 1) {

	    bprintf(pip->efp,
	        "%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	    bcontrol(pip->efp,BC_LINEBUF,0) ;

	    bflush(pip->efp) ;

	}

	if (f_version) {

	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	}

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto done ;


	ex = EX_OK ;

/* check a few more things */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv("TMPDIR") ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDIR ;

/* are there any early problems so far */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main: program=%s\n",program) ;
	}
	nprintf(NPRINTFILE,"main: program=%s\n",program) ;
#endif

	if ((program == NULL) || (program[0] == '\0'))
	    goto badprog ;

/* options? */

	{
		PARAMOPT_CUR	cur ;


	paramopt_curbegin(&aparams,&cur) ;

	while (paramopt_enumvalues(&aparams,PO_OPTION,&cur,&cp) >= 0) {

	    if (cp == NULL) continue ;

	    if ((i = matstr(procopts,cp,-1)) < 0)
	        continue ;

	    switch (i) {

	    case procopt_seven:
	        pip->f.sevenbit = TRUE ;
	        break ;

	    } /* end switch */

	} /* end while (option processing) */

	paramopt_curend(&aparams,&cur) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: sevenbit=%d\n",pip->f.sevenbit) ;
#endif

	} /* end block */


/* deal with password stuff */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: PASSWD file?\n") ;
#endif

	pfp = NULL ;
	if (pwfname != NULL) {

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: password file=%s\n",
	            pip->progname,pwfname) ;

	    rs = perm(pwfname,pip->uid,pip->gid,NULL,R_OK) ;

	    if (rs < 0)
	        goto badpwfile ;

	    if ((rs = pwfile_open(&pf,pwfname)) >= 0)
	        pfp = &pf ;

	} /* end if (processing passwd file) */


/* get the current working directory */

	if (pwd == NULL) {

	    if (getpwd(pwdbuf,MAXPATHLEN) > 0)
	        pwd = pwdbuf ;

	    u_stat(pwd,&pwd_sb) ;

	} /* end if */


/* find the file associated with this program, search PATH */

	rs = findfilepath(NULL,progfname,program,X_OK) ;

	if (rs < 0)
	    goto badfind ;

	if ((rs == 0) && (progfname[0] == '\0'))
	    strwcpy(progfname,program,MAXPATHLEN) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main: findfilepath progfname=%s\n",progfname) ;
	}
	nprintf(NPRINTFILE,"main: 1 progfname=%s\n",progfname) ;
#endif

	pp = progfname ;
	rs = currentdir(&pwd_sb,progfname,filebuf) ;

	if (rs > 0)
	    pp = filebuf ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main: findfilepath progfname=%s\n",pp) ;
	}
	nprintf(NPRINTFILE,"main: 2 progfname=%s\n",pp) ;
#endif

/* do we have an alternate user specified on invocation? */


/* is this program file accessible and executable by the current user? */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: u_open(), progfname=%s\n",pp) ;
#endif

	rs = u_open(pp,O_RDONLY,0666) ;

	pfd = rs ;
	if (rs < 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: u_open(), progfname=%s\n",pp) ;
#endif

	    goto badstat ;
	}

	if (pfd >= 0)
	    u_fstat(pfd,&sb) ;

	else
	    u_stat(pp,&sb) ;

#if	CF_DEBUG
	nprintf(NPRINTFILE,"main: u_stat() rs=%d\n",rs) ;
	if (DEBUGLEVEL(3))
	    debugprintf("main: EUID=%d EGID=%d\n",sb.st_uid,sb.st_gid) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: perm()\n") ;
#endif

	if (pfd >= 0)
	    rs = fperm(pfd,pip->uid,pip->gid,NULL,X_OK) ;

	else
	    rs = perm(pp,pip->uid,pip->gid,NULL,X_OK) ;

	if (rs < 0)
	    goto badacc1 ;


/* do we have an ARG0 given? */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: argc=%d argr=%d pai=%d arg0=%s\n",
	        argc,argr,pai,arg0) ;
#endif

	pargc = argr + 1 ;
	if ((arg0 == NULL) || (arg0[0] == '\0')) {

	    int	sl ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: custom argument build\n") ;
#endif

	    sl = sfbasename(pp,-1,&cp) ;

	    arg0 = mallocstrw(cp,sl) ;

		if (arg0 == NULL) {

			rs = SR_NOMEM ;
			goto badmalloc ;
		}

/* create a new program argument array */

	    size = (pargc + 1) * sizeof(char *) ;
	    rs = uc_malloc(size,&pargv) ;

	    if (rs < 0)
	        goto badmalloc ;

	    pargv[0] = arg0 ;
	    for (i = 1 ; i < (argr + 1) ; i += 1)
	        pargv[i] = argv[pai + i] ;

	    pargv[i] = NULL ;

	} else
	    pargv = argv + pai ;


/* does the file have SUID or SGID? */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {

	    debugprintf("main: mode=%07o\n",sb.st_mode) ;

	    debugprintf("main: ixusr=%d\n",
	        (sb.st_mode & S_IXUSR) ? 1 : 0) ;

	    debugprintf("main: ixgrp=%d\n",
	        (sb.st_mode & S_IXGRP) ? 1 : 0) ;

	    debugprintf("main: isuid=%d\n",
	        (sb.st_mode & S_ISUID) ? 1 : 0) ;

	    debugprintf("main: isgid=%d\n",
	        (sb.st_mode & S_ISGID) ? 1 : 0) ;

	}
#endif /* CF_DEBUG */

	if (((sb.st_mode & S_IXUSR) == S_IXUSR) &&
	    ((sb.st_mode & S_ISUID) == S_ISUID))
	    euid = sb.st_uid ;

	if (((sb.st_mode & S_IXGRP) == S_IXGRP) &&
	    ((sb.st_mode & S_ISGID) == S_ISGID))
	    egid = sb.st_gid ;


/* do we have to do some set-IDs? */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: setting EGID=%d\n",egid) ;
#endif

	if ((egid >= 0) && (egid != pip->egid))
	    u_setegid(egid) ;

	else if (pip->egid != pip->gid)
	    u_setegid(pip->gid) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: final EGID=%d\n",u_getegid()) ;
#endif


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: setting EUID=%d current_program euid=%d\n",
	        euid,pip->euid) ;
#endif

	if (euid >= 0) {

		if (euid != pip->euid) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: setting priviledge\n") ;
#endif

	    u_seteuid(euid) ;

		}

	} else if (pip->euid != pip->uid) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: removing priviledge\n") ;
#endif

	    u_seteuid(pip->uid) ;

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: final EUID=%d\n",u_geteuid()) ;
#endif


/* do we have a good open of the program file */

	if (pfd < 0) {

	    if ((pfd = u_open(pp,O_RDONLY,0666)) < 0)
	        goto badacc2 ;

	} /* end if */

	if ((len = u_read(pfd,buf,BUFLEN)) <= 0)
	    goto badlen ;

	interpreter = NULL ;
	if ((buf[0] == '#') && (buf[1] == '!')) {

	    buf[len] = '\0' ;
	    if ((cp = strchr(buf,'\n')) != NULL) {

	        *cp = '\0' ;
	        len = (cp - buf) ;

	    } /* end if */

	    cp = buf + 2 ;
	    len -= 2 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: interpreter len=%d\n",len) ;
#endif

	    fl = nextfield(cp,len,&interpreter) ;

	    if (fl <= 0) {

	        rs = SR_NOEXEC ;
	        goto badexec ;
	    }

	    len -= (interpreter + fl - cp - 1) ;
	    interpreter[fl] = '\0' ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: len=%d fl=%d interpreter=%s\n",
	            len,fl,interpreter) ;
#endif

	    interpretarg = interpreter + fl ;
	    if (len > 1) {

	        cp = interpreter + fl + 1 ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main: more next len=%d\n",len) ;
#endif

	        fl = nextfield(cp,len,&interpretarg) ;

	    } else
	        fl = 0 ;

	    interpretarg[fl] = '\0' ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: fl=%d interpretarg=%s\n",fl,interpretarg) ;
#endif

	} /* end if (was an interpreted file) */

	u_close(pfd) ;


/* finish up and out ! */

	if (pwfname != NULL)
	    pwfile_close(&pf) ;

	bflush(pip->efp) ;


/* OK, we do it */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {

	    debugprintf("main: execing program, EUID=%d EGID=%d\n",
	        u_geteuid(), u_getegid()) ;

	    for (i = 0 ; pargv[i] != NULL ; i += 1)
	        debugprintf("main: pargv[%d]=%s\n",i,pargv[i]) ;

	}
#endif /* CF_DEBUG */

	if (interpreter == NULL) {
	    rs = u_execve(pp,pargv,envv) ;
	} else
	    rs = execer(pip,&pwd_sb,interpreter,interpretarg,
	        pp,pargc,pargv,envv) ;

	ex = EX_NOEXEC ;
	bprintf(pip->efp,"%s: could not EXEC program (%d)\n",
	    pip->progname,rs) ;


/* we are out of here */
done:
retearly:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: program exiting ex=%u\n",
	        pip->progname,ex) ;

ret1:
	if (pip->open.aparams)
	    paramopt_finish(&aparams) ;

	if (pip->efp != NULL)
	    bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

	return ex ;

/* program usage */
usage:
	usage(pip) ;

	goto retearly ;

badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

badprog:
	bprintf(pip->efp,"%s: no program was specified\n",
	    pip->progname) ;

	goto bad0 ;

badpwfile:
	bprintf(pip->efp,"%s: could not open PASSWORD file (%d)\n",
	    pip->progname,rs) ;

	goto bad0 ;

badfind:
	bprintf(pip->efp,"%s: no program=%s (%d)\n",
	    pip->progname,program,rs) ;

	goto bad1 ;

badstat:
	bprintf(pip->efp,"%s: no stat program=%s (%d)\n",
	    pip->progname,pp,rs) ;

	goto bad1 ;

badacc1:
	bprintf(pip->efp,"%s: could not access program for execution (%d)\n",
	    pip->progname,rs) ;

	goto bad1 ;

badmalloc:
	bprintf(pip->efp,"%s: could not allocate memory (%d)\n",
	    pip->progname,rs) ;

	goto bad1 ;

badacc2:
	bprintf(pip->efp,"%s: could not access program for execution (%d)\n",
	    pip->progname,rs) ;

	goto bad1 ;

badlen:
	rs = len ;
	bprintf(pip->efp,"%s: could not read program file (%d)\n",
	    pip->progname,rs) ;

	goto bad2 ;

badexec:
	rs = len ;
	bprintf(pip->efp,"%s: could not execute program (%d)\n",
	    pip->progname,rs) ;

	goto bad2 ;

/* bad stuff */
bad2:
	u_close(pfd) ;

/* bad stuff comes to here */
bad1:
	if (pwfname != NULL)
	    pwfile_close(&pf) ;

bad0:
	ex = EX_DATAERR ;
	goto done ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [-V] <filepath> [<arg0> ...]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */



