/* main */

/* MACTRUNC program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	The program was written from scratch to do what the previous
	program by the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a fairly generic front-end subroutine for a program.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecmfui(const char *,int,int *) ;
extern int	cfdecmfull(const char *,int,ULONG *) ;
extern int	isdigitlatin(int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

static int	usage(struct proginfo *) ;


/* external variables */


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"sn",
	"pm",
	"option",
	"set",
	"follow",
	"af",
	"of",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_sn,
	argopt_pm,
	argopt_option,
	argopt_set,
	argopt_follow,
	argopt_af,
	argopt_of,
	argopt_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	ULONG	argvalue = -1 ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan ;
	int	rs, rs1 ;
	int	cl ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	const char	*pr = NULL ;
	const char	*pmspec = NULL ;
	const char	*searchname = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


	pip->verboselevel = 1 ;

/* process program arguments */

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
		const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            rs = cfdecmfull((argp + 1),(argl - 1),&argvalue) ;

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

/* do we have a keyword match or should we assume only key letters? */

	            kwi = matostr(argopts,2,akp,akl) ;

	            if (kwi >= 0) {

	                switch (kwi) {

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

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;

	                    break ;

/* verbose */
	                case argopt_verbose:
	                    pip->verboselevel = 2 ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

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

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *akp) {

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
				    if (avl)
	                            rs = cfdeci(avp,avl, &pip->debuglevel) ;

	                        }

	                        break ;

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
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

	                    case 'V':
	                        f_version = TRUE ;
	                        if (f_optequal)
	                            rs = SR_INVALID ;

	                        break ;

/* file name length restriction */
	                    case 'l':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
				    if (avl)
	                            rs = cfdecmfull(avp,avl, &argvalue) ;

	                        } else {

	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
	                                break ;
	                            }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

				    if (argl)
	                            rs = cfdecmfull(argp,argl,&argvalue) ;

	                        }

	                        break ;

/* verbose output */
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
	                        fprintf(pip->efp,
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                    } /* end switch */

	                    akp += 1 ;
	                    if (rs < 0)
	                        break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits or progopts) */

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

	    fprintf(pip->efp,
	        "%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	    bcontrol(pip->efp,BC_LINEBUF,0) ;

	    bflush(pip->efp) ;

	}

	if (f_version)
	    fprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv(VARTMPDNAME) ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;

/* get ready */

/* OK, we do it */

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = procfile(pip,&aparams,cp) ;

	        if (rs < 0) {

			fprintf(pip->efp,
			"%s: error (%d) in file=%s\n",
			pip->progname,rs,cp) ;

			break ;
		}

	} /* end for (looping through requested circuits) */

	if ((pan == 0) && (afname == NULL))
	    goto badnodirs ;

badoutopen:
	if (pip->debuglevel > 0)
	    fprintf(pip->efp,"%s: files=%u processed=%u\n",
	        pip->progname,pip->c_files,pip->c_processed) ;

done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->debuglevel > 0)
	    fprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

/* we are out of here */
retearly:
ret3:

ret2:
	if (pip->efp != NULL)
	    fclose(pip->efp) ;

ret1:
ret0:
	return ex ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	fprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

badnodirs:
	ex = EX_USAGE ;
	fprintf(pip->efp,"%s: no files or directories were specified\n",
	    pip->progname) ;

	goto ret1 ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;


	rs = fprintf(pip->efp,
	    "%s: USAGE> %s [<dir(s)> ...] [-s <suffix(es)>] [-Vv]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = fprintf(pip->efp,
	    "%s: \t[-f] [-af {<argfile>|-}]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


int procfile(pip,namespec,trunclen)
struct proginfo	*pip ;
char		namespec[] ;
ULONG		trunclen ;
{
	struct stat64	sb ;

	ULONG	flen ;

	int	rs ;
	int	fd ;
	int	f_flen ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	*fname ;
	char	*cp ;


	if (namespec == NULL)
	    return SR_FAULT ;

	if (namespec[0] == '\0')
	    return SR_OK ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	    debugprintf("process: namespec=\"%s\"\n",namespec) ;
#endif

	flen = trunclen ;
	f_flen = pip->f.trunclen ;

	if (! pip->f.literal) {

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	    debugprintf("process: not literal\n") ;
#endif

	    fname = namespec ;
	    if ((cp = strchr(namespec,'=')) != NULL) {

	        fname = tmpfname ;
	        strwcpy(tmpfname,namespec,(cp - namespec)) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	    debugprintf("process: fname=%s\n",fname) ;
#endif

	        rs = cfdecmfull((cp + 1),-1,&flen) ;
	        f_flen = (rs >= 0) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	    debugprintf("process: f_flen=%d flen=%llu\n",f_flen,flen) ;
#endif

	    }

	} else
	    fname = namespec ;

/* do it */

	fd = FD_STDIN ;
	if (strcmp(fname,"-") != 0) {

		rs = u_open64(fname,O_WRONLY,0666) ;
		fd = rs ;

	} else
		rs = fperm64(fd,-1,-1,NULL,W_OK) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	    debugprintf("process: u_open()/fperm() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    return rs ;

	rs = u_fstat64(fd,&sb) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3) {
	    debugprintf("process: u_fstat() rs=%d\n",rs) ;
	    debugprintf("process: isreg=%d\n",
			S_ISREG(sb.st_mode)) ;
	}
#endif

	if ((rs < 0) || (! S_ISREG(sb.st_mode)))
	    goto done ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	    debugprintf("process: filesize=%lld flen=%llu\n",
		sb.st_size,flen) ;
#endif

	if (pip->debuglevel > 0) {

		bprintf(pip->efp,"%s: len=%llu file=%s\n",
			pip->progname,flen,fname) ;

	}

	rs = 0 ;
	if (f_flen && (sb.st_size > flen)) {

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	    debugprintf("process: truncating to len=%llu\n",flen) ;
#endif

	    rs = uc_ftruncate64(fd,(offset_t) flen) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	    debugprintf("process: uc_ftruncate() rs=%d\n",rs) ;
#endif

	    rs = 1 ;

	} /* end if (file needed truncating) */

done:
	u_close64(fd) ;

	return rs ;
}
/* end subroutine (procfile) */



