/* progexec */

/* progexec the execution request */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */
#define	CF_DEBUG	0		/* debug print-outs switchable */
#define	CF_DEBUGE	0		/* debug 'u_execve(2)' */
#define	CF_FANCYSHUN	1		/* put fancy shell-under in environ */


/* revision history:

	= 1990-11-01, David A­D­ Morano

	This subroutine was originally written.


	= 2001-04-11, David A­D­ Morano

	This old dog program has been enhanced to serve as the environment
	wiper for executing MIPS programs.


*/

/* Copyright © 1990,2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine performs an 'exec(2)' on the given program with
	its environment and arguments.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<keyopt.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<buffer.h>
#include	<storebuf.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"envs.h"


/* local defines */

#ifndef	elementsof
#define	elementsof(a)	(sizeof(a) / sizeof((a)[0]))
#endif

#ifndef	VARPATH
#define	VARPATH	"PATH"
#endif

#ifndef	VARAST
#define	VARAST	"AST"
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	45		/* can hold long128_t in decimal */
#endif

#undef	SHUNLEN
#define	SHUNLEN		(DIGBUFLEN+2+MAXNAMELEN)


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snshellunder(char *,int,pid_t,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	ctdeci(char *,int,int) ;
extern int	ctdecl(char *,int,long) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	fileisprint(const char *) ;
extern int	hasallplusminus(const char *,int) ;
extern int	hasallminus(const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strnnlen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct intprog {
	char	fname[MAXPATHLEN + 1] ;
	char	arg[MAXPATHLEN + 1] ;
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int progexec(pip,progfname,argv,argr)
struct proginfo	*pip ;
const char	*progfname ;
const char	*argv[] ;
int		argr ;
{
	BUFFER	b ;

	vecstr	*elp ;

	int	rs = SR_OK ;
	int	si = 0 ;
	int	ai = 0 ;
	int	i ;
	int	cl ;
	int	size ;
	int	start = 10 ;
	int	f_shell ;
	int	f_m = FALSE ;
	int	f_sa = FALSE ;

	const char	**av = NULL ;
	const char	**ev = NULL ;
	const char	*abuf = NULL ;
	const char	*cp ;


/* sanity check */

	if (pip == NULL) return SR_FAULT ;

	if (progfname == NULL) return SR_FAULT ;
	if (progfname[0] == '\0') return SR_INVALID ;

/* continue */

	elp = &pip->exports ;
	f_shell = pip->f.shell ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("progexec: progfname=%s\n",progfname) ;
	    debugprintf("progexec: f_shell=%u\n",pip->f.shell) ;
	    debugprintf("progexec: f_progdash=%u\n",pip->f.progdash) ;
	    debugprintf("progexec: argr=%d\n",argr) ;
	    for (i = 0 ; argv[i] != NULL ; i += 1)
	        debugprintf("progexec: arg[%u]=>%s<\n",i,argv[i]) ;
	}
#endif /* CF_DEBUG */

#if	CF_FANCYSHUN
	{
	    const int	slen = SHUNLEN ;
	    char	sbuf[SHUNLEN+1] ;
	    if ((rs = snshellunder(sbuf,slen,pip->pid,progfname)) >= 0) {
	        rs = vecstr_envadd(elp,"_",sbuf,rs) ;
	    }
	}
#else
	rs = vecstr_envadd(elp,"_",progfname,-1) ;
#endif /* CF_FANCYSHUN */
	if (rs < 0) goto ret0 ;

	size = (argr + 2) * sizeof(const char *) ;
	if ((rs = uc_malloc(size,&av)) >= 0) {

/* should we prefix the minus thing? */

	    f_m = FALSE ;
	    f_m = f_m || pip->f.progdash ;

/* setup the zeroth argument */

	    si = 0 ;
	    cp = NULL ;
	    cl = -1 ;
	    if (f_shell) {
	        f_sa = TRUE ;
	    } else {
	        if (argr > 0) {
	            cp = argv[0] ;
	            cl = -1 ;
	            si = 1 ;
	            argr -= 1 ;
	            if (cp[0] != '\0') {
	                if (hasallplusminus(cp,-1)) {
	                    f_sa = TRUE ;
	                    f_m = f_m || hasallminus(cp,cl) ;
	                }
	            } else
	                f_sa = TRUE ;
	        } else
	            f_sa = TRUE ;
	    } /* end if */

	    if (f_sa) {
	        cl = sfbasename(progfname,-1,&cp) ;
	        start = (cl+2) ;
	    }

	    if ((rs = buffer_start(&b,start)) >= 0) {

	        if (f_sa || f_m) {
	            if (f_m)
	                rs = buffer_char(&b,'-') ;
	            if (rs >= 0)
	                rs = buffer_strw(&b,cp,cl) ;
	            if (rs >= 0) {
	                buffer_get(&b,&abuf) ;
	                av[ai++] = abuf ;
	            } /* end if */
	        } else if (cp != NULL) {
	            av[ai++] = cp ;
	        } /* end if */

/* setup all remaining arguments */

	        if (rs >= 0) {
	            for (i = si ; (argr > 0) && (argv[i] != NULL) ; i += 1) {
#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("progexec: arg[%u]=>%s<\n",i,argv[i]) ;
#endif
	                argr -= 1 ;
	                av[ai++] = argv[i] ;
	            } /* end for */
	            av[ai] = NULL ;
	        } /* end if */

	        if (rs >= 0) {
	            const char	*pfn = progfname ;
	            char	tmpfname[MAXPATHLEN + 1] ;
	            if (progfname[0] != '/') {
	                if ((rs = proginfo_pwd(pip)) >= 0) {
	                    pfn = tmpfname ;
	                    rs = mkpath2(tmpfname,pip->pwd,progfname) ;
	                }
	            }
	            if (rs >= 0)
	                rs = vecstr_envadd(elp,"_EF",pfn,-1) ;
	        }

	        if (rs >= 0)
	            rs = vecstr_envadd(elp,"_A0",av[0],-1) ;

	        if (rs >= 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(3)) {
	                int	i ;
	                for (i = 0 ; av[i] != NULL ; i += 1)
	                    debugprintf("progexec: av[%u]=>%s<\n",i,av[i]) ;
	            }
#endif

	            if ((rs = vecstr_getvec(elp,&ev)) >= 0) {

#if	CF_DEBUGE
#else
	                {
	                    const char **eav = (const char **) av ;
	                    const char **eev = (const char **) ev ;
	                    rs = u_execve(progfname,eav,eev) ;
	                }
#endif /* CF_DEBUGE */

	            } /* end if */

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("progexec: u_execve() rs=%d\n",rs) ;
#endif

	        } /* end if (get-buffer) */

	        buffer_finish(&b) ;
	    } /* end if (buffer) */

	    uc_free(av) ;
	} /* end if (memory-allocation) */

ret0:
	return rs ;
}
/* end subroutine (progexec) */


/* local subroutines */


