/* progconf */

/* program configuration */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 2008-10-10, David A­D­ Morano
	This was adapted from the BACKGROUND program.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module contains the subroutines that manage program configuration.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<paramfile.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"prsetfname.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	KBUFLEN
#define	KBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	PBUFLEN
#define	PBUFLEN		(KBUFLEN + VBUFLEN + MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(4 * MAXPATHLEN)
#endif

#define	PCONF		struct pconf
#define	PCONF_FL	struct pconf_flags


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecmfi(const char *,int,int *) ;
extern int	cfdecmfu(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	hasalldig(const char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* local structures */

struct pconf_flags {
	uint		opened:1 ;
} ;

struct pconf {
	PROGINFO	*pip ;
	const char	**envv ;
	PCONF_FL	f ;
	PARAMFILE	params ;
	int		nf ;		/* n-files */
} ;


/* forward references */

static int procfile(PROGINFO *,const char **,const char *) ;
static int procfile_begin(PROGINFO *) ;
static int procfile_end(PROGINFO *) ;
static int procfile_load(PROGINFO *,const char *) ;
static int procauxprog(PROGINFO *,const char *,int) ;

static int pconf_start(PCONF *,PROGINFO *,const char **) ;
static int pconf_fileadd(PCONF *,const char *) ;
static int pconf_check(PCONF *,time_t) ;
static int pconf_load(PCONF *) ;
static int pconf_loader(PCONF *,char *,int) ;
static int pconf_finish(PCONF *) ;


/* local variables */

static const char	*sysconfs[] = {
	"%p/etc/%n/%n.%f",
	"%p/etc/%n/%f",
	"%p/etc/%n.%f",
	"%p/%n.%f",
	NULL
} ;

static const char	*usrconfs[] = {
	"%h/etc/%n/%n.%f",
	"%h/etc/%n/%f",
	"%h/etc/%n.%f",
	NULL
} ;

static const char	*params[] = {
	"cmdkey",
	"logfile",
	"loglen",
	"logsize",
	"mailcheck",
	"testmsg",
	"auxprog",
	"mbdefault",
	"mbinput",
	"mbspam",
	"mbtrash",
	NULL
} ;

enum params {
	param_cmdkey,
	param_logfile,
	param_loglen,
	param_logsize,
	param_mailcheck,
	param_testmsg,
	param_auxprog,
	param_mbdefault,
	param_mbinput,
	param_mbspam,
	param_mbtrash,
	param_overlast
} ;

static const char	*prognames[] = {
	"shell",
	"getmail",
	"mailer",
	"editor",
	"metamail",
	"pager",
	"postspam",
	NULL
} ;

enum prognames {
	progname_shell,
	progname_getmail,
	progname_mailer,
	progname_editor,
	progname_metamail,
	progname_pager,
	progname_postspam,
	progname_overlast
} ;


/* exported subroutines */


int progconf_begin(PROGINFO *pip)
{
	int		rs = SR_OK ;

#if	CF_DEBUG 
	if (DEBUGLEVEL(3))
	    debugprintf("progconf_begin: ent\n") ;
#endif

/* look for system configuration file */

	if ((rs >= 0) && (! pip->f.nosysconf)) {
	    cchar	*cfn = CONFIGFNAME ;
	    rs = procfile(pip,sysconfs,cfn) ;
	}

#if	CF_DEBUG 
	if (DEBUGLEVEL(3))
	    debugprintf("progconf_begin: 1 rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    cchar	*cfn = pip->cfname ;
	    if (cfn == NULL) cfn = CONFIGFNAME ;
	    rs = procfile(pip,usrconfs,cfn) ;
	}

#if	CF_DEBUG 
	if (DEBUGLEVEL(3))
	    debugprintf("progconf_begin: 2 rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && (pip->config != NULL)) {
	    PCONF	*csp = pip->config ;
	    rs = pconf_load(csp) ;
	}

	if (rs < 0) {
	    procfile_end(pip) ;
	}

#if	CF_DEBUG 
	if (DEBUGLEVEL(3))
	    debugprintf("progconf_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progconf_begin) */


int progconf_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->config != NULL) {
	    rs1 = procfile_end(pip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

	return rs ;
}
/* end subroutine (progconf_end) */


int progconf_check(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (pip->config != NULL) {
	    PCONF	*csp = pip->config ;

	    rs = pconf_check(csp,pip->daytime) ;
	    f = rs ;

	} /* end if (conf) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (progconf_check) */


/* local subroutines */


static int procfile(PROGINFO *pip,cchar **confs,cchar *cfn)
{
	vecstr		*svp = &pip->svars ;
	const int	tlen = MAXPATHLEN ;
	const int	m = R_OK ;
	int		rs = SR_OK ;
	int		rs1 ;
	char		tfname[MAXPATHLEN+1] ;

	if ((rs1 = permsched(confs,svp,tfname,tlen,cfn,m)) >= 0) {
	    rs = procfile_load(pip,tfname) ;
	} else if (! isNotPresent(rs1))
	    rs = rs1 ;

	return rs ;
}
/* end subroutine (procfile) */


static int procfile_begin(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->config == NULL) {
	    const int	size = sizeof(PCONF) ;
	    void	*p ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        PCONF		*csp = p ;
	        const char	**envv = pip->envv ;
	        pip->config = p ;
	        rs = pconf_start(csp,pip,envv) ;
	        if (rs < 0) {
	            uc_free(pip->config) ;
	            pip->config = NULL ;
	        }
	    } /* end if (memory-allocation) */
	} /* end if (instatiation) */

	return rs ;
}
/* end subroutine (procfile_begin) */


static int procfile_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->config != NULL) {
	    PCONF	*csp = pip->config ;
	    rs1 = pconf_finish(csp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(pip->config) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->config = NULL ;
	} /* end if */

	return rs ;
}
/* end subroutine (procfile_end) */


static int procfile_load(PROGINFO *pip,const char *fname)
{
	int		rs = SR_OK ;

	if (pip->config == NULL) {
	    rs = procfile_begin(pip) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progconf_/procfile_load: procfile_begin() rs=%d\n",
		    rs) ;
#endif
	} /* end if (instatiation) */

	if ((rs >= 0) && (pip->config != NULL)) {
	    PCONF	*csp = pip->config ;
	    rs = pconf_fileadd(csp,fname) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progconf_/procfile_load: pconf_fileadd() "
		    "rs=%d\n",rs) ;
#endif
	}

	return rs ;
}
/* end subroutine (procfile_load) */


static int pconf_start(PCONF *csp,PROGINFO *pip,const char **envv)
{

	memset(csp,0,sizeof(PCONF)) ;
	csp->pip = pip ;
	csp->envv = envv ;

	return SR_OK ;
}
/* end subroutine (pconf_start) */


static int pconf_finish(PCONF *csp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (csp->nf > 0) {
	    csp->nf = 0 ;
	    rs1 = paramfile_close(&csp->params) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (pconf_finish) */


static int pconf_fileadd(PCONF *csp,const char *fname)
{
	PARAMFILE	*pfp = &csp->params ;
	int		rs ;
	if (csp->nf == 0) {
	    const char	**envv = csp->envv ;
	    rs = paramfile_open(pfp,envv,fname) ;
	} else {
	    rs = paramfile_fileadd(pfp,fname) ;
	}
	if (rs >= 0) csp->nf += 1 ;
	return rs ;
}
/* end subroutine (pconf_fileadd) */


static int pconf_check(PCONF *csp,time_t dt)
{
	int		rs ;
	int		f = FALSE ;

	if ((rs = paramfile_check(&csp->params,dt)) > 0) {
	    if (csp->nf > 0) {
	        f = TRUE ;
	        rs = pconf_load(csp) ;
	    }
	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (pconf_check) */


static int pconf_load(PCONF *csp)
{
	PROGINFO	*pip = csp->pip ;
	PARAMFILE	*pfp = &csp->params ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ; /* ¥ GCC false complaint */
	if (pfp == NULL) return SR_FAULT ; /* ¥ GCC false complaint */

	if (csp->nf > 0) {
	    const int	plen = PBUFLEN ;
	    int		size ;
	    char	*pbuf ;
	    size = (plen+1) ;
	    if ((rs = uc_malloc(size,&pbuf)) >= 0) {
	        rs = pconf_loader(csp,pbuf,plen) ;
	        uc_free(pbuf) ;
	    } /* end if (memory-allocation) */
	} /* end if */

	return rs ;
}
/* end subroutine (pconf_load) */


static int pconf_loader(PCONF *csp,char *pbuf,int plen)
{
	PROGINFO	*pip = csp->pip ;
	PARAMFILE	*pfp = &csp->params ;
	PARAMFILE_CUR	cur ;
	PARAMFILE_ENT	pe ;
	int		rs ;
	int		rs1 ;

	if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {
	    const int	elen = EBUFLEN ;
	    const char	*kp, *vp ;
	    int		pi ;
	    int		kl, vl ;
	    int		el, tl ;
	    int		v ;
	    int		f ;
	    cchar	*pr = pip->pr ;
	    char	tfname[MAXPATHLEN + 1] ;
	    char	ebuf[EBUFLEN + 1] ;

	    while (rs >= 0) {
	        kl = paramfile_enum(pfp,&cur,&pe,pbuf,plen) ;
	        if (kl == SR_NOTFOUND) break ;
	        rs = kl ;
	        if (rs < 0) break ;

	        kp = pe.key ;
	        vp = pe.value ;
	        vl = pe.vlen ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progconf_read: enum k=%t\n",kp,kl) ;
#endif

	        pi = matpstr(params,2,kp,kl) ;
	        if (pi < 0) continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progconf_read: param=%s (%u)\n",
	                params[pi],pi) ;
#endif

	        ebuf[0] = '\0' ;
	        el = 0 ;
	        if (vl > 0) {
	            el = expcook_exp(&pip->cooks,0,ebuf,elen,vp,vl) ;
	            if (el >= 0) ebuf[el] = '\0' ;
	        } /* end if */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("progconf_read: ebuf=>%t<\n",
	                ebuf,strlinelen(ebuf,el,50)) ;
	            debugprintf("progconf_read: switch=%s(%u)\n",
	                params[pi],pi) ;
	        }
#endif

	        if (el < 0)
	            continue ;

	        switch (pi) {

	        case param_mailcheck:
	            if ((el > 0) && (! pip->final.mailcheck)) {
	                if (hasalldig(ebuf,el)) {
	                    if ((rs = cfdecti(ebuf,el,&v)) >= 0) {
	                        if (v >= 0) {
	                            pip->have.mailcheck = TRUE ;
	                            pip->changed.mailcheck = TRUE ;
	                            pip->final.mailcheck = TRUE ;
	                            pip->mailcheck = v ;
	                        }
	                    }
	                }
	            }
	            break ;

	        case param_loglen:
	        case param_logsize:
	            if ((el > 0) && (! pip->final.logsize)) {
	                if (hasalldig(ebuf,el)) {
	                    rs1 = cfdecmfi(ebuf,el,&v) ;
	                    if ((rs1 >= 0) && (v >= 0)) {
	                        pip->have.logsize = TRUE ;
	                        pip->changed.logsize = TRUE ;
	                        pip->final.logsize = TRUE ;
	                        pip->logsize = v ;
	                    }
	                }
	            }
	            break ;

	        case param_cmdkey:
	            if (! pip->final.cmdfname) {
	                pip->have.cmdfname = TRUE ;
	                tl = prsetfname(pr,tfname,ebuf,el,TRUE,
	                    CMDMAPFNAME,pip->searchname,"") ;
	                f = (pip->cmdfname == NULL) ;
	                f = f || (strcmp(pip->cmdfname,tfname) != 0) ;
	                if (f) {
	                    const char	**vpp = &pip->cmdfname ;
	                    pip->final.cmdfname = TRUE ;
	                    pip->changed.cmdfname = TRUE ;
	                    rs = proginfo_setentry(pip,vpp,tfname,tl) ;
	                }
	            }
	            break ;

	        case param_logfile:
	            if (! pip->final.lfname) {
	                pip->have.lfname = TRUE ;
	                tl = prsetfname(pr,tfname,ebuf,el,TRUE,
	                    LOGCNAME,pip->searchname,"") ;
	                f = (pip->lfname == NULL) ;
	                f = f || (strcmp(pip->lfname,tfname) != 0) ;
	                if (f) {
	                    const char	**vpp = &pip->lfname ;
	                    pip->final.lfname = TRUE ;
	                    pip->changed.lfname = TRUE ;
	                    rs = proginfo_setentry(pip,vpp,tfname,tl) ;
	                }
	            }
	            break ;

	        case param_testmsg:
	            if (el > 0) {
	                const char	**vpp = &pip->testmsg ;
	                if (el > 76) el = 76 ;
	                rs = proginfo_setentry(pip,vpp,ebuf,el) ;
	            }
	            break ;

	        case param_auxprog:
	            if (el > 0) {
	                rs = procauxprog(pip,ebuf,el) ;
	            }
	            break ;

	        case param_mbdefault:
	        case param_mbinput:
	        case param_mbspam:
	        case param_mbtrash:
	            if (el > 0) {
	                const char	**vpp = NULL ;
	                switch (pi) {
	                case param_mbdefault:
	                    vpp = &pip->mbname_def ;
	                    break ;
	                case param_mbinput:
	                    vpp = &pip->mbname_in ;
	                    break ;
	                case param_mbspam:
	                    vpp = &pip->mbname_spam ;
	                    break ;
	                case param_mbtrash:
	                    vpp = &pip->mbname_trash ;
	                    break ;
	                } /* end switch */
	                rs = proginfo_setentry(pip,vpp,ebuf,el) ;
	            }
	            break ;

	        } /* end switch */

	        if (rs < 0) break ;
	    } /* end while (enumerating) */

	    paramfile_curend(pfp,&cur) ;
	} /* end if (paramfile-cur) */

	return rs ;
}
/* end subroutine (pconf_loader) */


static int procauxprog(PROGINFO *pip,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		vl = 0 ;
	int		kl ;
	int		pi ;
	const char	*tp ;
	const char	*kp = sp ;
	const char	*vp ;

	if (sl < 0) sl = strlen(sp) ;

	if ((tp = strnchr(sp,sl,CH_FS)) != NULL) {
	    kl = (tp-sp) ;
	    vp = (tp+1) ;
	    vl = ((sp+sl)-vp) ;
	    if (vl > 0) {
	        if ((pi = matstr(prognames,kp,kl)) >= 0) {
	            const char	**vpp = NULL ;
	            switch (pi) {
	            case progname_shell:
	                vpp = &pip->prog_shell ;
	                break ;
	            case progname_getmail:
	                vpp = &pip->prog_getmail ;
	                break ;
	            case progname_mailer:
	                vpp = &pip->prog_mailer ;
	                break ;
	            case progname_editor:
	                vpp = &pip->prog_editor ;
	                break ;
	            case progname_metamail:
	                vpp = &pip->prog_metamail ;
	                break ;
	            case progname_pager:
	                vpp = &pip->prog_pager ;
	                break ;
	            case progname_postspam:
	                vpp = &pip->prog_postspam ;
	                break ;
	            } /* end switch */
	            rs = proginfo_setentry(pip,vpp,vp,vl) ;
	        } /* end if */
	    } /* end if (non-zero value) */
	} /* end if (key-val pair) */

	return rs ;
}
/* end subroutine (procaxprog) */


