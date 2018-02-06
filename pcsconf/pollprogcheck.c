/* pollprogcheck */

/* initiate possible polling of PCS activities */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written.

	= 2008-10-97, David A­D­ Morano
	Changed somewhat to fit into the new polling structure.

*/

/* Copyright © 1998,2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is called by PCS programs.  This may initiate an
	invocation of the PCSPOLL program.  Some quickie checks are made
	here first before calling that program in order to reduce the
	number of times that program needs to be invoked unnecessarily.

	Synopsis:

	int pollprogcheck(pr,sn,envv,pcp)
	const char	pr[] ;
	const char	sn[] ;
	const char	*envv[] ;
	PCSCONF		*pcp ;

	Arguments:

	pr		PCS system program root (if available)
	sn		name to use as program search-name
	envv		caller environment
	pcp		pointer to PCSCONF object

	Returns:

	>=0		OK
	<0		some error


*******************************************************************************/


#define	PCSCONF_MASTER		0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<pcsconf.h>
#include	<spawner.h>
#include	<localmisc.h>


/* local defines */

#define	CHECKER		struct checker

#define	PCS_PROGNAME	"pcspoll"
#define	PCS_PROGPOLL	"pcspoll"
#define	PCS_STAMPDNAME	"var/timestamps"
#define	PCS_STAMPNAME	"pcspoll"
#define	PCS_MINCHECK	(5*60)

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	NOFILE
#define	NOFILE		20
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(6 * MAXPATHLEN)
#endif

#ifndef	EX_NOEXEC
#define	EX_NOEXEC	126
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	mkpath3w(char *,const char *,const char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	matkeystr(const char **,char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optvalue(const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	isNotPresent(int) ;

extern int	pcsgetprog(const char *,char *,const char *) ;
extern int	pcsgetprogpath(const char *,char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strnwcpy(char *,int,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */

extern const char	**environ ;


/* local structures */

struct checker_confs {
	const char	*progpoll ;
	const char	*stampdname ;
	const char	*stampname ;
} ;

struct checker {
	VECSTR		stores ;
	const char	*pr ;		/* passed argument */
	const char	*sn ;		/* passed argument */
	const char	**envv ;	/* passed argument */
	PCSCONF		*pcp ;		/* passed argument */
	const char	*progpoll ;
	const char	*stampdname ;
	struct checker_confs	confs ;
	int		intcheck ;	/* interval-check */
} ;


/* forward references */

static int checker_start(CHECKER *,const char *,const char *,const char **,
			PCSCONF *) ;
static int checker_finish(CHECKER *) ;
static int checker_setentry(CHECKER *,const char **,const char *,int) ;
static int checker_getconf(CHECKER *) ;
static int checker_findprog(CHECKER *) ;
static int checker_findprogconf(CHECKER *,char *) ;
static int checker_findprogpr(CHECKER *,char *) ;
static int checker_stampdir(CHECKER *) ;
static int checker_stampdirconf(CHECKER *,char *) ;
static int checker_stampdirpr(CHECKER *,char *) ;
static int checker_stampcheck(CHECKER *) ;
static int checker_stampcheckname(CHECKER *) ;
static int checker_progrun(CHECKER *) ;

static int mksfname(char *,const char *,const char *,const char *) ;


/* local variables */

static const char *confkeys[] = {
	"pcspoll:progpoll",
	"pcspoll:stampdir",
	"pcspoll:stampname",
	"pcspoll:mincheck",
	NULL
} ;

enum confkeys {
	confkey_progpoll,
	confkey_stampdir,
	confkey_stampname,
	confkey_mincheck,
	confkey_overlast
} ;

#ifdef	COMMENT
static const char *envok[] = {
	"HZ",
	"TZ",
	"USERNAME",
	"LOGNAME",
	"HOME",
	"PATH",
	"LANG",
	"NODE",
	"DOMAIN",
	"LOCALDOMAIN",
	"NAME",
	"FULLNAME",
	"MAILNAME",
	"ORGANIZATION",
	"PCS",
	NULL
} ;
#endif /* COMMENT */


/* exported subroutines */


int pollprogcheck(pr,sn,envv,pcp)
const char	pr[] ;
const char	sn[] ;
const char	*envv[] ;
PCSCONF		*pcp ;
{
	CHECKER	c ;

	int	rs ;
	int	n = 0 ;

#if	CF_DEBUGS
	debugprintf("pollprogcheck: pr=%s\n",pr) ;
	debugprintf("pollprogcheck: sn=%s\n",sn) ;
	debugprintf("pollprogcheck: pcp={%p}\n",pcp) ;
#endif

	if (pr == NULL) return SR_FAULT ;
	if (sn == NULL) return SR_FAULT ;

	if ((rs = checker_start(&c,pr,sn,envv,pcp)) >= 0) {

	    if ((rs = checker_getconf(&c)) >= 0) {
	        if ((rs = checker_findprog(&c)) > 0) {
	            const char	*pf = c.progpoll ;
	            if ((pf != NULL) && (pf[0] != '-')) {
	                struct ustat	sb ;
	                if ((u_stat(pf,&sb) >= 0) && S_ISREG(sb.st_mode)) {
	                    IDS	id ;
	                    if ((rs = ids_load(&id)) >= 0) {
	                        int	rs1 ;
	                        if ((rs1 = sperm(&id,&sb,X_OK)) >= 0) {
	                            if ((rs = checker_stampdir(&c)) > 0) {
	                                rs = checker_stampcheck(&c) ;
	                                n += rs ;
#if	CF_DEBUGS
	debugprintf("pollprogcheck: 6 rs=%d n=%u\n",rs) ;
#endif
	                            }
	                        } else if (! isNotPresent(rs1))
	                            rs = rs1 ;
#if	CF_DEBUGS
	debugprintf("pollprogcheck: 7 rs=%d n=%u\n",rs) ;
#endif
	                        ids_release(&id) ;
	                    } /* end if (ids) */
#if	CF_DEBUGS
	debugprintf("pollprogcheck: 8 rs=%d n=%u\n",rs) ;
#endif
	                } /* end if (stat) */
	            }
	        } /* end if (findprog) */
#if	CF_DEBUGS
	debugprintf("pollprogcheck: 10 rs=%d n=%u\n",rs) ;
#endif
	    } /* end if (getconf) */
#if	CF_DEBUGS
	debugprintf("pollprogcheck: 11 rs=%d n=%u\n",rs) ;
#endif

	    checker_finish(&c) ;
	} /* end if (checker) */

#if	CF_DEBUGS
	debugprintf("pollprogcheck: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (pollprogcheck) */


/* local subroutines */


static int checker_start(chp,pr,sn,envv,pcp)
CHECKER		*chp ;
const char	*pr ;
const char	*sn ;
const char	**envv ;
PCSCONF		*pcp ;
{
	int	rs ;

	memset(chp,0,sizeof(CHECKER)) ;
	chp->pr = pr ;
	chp->sn = sn ;
	chp->envv = envv ;
	chp->pcp = pcp ;

	rs = vecstr_start(&chp->stores,1,0) ;

#if	CF_DEBUGS
	debugprintf("checker_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (checker_start) */


static int checker_finish(chp)
CHECKER		*chp ;
{
	int	rs = SR_OK ;
	int	rs1 ;

#if	CF_DEBUGS
	debugprintf("checker_finish: ent\n") ;
#endif

	rs1 = vecstr_finish(&chp->stores) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("checker_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (checker_finish) */


int checker_setentry(chp,epp,vp,vl)
CHECKER		*chp ;
const char	**epp ;
const char	*vp ;
int		vl ;
{
	VECSTR	*slp = &chp->stores ;

	int	rs = SR_OK ;
	int	oi = -1 ;
	int	len = 0 ;

	if (chp == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_INVALID ;

	if (*epp != NULL)
	    oi = vecstr_findaddr(slp,*epp) ;

	if (vp != NULL) {
	    len = strnlen(vp,vl) ;
	    rs = vecstr_store(slp,vp,len,epp) ;
	} else if (epp != NULL)
	    *epp = NULL ;

	if ((rs >= 0) && (oi >= 0))
	    vecstr_del(slp,oi) ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (checker_setentry) */


static int checker_getconf(CHECKER *chp)
{
	PCSCONF		*pcp = chp->pcp ;

	const int	vlen = VBUFLEN ;

	int	rs ;
	int	size ;
	int	n = 0 ;

	char	*vbuf ;

#if	CF_DEBUGS
	debugprintf("checker_getconf: ent pcp={%p}\n",pcp) ;
#endif

	size = (vlen+1) ;
	if ((rs = uc_malloc(size,&vbuf)) >= 0) {
	    int	i ;
	    for (i = 0 ; confkeys[i] != NULL ; i += 1) {
	        const char	*kp = confkeys[i] ;
	        int		vl ;
#if	CF_DEBUGS
	debugprintf("checker_getconf: k=%s\n",kp) ;
#endif
	        if ((vl = pcsconf_fetchone(pcp,kp,-1,vbuf,vlen)) >= 0) {
	            const char	*vp = NULL ;
	            const char	**vpp = NULL ;
	            n += 1 ;
	            switch (i) {
	            case confkey_progpoll:
	                vp = vbuf ;
	                vpp = &chp->confs.progpoll ;
	                break ;
	            case confkey_stampdir:
	                vp = vbuf ;
	                vpp = &chp->confs.stampdname ;
	                break ;
	            case confkey_stampname:
	                vp = vbuf ;
	                vpp = &chp->confs.stampname ;
	                break ;
	            case confkey_mincheck:
	                if (vl > 0) {
	                    int	v ;
	                    rs = cfdecti(vbuf,vl,&v) ;
	                    chp->intcheck = v ;
	                }
	                break ;
	            } /* end if */
#if	CF_DEBUGS
	debugprintf("checker_getconf: switch-out i=%u rs=%d\n",i,rs) ;
#endif
	            if ((rs >= 0) && (vp != NULL)) {
	                rs = checker_setentry(chp,vpp,vp,vl) ;
#if	CF_DEBUGS
	debugprintf("checker_getconf: _setentry() rs=%d\n",rs) ;
#endif
	            }
	        } else if (vl != SR_NOTFOUND)
	            rs = vl ;
#if	CF_DEBUGS
	debugprintf("checker_getconf: pcsconf_fetchone() rs=%d vl=%d\n",rs,vl) ;
#endif
	    } /* end for */
	    uc_free(vbuf) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("checker_getconf: ret rs=%d n=%u\n",rs,n) ;
#endif
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (checker_getconf) */


static int checker_findprog(CHECKER *chp)
{
	int	rs = SR_OK ;
	int	rl = 0 ;

	char	rbuf[MAXPATHLEN+1] = { 0 } ;

#if	CF_DEBUGS
	debugprintf("checker_findprog: ent\n") ;
#endif

	if ((rs = checker_findprogconf(chp,rbuf)) >= 0) {
	    rl = rs ;
	    if ((rbuf[0] == '\0') && (rbuf[0] != '-')) {
	        rs = checker_findprogpr(chp,rbuf) ;
	        rl = rs ;
	    }
	    if ((rs >= 0) && (rl > 0) && (rbuf[0] != '-')) {
	        rs = checker_setentry(chp,&chp->progpoll,rbuf,rl) ;
	    }
	} /* end if */

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (checker_findprog) */


static int checker_findprogconf(CHECKER *chp,char *rbuf)
{
	int	rs = SR_OK ;
	int	rl = 0 ;

	const char	*vp = chp->confs.progpoll ;

	rbuf[0] = '\0' ;
	if ((vp != NULL) && (vp[0] != '\0')) {
	    if ((vp[0] != '/') && (vp[0] != '-')) {
	        rs = mkpath3w(rbuf,chp->pr,"bin",vp,-1) ;
	        rl = rs ;
	    } else {
	        rs = mkpath1w(rbuf,vp,-1) ;
	        rl = rs ;
	    }
	} /* end if (conf-progpoll) */

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (checker_findprogconf) */


static int checker_findprogpr(CHECKER *chp,char *rbuf)
{
	int	rs ;

	const char	*prog = PCS_PROGPOLL ;

	rs = mkpath3(rbuf,chp->pr,"bin",prog) ;

	return rs ;
}
/* end subroutine (checker_findprogpr) */


static int checker_stampdir(CHECKER *chp)
{
	int	rs = SR_OK ;
	int	rl = 0 ;

	char	rbuf[MAXPATHLEN+1] = { 0 } ;

#if	CF_DEBUGS
	debugprintf("checker_stampdir: ent\n") ;
#endif

	if ((rs = checker_stampdirconf(chp,rbuf)) >= 0) {
	    rl = rs ;
	    if ((rbuf[0] == '\0') && (rbuf[0] != '-')) {
	        rs = checker_stampdirpr(chp,rbuf) ;
	        rl = rs ;
	    }
	    if ((rs >= 0) && (rl > 0) && (rbuf[0] != '-')) {
	        rs = checker_setentry(chp,&chp->stampdname,rbuf,rl) ;
	    }
	} /* end if */

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (checker_stampdir) */


static int checker_stampdirconf(CHECKER *chp,char *rbuf)
{
	int		rs = SR_OK ;
	int		rl = 0 ;

	const char	*vp = chp->confs.stampdname ;

	rbuf[0] = '\0' ;
	if ((vp != NULL) && (vp[0] != '\0')) {
	    if ((vp[0] != '/') && (vp[0] != '-')) {
	        rs = mkpath2w(rbuf,chp->pr,vp,-1) ;
	        rl = rs ;
	    } else {
	        rs = mkpath1w(rbuf,vp,-1) ;
	        rl = rs ;
	    }
	} /* end if (conf-stampdname) */

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (checker_stampdirconf) */


static int checker_stampdirpr(CHECKER *chp,char *rbuf)
{
	int	rs ;

	const char	*dname = PCS_STAMPDNAME ;

	rs = mkpath2(rbuf,chp->pr,dname) ;

	return rs ;
}
/* end subroutine (checker_stampdirpr) */


static int checker_stampcheck(CHECKER *chp)
{
	int	rs = SR_OK ;
	int	n = 0 ;

	const char	*dn = chp->stampdname ;

#if	CF_DEBUGS
	debugprintf("checker_stampcheck: ent\n") ;
#endif

	if ((dn != NULL) && (dn[0] != '-')) {
	    struct ustat	sb ;
	    if ((u_stat(dn,&sb) >= 0) && S_ISDIR(sb.st_mode)) {
	        if ((rs = checker_stampcheckname(chp)) > 0) {
	            n = rs ;
	        }
	    } /* end if (stat) */
	} /* end if */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (checker_stampcheck) */


static int checker_stampcheckname(CHECKER *chp)
{
	int	rs = SR_OK ;
	int	n = 0 ;

	const char	*sname = chp->confs.stampname ;

	char	sfname[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	debugprintf("checker_stampcheckname: ent\n") ;
	debugprintf("checker_stampcheckname: sd=%s\n",
	    chp->stampdname) ;
#endif

	if ((sname == NULL) || (sname[0] == '\0'))
	    sname = PCS_STAMPNAME ;

	if ((rs = mksfname(sfname,chp->pr,chp->stampdname,sname)) >= 0) {
	    struct ustat	sb ;
	    int		f_run = FALSE ;
#if	CF_DEBUGS
	    debugprintf("checker_stampcheckname: sf=%s\n",sfname) ;
	    {
	        int	rs1 = u_stat(sfname,&sb) ;
	        debugprintf("checker_stampcheckname: srs=%d\n",rs1) ;
	    }
#endif
	    if (! f_run) {
	        f_run = (u_stat(sfname,&sb) < 0) ;
	    }
	    if (! f_run) {
	        const int	to = MAX(chp->intcheck,PCS_MINCHECK) ;
	        time_t		daytime = time(NULL) ;
	        if (S_ISREG(sb.st_mode))
	            f_run = ((daytime-sb.st_mtime) >= to) ;
#if	CF_DEBUGS
	        {
	            char	timebuf[TIMEBUFLEN+1] ;
	            debugprintf("checker_stampcheckname: to=%d\n",to) ;
	            timestr_logz(daytime,timebuf) ;
	            debugprintf("checker_stampcheckname: dt=%s\n",timebuf) ;
	            timestr_logz(sb.st_mtime,timebuf) ;
	            debugprintf("checker_stampcheckname: mt=%s\n",timebuf) ;
	        }
#endif /* CF_DEBUGS */
	    }
	    if (f_run) {
	        rs = checker_progrun(chp) ;
	        n += rs ;
	    }
	} /* end if (stamp-filename) */

#if	CF_DEBUGS
	debugprintf("checker_stampcheckname: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (checker_stampcheckname) */


static int checker_progrun(CHECKER *chp)
{
	SPAWNER	s ;
	int	rs = SR_OK ;
	int	n = 0 ;

#if	CF_DEBUGS
	debugprintf("checker_progrun: ent\n") ;
#endif

	if (chp == NULL) return SR_FAULT ;

	if (chp->progpoll != NULL) {
	    int		al ;
	    const char	*pf = chp->progpoll ;
	    const char	**ev = chp->envv ;
	    const char	*av[3] ;
	    const char	*ap ;
	    if ((al = sfbasename(pf,-1,&ap)) > 0) {
	        int	i = 0 ;
	        char	argz[MAXNAMELEN+1] ;
	        strnwcpy(argz,MAXNAMELEN,ap,al) ;
	        av[i++] = argz ;
	        av[i++] = NULL ;
	        if ((rs = spawner_start(&s,pf,av,ev)) >= 0) {
	            const char	*pcs = "PCS" ;
	            if (getourenv(ev,pcs) == NULL) {
	                rs = spawner_envset(&s,pcs,chp->pr,-1) ;
	            }
	            if (rs >= 0) {
	                spawner_sigignores(&s) ;
	                spawner_setsid(&s) ;
	                for (i = 0 ; i < 3 ; i += 1)
	                    spawner_fdclose(&s,i) ;
	                if ((rs = spawner_run(&s)) >= 0) {
			    n = rs ;
	                } /* end if (run) */
	            } /* end if */
	            spawner_finish(&s) ;
	        } /* end if (spawner) */
	    } /* end if */
	} /* end if */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (checker_progrun) */


static int mksfname(rbuf,pr,sdname,sname)
char		rbuf[] ;
const char	*pr ;
const char	*sdname ;
const char	*sname ;
{
	int	rs ;

	if (sdname[0] != '/') {
	    rs = mkpath3(rbuf,pr,sdname,sname) ;
	} else
	    rs = mkpath2(rbuf,sdname,sname) ;

	return rs ;
}
/* end subroutine (mksfname) */


