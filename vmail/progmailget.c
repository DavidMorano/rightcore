/* progmailget */

/* get mew mail from system mail-spool area */


#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_ACCESS	1		/* use 'u_access(3u)' */


/* revision history:

	= 1994-01-23, David A­D­ Morano
	This module was copied and modified from the VMAIL original.  A variety
	of enhancements were made to prevent it from crashing due to short
	buffers.  No checks were being made about whether a copy into a buffer
	was overflowing!  Of course, nobody likes a program that crashes either
	(including myself).  It was the crashing of this (and other) programs
	that lead me to fixing this crap up in the first place!  I can hardly
	believe that those original PCS writers got away with so much crappy
	programming.  But there you have it!

	= 1996-06-18, David A­D­ Morano
	I did:
		- remove old mail spool locks

	= 1996-07-24, David A­D­ Morano
	I rewrote the "getnewmail" subroutine in part to:
		- lock user's "new" mailbox when fetching new mail
		  NOTE: This has to be removed when real proper
			mailbox handling is implemented.
		- added full binary compatibility for new mail

	= 2007-11-13, David A­D­ Morano
	Oh man!  How long have I been toiling with this thing?  I added the
	ability to grab mail from multiple users.  I also rewrote from the
	original (before I started cleaning up this crap in 1994) much of the
	way that this process takes place.

	= 2014-03-02, David A­D­ Morano
	Yes, it has been twenty years now.  Where has the time gone?  This has
	been substantially rewritten to check for new mail before attempting to
	lock the user's incoming mailbox.  We check all mail-user mail files
	before attempting to start to transfer any new mail to the incoming
	mailbox.

*/

/* Copyright © 1994,1996,2007 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine gets new mail (if any) from the system mail-spool area.

	Synopsis:

	int progmailget(pip,app)
	PROGINFO	*pip ;
	PARAMOPT	*app ;

	Arguments:

	pip		program information pointer
	app		pointer to argument PARAMOPT object

	Returns:

	>=0		successful with count of messages
	<0		cannot be opened or other error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<paramopt.h>
#include	<vecstr.h>
#include	<vechand.h>
#include	<envhelp.h>
#include	<spawnproc.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	O_FLAGS		(O_RDWR | O_CREAT | O_APPEND)

#ifndef	MUINFO
#define	MUINFO		struct muinfo
#endif

#define	UBUFLEN		MAXPATHLEN

#ifndef	TO_LOCK
#define	TO_LOCK		2
#endif


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	lockfile(int,int,offset_t,offset_t,int) ;
extern int	lockend(int,int,int,int) ;
extern int	pcsgetprog(const char *,char *,const char *) ;
extern int	isNotPresent(int) ;

extern int	proglog_printf(PROGINFO *,cchar *,...) ;
extern int	progvmerr_printf(PROGINFO *,cchar *,...) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strdcpy3(char *,int,cchar *,cchar *,cchar *) ;


/* external variables */


/* local structures */

struct muinfo {
	const char	*varmailusers ;
	const char	*progfname ;
	const char	*argz ;
	int		i ;
} ;


/* forward references */

static int	procmailgeter(PROGINFO *,PARAMOPT *) ;
static int	procmailgeteruc(PROGINFO *,PARAMOPT *,vechand *) ;
static int	procmailgeterux(PROGINFO *,vechand *,cchar *,cchar *) ;
static int	procmuc(PROGINFO *,PARAMOPT *,const char *) ;
static int	procmailgeteru(PROGINFO *,vechand *) ;
static int	procmailgeterum(PROGINFO *,vechand *,int,const char *) ;
static int	procmkenvusers(PROGINFO *,vechand *,ENVHELP *,struct muinfo *) ;
static int	procmailgeterumer(PROGINFO *,int,cchar **,struct muinfo *) ;

static int	mbopen(cchar *,mode_t,gid_t) ;


/* local variables */

static cchar	*envbads[] = { /* these are not really necessary */
	"PCSGETMAIL_EF",
	"PCSGETMAIL_MAILUSERS",
	"_",
	"_A0",
	"_EF",
	"_AST_FEATURES",
	"A__z",
	NULL
} ;


/* exported subroutines */


int progmailget(PROGINFO *pip,PARAMOPT *app)
{
	int		rs = SR_OK ;
	int		len = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("progmailget: ent\n") ;
	    debugprintf("progmailget: prog_getmail=%s\n",pip->prog_getmail) ;
	    debugprintf("progmailget: folder=%s\n",pip->folderdname) ;
	    debugprintf("progmailget: mb=%s\n",pip->mbname_in) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: progmailget=%s\n",
	        pip->progname,pip->prog_getmail) ;
	}

	if (pip->f.mailget && (pip->prog_getmail != NULL)) {
	    const char	*folder = pip->folderdname ;
	    const char	*mb = pip->mbname_in ;

	    proglog_printf(pip,"mbin=%s",mb) ;
	    if (pip->debuglevel > 0) {
	        progvmerr_printf(pip,"%s: mbin=%s",pn,mb) ;
	    }
	    if ((mb != NULL) && (mb[0] != '\0')) {
	        char	infname[MAXPATHLEN + 1] ;
	        if ((rs = mkpath2(infname,folder,mb)) >= 0) {
	            struct ustat	sb ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("progmailget: infname=%s\n",infname) ;
#endif
	            if ((rs = uc_stat(infname,&sb)) >= 0) {
	                if (S_ISREG(sb.st_mode)) {
	                    rs = procmailgeter(pip,app) ;
	                    len = rs ;
	                    fmt = "newmail bytes=%u\n" ;
	                    proglog_printf(pip,fmt,len) ;
	                    if (pip->debuglevel > 0) {
	                        fmt = "%s: newmail bytes=%u\n" ;
	                        progvmerr_printf(pip,fmt,pn,len) ;
	                    }
	                } else {
	                    rs = SR_ISDIR ;
	                    fmt = "incoming mailbox not regular (%d)\n" ;
	                    proglog_printf(pip,fmt,rs) ;
	                    fmt = "%s: incoming mailbox not regular (%d)\n" ;
	                    progvmerr_printf(pip,fmt,pn,rs) ;
	                }
	            } else if (isNotPresent(rs)) {
	                fmt = "incoming mailbox not present (%d)\n" ;
	                proglog_printf(pip,fmt,rs) ;
	                if (pip->debuglevel > 0) {
	                    fmt = "%s: incoming mailbox not present (%d)\n" ;
	                    progvmerr_printf(pip,fmt,pn,rs) ;
	                }
	                rs = SR_OK ;
	            }
	        } /* end if (mkpath) */

	    } /* end if (have incoming mailbox) */

	} /* end if (requested action) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmailget: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (progmailget) */


/* local subroutines */


static int procmailgeter(PROGINFO *pip,PARAMOPT *app)
{
	vecstr		*mlp = &pip->mailusers ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmailget/_er: ent\n") ;
#endif

	if ((rs = vecstr_count(mlp)) > 0) {
	    VECHAND		musers ;
	    const int	vo = VECHAND_OSWAP ;
	    if ((rs = vechand_start(&musers,1,vo)) >= 0) {
	        int		i ;
	        int		c = 0 ;
	        const char	*mup = NULL ;

	        for (i = 0 ; (rs1 = vecstr_get(mlp,i,&mup)) >= 0 ; i += 1) {
	            if (mup != NULL) {
#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("progmailget/_er: adding mu=%s\n",mup) ;
#endif
	                c += 1 ;
	                rs = vechand_add(&musers,mup) ;
	            }
	            if (rs < 0) break ;
	        } /* end for */
	        if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;

	        if ((rs >= 0) && (c > 0)) {
	            if ((rs = procmailgeteruc(pip,app,&musers)) > 0) {
	                rs = procmailgeteru(pip,&musers) ;
	                len = rs ;
	            } /* end if (procmailgeteruc) */
	        } /* end if (ok) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progmailget/_er: "
	                "procmailgeteru() rs=%d\n",rs) ;
#endif

	        rs1 = vechand_finish(&musers) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (vechand) */
	} /* end if (have mail-uses) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmailget/_er: ret rs=%d len=%u\n",
	        rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procmailgeter) */


static int procmailgeteruc(PROGINFO *pip,PARAMOPT *app,vechand *mlp)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*mup ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmailget/_eruc: ent\n") ;
#endif

	for (i = 0 ; vechand_get(mlp,i,&mup) >= 0 ; i += 1) {
	    if (mup != NULL) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progmailget/_eruc: mu=%s\n",mup) ;
#endif
	        proglog_printf(pip,"mu=%s",mup) ;
	        if (pip->debuglevel > 0) {
	            progvmerr_printf(pip,"%s: mu=%s",pn,mup) ;
	        }

	        rs = procmuc(pip,app,mup) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3)) {
	            debugprintf("progmailget/_eruc: mu=%s\n",mup) ;
	            debugprintf("progmailget/_eruc: procmuc() rs=%d\n",
	                rs) ;
	        }
#endif
	        c += rs ;
	        if (rs == 0) {
	            vechand_del(mlp,i--) ;
	        }

	    } /* end if (non-null) */
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmailget/_eruc: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailgeteruc) */


static int procmuc(PROGINFO *pip,PARAMOPT *app,const char *mup)
{
	PARAMOPT_CUR	cur ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	const char	*po = PO_MAILDIRS ;
	const char	*ccp ;

	if (pip == NULL) return SR_FAULT ; /* LINT */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmailget/procmuc: ent\n") ;
#endif

	if ((rs = paramopt_curbegin(app,&cur)) >= 0) {
	    USTAT	sb ;
	    char	msfname[MAXPATHLEN+1] ;

	    while (paramopt_enumvalues(app,po,&cur,&ccp) >= 0) {
	        if (ccp != NULL) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progmailget/procmuc: md=%s\n",ccp) ;
#endif

	        if ((rs = mkpath2(msfname,ccp,mup)) >= 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("progmailget/procmuc: mf=%s\n",
	                    msfname) ;
#endif

	            rs1 = uc_stat(msfname,&sb) ;
	            if ((rs1 >= 0) && (! S_ISDIR(sb.st_mode))) {
	                const int	am = (R_OK|W_OK) ;
#if	CF_ACCESS
	                if (sb.st_size > 0) {
	                    rs1 = u_access(msfname,am) ;
	                    if (rs1 >= 0) c += 1 ;
	                }
#else /* CF_ACCESS */
	                if (sb.st_size > 0) {
	                    rs1 = sperm(&pip->id,&sb,am) ;
	                    if (rs1 >= 0) c += 1 ;
	                }
#endif /* CF_ACCESS */
	            }
	        }

		}
		if (rs < 0) break ;
	    } /* end while */

	    paramopt_curend(app,&cur) ;
	} /* end if (cursor) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmailget/procmuc: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmuc) */


static int procmailgeteru(PROGINFO *pip,vechand *mlp)
{
	int		rs ;
	int		len = 0 ;
	char		ibuf[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmailget/_eru: ent mb=%s\n",pip->mbname_in) ;
#endif

	if ((rs = mkpath2(ibuf,pip->folderdname,pip->mbname_in)) >= 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    cchar	*pfn = pip->prog_getmail ;
	    char	pbuf[MAXPATHLEN + 1] ;
	    if ((rs = pcsgetprog(pip->pr,pbuf,pip->prog_getmail)) >= 0) {
	        if (rs > 0) pfn = pbuf ;
		if (pip->debuglevel > 0) {
		    fmt = "%s: pcsgetmail=%s (%d)\n" ;
	    	    bprintf(pip->efp,fmt,pn,pfn,rs) ;
		}

#if	CF_DEBUG
		if (DEBUGLEVEL(3)) {
	    	debugprintf("progmailget/_eru: ibuf=%s\n",ibuf) ;
	    	debugprintf("progmailget/_eru: pfn=%s\n",pfn) ;
		}
#endif

		rs = procmailgeterux(pip,mlp,ibuf,pfn) ;

	    } else if (isNotPresent(rs)) {
		fmt = "pcsgetmail=%s (%d)" ;
	        proglog_printf(pip,fmt,pfn,rs) ;
		rs = SR_OK ;
	    } /* end if (ppcsgetprog) */
	} /* end if (mkpath) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmailget/_eru: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (progmailgeteru) */


static int procmailgeterux(PROGINFO *pip,vechand *mlp,cchar *ifn,cchar *pfn)
{
	const mode_t	om = 0666 ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
	if ((rs = mbopen(ifn,om,pip->gid_mail)) >= 0) {
	    USTAT	sb ;
	    const int	mfd = rs ;
	    if ((rs = u_fstat(mfd,&sb)) >= 0) {
	        if (S_ISREG(sb.st_mode)) {
		    const int	to = TO_LOCK ;
	            if ((rs = lockend(mfd,TRUE,0,to)) >= 0) {
		        offset_t	offstart, offend ;
			int		cmd ;

	                if ((rs = u_seeko(mfd,0L,SEEK_END,&offstart)) >= 0) {

	                    if (rs >= 0) {
	                        rs = procmailgeterum(pip,mlp,mfd,pfn) ;
	                        len = rs ;
	                    }

	                    u_tell(mfd,&offend) ;
	                } /* end if (seek-end) */

	                cmd = F_UNLOCK ;
	                rs1 = lockfile(mfd,cmd,offstart,0L,0) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (lock-file) */
	        } else {
	            rs = SR_ISDIR ;
	        }
	    } /* end if (statted) */
	    rs1 = u_close(mfd) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (opened) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procmailgeterux) */


static int procmailgeterum(pip,mlp,mfd,progfname)
PROGINFO	*pip ;
vechand		*mlp ;
int		mfd ;
const char	*progfname ;
{
	int		rs ;
	int		rs1 ;
	int		cl ;
	int		size = 0 ;
	int		len = 0 ;
#if	CF_DEBUG
#else
	const char	*efnull = STDNULLFNAME ;
#endif
	const char	*ext_mailusers = VARMAILUSERS ;
	const char	*ext_ef = "EF" ;
	const char	*cp ;
	char		*p ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progmailget/_erum: ent\n") ;
#endif

	cl = sfbasename(progfname,-1,&cp) ;

	size += (3*(cl+2)) ;
	size += strlen(ext_ef) ;
	size += strlen(ext_mailusers) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    struct muinfo	mi ;
	    ENVHELP		env ;
	    const char		*argz ;
	    const char		*env_ef ;
	    const char		*env_mailusers ;
	    char		*bp = p ;

	    argz = bp ;
	    bp = (strwcpyuc(bp,cp,cl) + 1) ;
	    env_ef = bp ;
	    bp = strdcpy3(bp,(size-1),argz,"_",ext_ef) + 1 ;
	    env_mailusers = bp ;
	    bp = strdcpy3(bp,(size-1),argz,"_",ext_mailusers) + 1 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
	        debugprintf("progmailget/_erum: env_ef=%s\n",env_ef) ;
	        debugprintf("progmailget/_erum: env_mailusers=%s\n",
	            env_mailusers) ;
	    }
#endif /* CF_DEBUG */

	    memset(&mi,0,sizeof(struct muinfo)) ;
	    mi.varmailusers = env_mailusers ;
	    mi.progfname = progfname ;
	    mi.argz = argz ;

	    if ((rs = envhelp_start(&env,envbads,pip->envv)) >= 0) {

#if	CF_DEBUG
#else
	        rs = envhelp_envset(&env,env_ef,efnull,-1) ;
#endif /* CF_DEBUG */

	        if (rs >= 0) {
	            while ((rs = procmkenvusers(pip,mlp,&env,&mi)) > 0) {
	                const char	**ev ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(5)) {
	                    const char	*ep ;
	                    envhelp_present(&env,env_mailusers,-1,&ep) ;
	                    debugprintf("progmailget/_erum: "
	                        "env_mailusers=%s\n",env_mailusers) ;
	                }
#endif

	                if ((rs = envhelp_getvec(&env,&ev)) >= 0) {
	                    rs = procmailgeterumer(pip,mfd,ev,&mi) ;
	                    len = rs ;
	                }

	                if (rs < 0) break ;
	            } /* end while (env-users) */
	        } /* end if (ok) */

	        rs1 = envhelp_finish(&env) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (envhelp) */

	    uc_free(p) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progmailget/_erum: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procmailgeterum) */


static int procmkenvusers(pip,mlp,ehp,mip)
PROGINFO	*pip ;
vechand		*mlp ;
ENVHELP		*ehp ;
struct muinfo	*mip ;
{
	const int	ulen = UBUFLEN ;
	int		rs = SR_OK ;
	int		c = 0 ;
	char		*ubuf ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progmailget/procmkenvusers: ent\n") ;
#endif

	if (pip == NULL) return SR_FAULT ;

	if ((rs = vechand_count(mlp)) > 0) {
	    const int	size = (ulen+1) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progmailget/procmkenvusers: c=%u\n",rs) ;
#endif
	    if ((rs = uc_malloc(size,&ubuf)) >= 0) {
	        int	ul = 0 ;
	        int	ci = 0 ;
	        cchar	*mup ;

	        while (vechand_get(mlp,mip->i,&mup) >= 0) {
	            char	*bp = (ubuf+ul) ;
	            int		mul = strlen(mup) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("progmailget/procmkenvusers: mu=%s\n",mup) ;
#endif

	            if ((ulen-(ul+1)) < (mul+1)) break ;

	            if (ul > 0) {
	                *bp++ = ((ci+mul+1) <= 76) ? ' ' : '\n' ;
	            }
	            bp = strwcpy(bp,mup,mul) ;

	            ul += (bp-(ubuf+ul)) ;
	            ci = ((ci+mul+1) <= 76) ? (ci+1+mul) : 0 ;

	            c += 1 ;
	            mip->i += 1 ;
	        } /* end while */

	        if ((rs >= 0) && (c > 0)) {
	            rs = envhelp_envset(ehp,mip->varmailusers,ubuf,ul) ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(5)) {
	            debugprintf("progmailget/procmkenvusers: ubuf=%t\n",
	                ubuf,strlinelen(ubuf,ul,40)) ;
	        }
#endif /* CF_DEBUG */

	        uc_free(ubuf) ;
	    } /* end if (m-a) */
	} /* end if (vechand_count) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progmailget/procmkenvusers: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmkenvusers) */


static int procmailgeterumer(pip,mfd,ev,mip)
PROGINFO	*pip ;
int		mfd ;
const char	**ev ;
struct muinfo	*mip ;
{
	SPAWNPROC	disp ;
	pid_t		pid ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		cs ;
	int		ai = 0 ;
	const char	*av[10] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    int	i ;
	    debugprintf("procmailget/_erumer: env¬\n") ;
	    for (i = 0 ; ev[i] != NULL ; i += 1)
	        debugprintf("procmailget/~erumer: e[%03u]»%t\n",i,
	            ev[i],strlinelen(ev[i],-1,50)) ;
	}
#endif /* CF_DEBUG */

	memset(&disp,0,sizeof(SPAWNPROC)) ;
	disp.opts |= SPAWNPROC_OSETPGRP ;
	disp.disp[0] = SPAWNPROC_DCLOSE ;
	disp.disp[1] = SPAWNPROC_DDUP ;
	disp.fd[1] = mfd ;
	disp.disp[2] = SPAWNPROC_DCLOSE ;

	av[ai++] = mip->argz ;
#ifdef	COMMENT
	av[ai++] = "-D=5" ;
	av[ai++] = "-ef" ;
	av[ai++] = "ee" ;
	av[ai++] = "-Q" ;
#endif /* COMMENT */
	av[ai] = NULL ;
	rs1 = spawnproc(&disp,mip->progfname,av,ev) ;
	pid = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmailget: spawnproc() rs=%d\n",rs1) ;
#endif

	if (rs1 >= 0) {
	    rs1 = u_waitpid(pid,&cs,0) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progmailget/_erumer: u_waitpid() rs=%d cs=%u\n",
	            rs1,cs) ;
#endif
	    if ((rs1 >= 0) && (pip->debuglevel > 0)) {
	        const char	*pn = pip->progname ;
	        const char	*spn = "mailget" ;
	        const char	*fmt ;
	        if (WIFEXITED(cs)) {
	            int	ex = WEXITSTATUS(cs) ;
	            fmt = "%s: %s exited ex=%u\n" ;
	            bprintf(pip->efp,fmt,pn,spn,ex) ;
	        } else if (WIFSIGNALED(cs)) {
	            int	sig = WTERMSIG(cs) ;
	            fmt = "%s: %s signaled sig=%u\n" ;
	            bprintf(pip->efp,fmt,pn,spn,sig) ;
	        } else {
	            fmt = "%s: %s did-something\n" ;
	            bprintf(pip->efp,fmt,pn,spn) ;
	        }
#if	CF_DEBUG
	        if (DEBUGLEVEL(3)) {
	            if (WIFEXITED(cs)) {
	                int	ex = WEXITSTATUS(cs) ;
	                fmt = "progmailget/_erumer: %s exited ex=%u\n" ;
	                debugprintf(fmt,spn,ex) ;
	            } else if (WIFSIGNALED(cs)) {
	                int	sig = WTERMSIG(cs) ;
	                fmt = "progmailget/_erumer: %s signaled sig=%u\n" ;
	                debugprintf(fmt,spn,sig) ;
	            } else {
	                fmt = "progmailget/_erumer: %s did-something\n" ;
	                debugprintf(fmt,spn) ;
	            }
	        }
#endif /* CF_DEBUG */
	    }

	} /* end if (spawned) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("procmailget/_erumer: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmailgeterumer) */


static int mbopen(cchar *fname,mode_t om,gid_t gid)
{
	int		rs ;
	int		oflags = O_FLAGS ;
	int		mfd = -1 ;
	rs = uc_open(fname,oflags,om) ;
	mfd = rs ;
	if (isNotPresent(rs)) {
	    oflags = (O_FLAGS | O_CREAT) ;
	    rs = uc_open(fname,oflags,om) ;
	    mfd = rs ;
	    if ((rs >= 0) && (gid > 0)) {
	        u_fchown(mfd,-1,gid) ;
	    }
	} /* end if (created mailbox file) */
	return (rs >= 0) ? mfd : rs ;
}
/* end subroutine (mbopen) */


