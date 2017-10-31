/* main (SYSEXEC) */

/* (another) program to switch programs */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory allocations */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program takes its own invocation name and looks it up in a program
	map to find possible filepaths to an executable to execute.  If it
	doesn't find the program in the map, or if all of the program names
	listed in the map are not executable for some reason, then the program
	executable search PATH is searched.

	In all cases, some care is taken so as to not accidentally execute
	ourselves (again)!

	Synopsis:

	$ <name> <arguments_for_name>


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ids.h>
#include	<envhelp.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	PROGINFO
#define	PROGINFO	PROGINFO
#endif


/* external subroutines */

extern int	snfilemode(char *,int,mode_t) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	proc_devino(PROGINFO *) ;
static int	proc_idsbegin(PROGINFO *) ;
static int	proc_idsend(PROGINFO *) ;
static int	proc_find(PROGINFO *,ENVHELP *,cchar *,cchar *[]) ;
static int	proc_progok(PROGINFO *,cchar *) ;
static int	proc_exec(PROGINFO *,cchar *,cchar **,cchar **) ;


/* local variables */

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
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

static cchar	*sysdirs[] = {
	"/usr/xpg4/bin",
	"/usr/ccs/bin",
	"/usr/bin",
	"/usr/SUNWspro/bin",
	"/usr/extra/bin",
	"/usr/extra/sbin",
	"/usr/extra/libexec",
	"/usr/sbin",
	NULL
} ;

static cchar	*envbads[] = {
	"LD_PRELOAD",
	"RANDOM",
	"A__z",
	NULL
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	bfile		errfile ;

#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		rs = SR_OK ;
	int		rs1 ;
	int		ex = EX_INFO ;
	cchar		*pr = NULL ;
	cchar		*cp ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

	if ((cp = getenv(VARERRORFNAME)) == NULL) cp = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,cp,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	} else if (! isNotPresent(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

/* initialize */

	pip->verboselevel = 1 ;

/* get the program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,NULL) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("main: pr=%s\n",pip->pr) ;
	debugprintf("main: sn=%s\n",pip->searchname) ;
#endif

/* go */

	if (rs >= 0) {
	    if ((rs = proc_devino(pip)) >= 0) {
	        ENVHELP		env ;
	        if ((rs = envhelp_start(&env,envbads,pip->envv)) >= 0) {
	            if ((rs = proc_idsbegin(pip)) >= 0) {
	                cchar		*en = getexecname() ;
	                char		ebuf[MAXPATHLEN+1] ;
	                if ((rs = mkpath1(ebuf,en)) >= 0) {
	                    int		bl ;
	                    cchar	*bp ;
	                    if ((bl = sfbasename(ebuf,rs,&bp)) > 0) {
	                        while (bl && (bp[bl-1] == '/')) {
	                            bl -= 1 ;
	                        }
	                        ebuf[(bp+bl)-ebuf] = '\0' ;
	                        rs = proc_find(pip,&env,bp,argv) ;
	                    } /* end if (sfbasename) */
	                } /* end if (procinfo_getename) */
	                rs1 = proc_idsend(pip) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (proc-ids) */
	            rs1 = envhelp_finish(&env) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (envhelp_finish) */
	    } /* end if (proc_devino) */
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    ex = EX_OSERR ;
	    fmt = "%s: resource failure (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	} /* end if (ok) */

	if ((rs < 0) && (ex == EX_OK)) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    ex = EX_NOPROG ;
	    fmt = "%s: inaccessible software components (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_NOENT:
	        ex = EX_NOPROG ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} /* end if (bad) */

/* early return thing */

	if (pip->efp != NULL) {
	    pip->f.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	proginfo_finish(pip) ;

badprogstart:

#if	CF_DEBUGS && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int proc_devino(PROGINFO *pip)
{
	const int	tlen = MAXPATHLEN ;
	int		rs ;
	char		tbuf[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	debugprintf("main/proc_devino: ent\n") ;
#endif

	if ((rs = proginfo_getename(pip,tbuf,tlen)) >= 0) {
	    struct ustat	sb ;
#if	CF_DEBUGS
	    debugprintf("main/proc_devino: ent tbuf=%s\n",tbuf) ;
#endif
	    if ((rs = u_stat(tbuf,&sb)) >= 0) {
	        pip->dev = sb.st_dev ;
	        pip->ino = sb.st_ino ;
#if	CF_DEBUGS
	        debugprintf("main/proc_devino: dev=\\x%08x ino=%llu\n",
	            pip->dev,pip->ino) ;
#endif
	    }
	} /* end if (proginfo_getename) */

	return rs ;
}
/* end subroutine (proc_devino) */


static int proc_idsbegin(PROGINFO *pip)
{
	return ids_load(&pip->id) ;
}
/* end subroutine (proc_idsbegin) */


static int proc_idsend(PROGINFO *pip)
{
	return ids_release(&pip->id) ;
}
/* end subroutine (proc_idsend) */


static int proc_find(PROGINFO *pip,ENVHELP *elp,cchar *bn,cchar *argv[])
{
	int		rs = SR_OK ;
	int		i ;
	char		rbuf[MAXPATHLEN+1] ;
	for (i = 0 ; sysdirs[i] != NULL ; i += 1) {
	    cchar	*sd = sysdirs[i] ;
	    if ((rs = mkpath2(rbuf,sd,bn)) >= 0) {
#if	CF_DEBUGS
	        debugprintf("main: rbuf=%s\n",rbuf) ;
#endif
	        if ((rs = proc_progok(pip,rbuf)) > 0) {
	            cchar	**ev ;
	            if ((rs = envhelp_getvec(elp,&ev)) >= 0) {
	                rs = proc_exec(pip,rbuf,argv,ev) ;
	            }
	        }
	    } /* end if (mkpath) */
#if	CF_DEBUGS
	    debugprintf("main: for-bot rs=%d\n",rs) ;
#endif
	    if ((rs < 0) && (! isNotPresent(rs))) break ;
	} /* end for */
	return rs ;
}
/* end subroutine (proc_find) */


static int proc_progok(PROGINFO *pip,cchar *progfname)
{
	struct ustat	sb ;
	int		rs ;
	int		f = FALSE ;
#if	CF_DEBUGS
	debugprintf("main/proc_progok: ent progfname=%s\n",progfname) ;
#endif
	if ((rs = u_stat(progfname,&sb)) >= 0) {
#if	CF_DEBUGS
	    {
	        const int	mlen = TIMEBUFLEN ;
	        char		mbuf[TIMEBUFLEN+1] ;
	        snfilemode(mbuf,mlen,sb.st_mode) ;
	        debugprintf("main/proc_progok: mode=%s (%07o)\n",
	            mbuf,sb.st_mode) ;
	        debugprintf("main/proc_progok: mode=\\x%08x\n",sb.st_mode) ;
	    }
#endif /* CF_DEBUGS */
	    if (S_ISREG(sb.st_mode)) {
	        if ((rs = sperm(&pip->id,&sb,X_OK)) >= 0) {
	            if ((sb.st_dev != pip->dev) || (sb.st_ino != pip->ino)) {
	                f = TRUE ;
	            }
	        } else if (isNotPresent(rs)) {
#if	CF_DEBUGS
	            debugprintf("main/proc_progok: sperm() rs=%d\n",rs) ;
#endif
	            rs = SR_OK ;
	        }
	    } /* end if (regular-file) */
	} else if (isNotPresent(rs)) {
#if	CF_DEBUGS
	    debugprintf("main/proc_progok: u_stat() rs=%d\n",rs) ;
#endif
	    rs = SR_OK ;
	}
#if	CF_DEBUGS
	debugprintf("main/proc_progok: rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (proc_progok) */


static int proc_exec(PROGINFO *pip,cchar *rbuf,cchar **argv,cchar **penvv)
{
	int		rs ;
	bflush(pip->efp) ;
#if	CF_DEBUGS
	debugprintf("main/proc_exec: rbuf=%s\n",rbuf) ;
#endif
#ifdef	COMMENT
	{
	    const int	rsn = SR_NOTFOUND ;
	    if ((rs = uc_execve(rbuf,argv,penvv)) == rsn) {
	        rs = uc_isaexecve(rbuf,argv,penvv) ;
	    }
	}
#else /* COMMENT */
	rs = uc_isaexecve(rbuf,argv,penvv) ;
#endif /* COMMENT */
#if	CF_DEBUGS
	debugprintf("main/proc_exec: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (proc_exec) */

	    
