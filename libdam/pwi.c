/* pwi */

/* PassWord Index manager */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a small hack for use by the USERINFO built-in command that is
	part of the Korn Shell (KSH).

	This object provides some front-end glue for using the IPASSWD object
	on an IPASSWD database.

	Notes:

	= Searching for a PWI DB file:

	If a PWI DB name is passed to us, we only search for that DB.  If no
	PWI DB is passwed, we search first for a DB with the same name as our
	cluster name (if we have one); otherwise failing that we search for a
	DB with our node name.

	If no DB is present then we either make (a-fresh) the DB given to us by
	name, or we make a DB using our cluster name.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<endianstr.h>
#include	<mkpath.h>
#include	<mkfnamesuf.h>
#include	<char.h>
#include	<realname.h>
#include	<ipasswd.h>
#include	<vecstr.h>
#include	<spawnproc.h>
#include	<ugetpw.h>
#include	<localmisc.h>

#include	"pwi.h"


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#define	GETPW_UID	ugetpw_uid
#else
#define	GETPW_NAME	getpw_name
#define	GETPW_UID	getpw_uid
#endif /* CF_UGETPW */

#define	DBDNAME		"var/pwi"

#ifndef	PASSWDFNAME
#define	PASSWDFNAME	"/etc/passwd"
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#define	TO_FILEMOD	(24 * 3600)

#define	PROG_MKPWI	"mkpwi"

#ifndef	VARHZ
#define	VARHZ		"HZ"
#endif

#ifndef	VARNODE
#define	VARNODE		"NODE"
#endif

#ifndef	VARHOMEDNAME
#define	VARHOMEDNAME	"HOME"
#endif

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#ifndef	VARLOGNAME
#define	VARLOGNAME	"LOGNAME"
#endif

#ifndef	VARTZ
#define	VARTZ		"TZ"
#endif

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif

#undef	VARDBNAME
#define	VARDBNAME	"MKPWI_DBNAME"

#undef	VARPRPWI
#define	VARPRPWI	"MKPWI_PROGRAMROOT"

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#define	PWDESC		struct pwdesc


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy4(char *,int, cchar *,cchar *,cchar *,cchar *) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getnodename(char *,int) ;
extern int	getclustername(const char *,char *,int,const char *) ;
extern int	getgecosname(const char *,int,const char **) ;
extern int	hasuc(const char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* local structures */

struct subinfo_flags {
	uint		dbname:1 ;
} ;

struct subinfo {
	const char	*pr ;
	const char	*dbname ;
	SUBINFO_FL	alloc ;
	const char	*midname ;
} ;

struct pwdesc {
	struct passwd	*pwp ;
	char		*pwbuf ;
	int		pwlen ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,const char *,const char *) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_midname(SUBINFO *) ;
static int	subinfo_mkpwi(SUBINFO *) ;

static int	realname_isextra(REALNAME *,PWDESC *,const char *) ;


/* local variables */

static cchar	*exports[] = {
	VARHZ,
	VARNODE,
	VARHOMEDNAME,
	VARUSERNAME,
	VARLOGNAME,
	VARTZ,
	VARPWD,
	NULL
} ;

/* use fixed locations for security reasons (like we care!) */
static cchar	*prbins[] = {
	"bin",
	"sbin",
	NULL
} ;

static cchar	*extras = "¹²³" ;


/* exported subroutines */


int pwi_open(PWI *op,cchar *pr,cchar *dbname)
{
	SUBINFO	si, *sip = &si ;
	struct ustat	sb, *sbp = &sb ;
	const time_t	daytime = time(NULL) ;
	int		rs ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("pwi_open: dbname=%s\n",dbname) ;
#endif

	if ((rs = subinfo_start(sip,pr,dbname)) >= 0) {
	    if ((rs = subinfo_midname(sip)) >= 0) {
	        time_t		ti_pwi ;
	        const int	to = TO_FILEMOD ;
	        const char	*suf = IPASSWD_SUF ;
	        const char	*endstr = ENDIANSTR ;
	        const char	*midname = sip->midname ;
	        char		fname[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	        debugprintf("pwi_open: midname=%s\n",midname) ;
#endif

	        if ((rs = mkfnamesuf2(fname,midname,suf,endstr)) >= 0) {
	            rs1 = u_stat(fname,sbp) ;

#if	CF_DEBUGS
	            debugprintf("pwi_open: u_stat() rs=%d fname=%s\n",
	                rs1,fname) ;
#endif

	            ti_pwi = sbp->st_mtime ;
	            if ((rs1 >= 0) && (ti_pwi == 0)) {
	                rs1 = SR_NOTFOUND ;
#if	CF_DEBUGS
	                debugprintf("pwi_open: zero size \n") ;
#endif
	            }

	            if ((rs1 >= 0) && ((daytime - ti_pwi) >= to)) {
	                rs1 = SR_NOTFOUND ;
#if	CF_DEBUGS
	                debugprintf("pwi_open: expiration\n") ;
#endif
	            }

	            if (rs1 >= 0) {

	                rs1 = u_stat(PASSWDFNAME,&sb) ;

	                if ((rs1 >= 0) && (sb.st_mtime > ti_pwi)) {
	                    rs1 = SR_NOTFOUND ;
#if	CF_DEBUGS
	                    debugprintf("pwi_open: PASSWD file is younger\n") ;
#endif
	                }

	            } /* end if (checking against system PASSWD file) */

	            if ((rs1 == SR_NOTFOUND) || (rs1 == SR_ACCESS)) {
	                rs = subinfo_mkpwi(sip) ;
	            } else {
	                rs = rs1 ;
		    }

	        } /* end if (mkfnamesuf) */

#if	CF_DEBUGS
	        debugprintf("pwi_open: mid rs=%d\n",rs) ;
	        debugprintf("pwi_open: midname=%s\n",sip->midname) ;
#endif

	        if (rs >= 0) {
	            if ((rs = ipasswd_open(&op->db,midname)) >= 0) {
	                op->magic = PWI_MAGIC ;
	            }
	        }
	    } /* end if (subinfo_midname) */
	    rs1 = subinfo_finish(sip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("pwi_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pwi_open) */


int pwi_close(PWI *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PWI_MAGIC) return SR_NOTOPEN ;

	rs1 = ipasswd_close(&op->db) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (pwi_close) */


int pwi_lookup(PWI *op,char *rbuf,int rlen,cchar *name)
{
	IPASSWD_CUR	cur ;
	REALNAME	rn ;
	const int	nlen = REALNAMELEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		nl ;
	int		c = 0 ;
	int		fopts = 0 ;
	int		ul = 0 ;
	const char	*np ;
	char		nbuf[REALNAMELEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != PWI_MAGIC) return SR_NOTOPEN ;

	if (name[0] == '\0') return SR_INVALID ;

	rbuf[0] = '\0' ;
	if ((rlen >= 0) && (rlen < USERNAMELEN))
	    return SR_OVERFLOW ;

/* conditionally convert to lower case as needed */

	np = name ;
	nl = -1 ;
	if (hasuc(name,-1)) {
	    np = nbuf ;
	    rs = sncpylc(nbuf,nlen,name) ;
	    nl = rs ;
	}

#if	CF_DEBUGS
	debugprintf("pwi_lookup: ent name=%t\n",np,nl) ;
#endif

/* load "name" into REALNAME object for lookup */

	if (rs >= 0) {
	    struct passwd	pw ;
	    const int		pwlen = getbufsize(getbufsize_pw) ;
	    char		*pwbuf ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
		PWDESC	pd ;
		pd.pwp = &pw ;
		pd.pwbuf = pwbuf ;
		pd.pwlen = pwlen ;
	        if ((rs = realname_start(&rn,np,nl)) >= 0) {

	            if ((rs = ipasswd_curbegin(&op->db,&cur)) >= 0) {
		        char	un[USERNAMELEN + 1] ;

	                while (rs >= 0) {
	                    rs1 = ipasswd_fetch(&op->db,&rn,&cur,fopts,un) ;
	                    if (rs1 == SR_NOTFOUND) break ;
			    rs = rs1 ;

#if	CF_DEBUGS
			    debugprintf("pwi_lookup: matched u=%s\n",un) ;
#endif

			    if (rs >= 0) {
			        if ((rs = realname_isextra(&rn,&pd,un)) == 0) {
	                            ul = rs1 ; /* this must be HERE! */
	                            c += 1 ;
	                            rs = sncpy1(rbuf,rlen,un) ;
			        }
			    }

	                } /* end while */

	                rs1 = ipasswd_curend(&op->db,&cur) ;
			if (rs >= 0) rs = rs1 ;
	            } /* end if (cursor) */

	            rs1 = realname_finish(&rn) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (realname) */
		rs1 = uc_free(pwbuf) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (memory-allocation) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("pwi_lookup: mid rs=%d c=%u\n",rs,c) ;
#endif

/* if there was more than one match to the name, punt and issue error */

	if (rs >= 0) {
	    if (c <= 0) {
	        rs = SR_NOTFOUND ;
	    } else if (c > 1) {
	        rs = SR_NOTUNIQ ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("main/lookup: ret rs=%d ul=%u\n",rs,ul) ;
#endif

	return (rs >= 0) ? ul : rs ;
}
/* end subroutine (pwi_lookup) */


/* private subroutines */


static int subinfo_start(SUBINFO *sip,cchar *pr,cchar *dbname)
{

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->pr = pr ;
	sip->dbname = dbname ;

	return SR_OK ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->midname != NULL) {
	    rs1 = uc_free(sip->midname) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->midname = NULL ;
	}

	if (sip->alloc.dbname && (sip->dbname != NULL)) {
	    rs1 = uc_free(sip->dbname) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->dbname = NULL ;
	}

	sip->pr = NULL ;
	return rs ;
}
/* end subroutine (subinfo_finish) */


/* find the inverse-passwd database file */
static int subinfo_midname(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		dblen = -1 ;
	cchar		*dbname = sip->dbname ;
	char		namebuf[MAXNAMELEN + 1] ;

	if ((dbname == NULL) || (dbname[0] == '\0')) {
	    const int	nlen = NODENAMELEN ;
	    char	nbuf[NODENAMELEN + 1] ;
	    char	cbuf[NODENAMELEN + 1] ;
	    if ((rs = getnodename(nbuf,nlen)) >= 0) {
	        const int	rsn = SR_NOTFOUND ;
	        cchar		*nn ;
	        if ((rs = getclustername(sip->pr,cbuf,nlen,nbuf)) >= 0) {
	            nn = cbuf ;
		} else if (rs == rsn) {
		    rs = SR_OK ;
		    nn = nbuf ;
		}
		if (rs >= 0) {
	            rs = mkpath3(namebuf,sip->pr,DBDNAME,nn) ;
	            dbname = namebuf ;
	            dblen = rs ;
		}
	    }
	} /* end if (empty specification) */

#if	CF_DEBUGS
	debugprintf("subinfo_midname: mid dbname=%s\n",dbname) ;
#endif

	if (rs >= 0) {
	    cchar	*cp ;
	    if ((rs = uc_mallocstrw(dbname,dblen,&cp)) >= 0) {
	        sip->midname = cp ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("subinfo_midname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_midname) */


/* make the inverse-passwd database file */
static int subinfo_mkpwi(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		i ;
	const char	*pr = sip->pr ;
	const char	*dbname = sip->midname ;
	const char	*progmkpwi = PROG_MKPWI ;
	char		progfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("pwi/subinfo_mkpwi: dbname=%s\n",dbname) ;
#endif

	if (dbname == NULL)
	    return SR_FAULT ;

	if (dbname[0] == '\0')
	    return SR_INVALID ;

	for (i = 0 ; prbins[i] != NULL ; i += 1) {
	    if ((rs = mkpath3(progfname,pr,prbins[i],progmkpwi)) >= 0) {
	        rs = perm(progfname,-1,-1,NULL,X_OK) ;
	    }
	    if (rs >= 0) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("pwi/subinfo_mkpwi: pr=%s\n",sip->pr) ;
	debugprintf("pwi/subinfo_mkpwi: progfname=%s\n",progfname) ;
	debugprintf("pwi/subinfo_mkpwi: perm() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    SPAWNPROC	ps ;
	    vecstr	envs ;
	    pid_t	cpid ;
	    const int	vo = VECSTR_PNOHOLES ;
	    int		cstat, cex ;

	    if ((rs = vecstr_start(&envs,10,vo)) >= 0) {
		const int	alen = MAXNAMELEN ;
		int		cl ;
	        const char	*av[10] ;
		const char	*cp ;
		const char	*argz = progmkpwi ;
		char		abuf[MAXNAMELEN+1] ;
	        i = 0 ;

		if ((cl = sfbasename(progmkpwi,-1,&cp)) > 0) {
		    argz = abuf ;
		    strwcpyuc(abuf,cp,MIN(cl,alen)) ;
		}

/* setup arguments */

	        av[i++] = argz ;
	        av[i++] = NULL ;

/* setup environment */

	        vecstr_envadd(&envs,VARPRPWI,sip->pr,-1) ;

		if (sip->dbname != NULL)
	            vecstr_envadd(&envs,VARDBNAME,sip->dbname,-1) ;

	        for (i = 0 ; exports[i] != NULL ; i += 1) {
	            if ((cp = getenv(exports[i])) != NULL) {
	                rs = vecstr_envadd(&envs,exports[i],cp,-1) ;
		    }
	            if (rs < 0) break ;
	        } /* end for */

/* go */

	        if (rs >= 0) {
	            cchar	**ev ;
	            if ((rs = vecstr_getvec(&envs,&ev)) >= 0) {

	            memset(&ps,0,sizeof(SPAWNPROC)) ;
	            ps.opts |= SPAWNPROC_OIGNINTR ;
	            ps.opts |= SPAWNPROC_OSETPGRP ;
	            for (i = 0 ; i < 3 ; i += 1) {
			if (i != 2) {
	                    ps.disp[i] = SPAWNPROC_DCLOSE ;
			} else {
	                    ps.disp[i] = SPAWNPROC_DINHERIT ;
			}
	            } /* end for */

	            rs = spawnproc(&ps,progfname,av,ev) ;
	            cpid = rs ;

		    } /* end if (vecstr_getvec) */
	        } /* end if (ok) */
	        vecstr_finish(&envs) ;
	    } /* end if (vecstr) */

	    if (rs >= 0) {

	        cstat = 0 ;
	        rs = 0 ;
	        while (rs == 0) {
	            rs = u_waitpid(cpid,&cstat,0) ;
#if	CF_DEBUGS
	            debugprintf("pwi/subinfo_mkpwi: u_waitpid() rs=%d\n",rs) ;
#endif
	            if (rs == SR_INTR) rs = 0 ;
	        } /* end while */

	        if (rs >= 0) {

	            cex = 0 ;
	            if (WIFSIGNALED(cstat))
	                rs = SR_UNATCH ;	/* protocol not attached */

#if	CF_DEBUGS
	            debugprintf("pwi/subinfo_mkpwi: signaled? rs=%d\n",rs) ;
#endif

	            if ((rs >= 0) && WIFEXITED(cstat)) {

	                cex = WEXITSTATUS(cstat) ;

	                if (cex != 0)
	                    rs = SR_LIBBAD ;

#if	CF_DEBUGS
	                debugprintf("pwi/subinfo_mkpwi: "
				"exited? cex=%d rs=%d\n",cex,rs) ;
#endif

	            } /* end if (wait-exited) */

	        } /* end if (process finished) */

	    } /* end if */

	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("pwi/subinfo_mkpwi: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_mkpwi) */


static int realname_isextra(REALNAME *op,PWDESC *pdp,const char *un)
{
	struct passwd	*pwp = pdp->pwp ;
	const int	pwlen = pdp->pwlen ;
	int		rs ;
	int		f = FALSE ;
	const char	*ln ;
	char		*pwbuf = pdp->pwbuf ;

	if ((rs = realname_getlast(op,&ln)) >= 0) {
	    const int	ll = rs ;
#if	CF_DEBUGS
	    debugprintf("pwi/realname_isextra: ln=>%t<\n",ln,ll) ;
#endif
	    if (strnpbrk(ln,ll,extras) == NULL) {
		if ((rs = GETPW_NAME(pwp,pwbuf,pwlen,un)) > 0) {
		    cchar	*np ;
		    if ((rs = getgecosname(pwp->pw_gecos,-1,&np)) > 0) {
			f = (strnpbrk(np,rs,extras) != NULL) ;
		    }
		} else if (rs == SR_NOTFOUND)
		    rs = SR_OK ;
	    } /* end if (query does not have special extras) */
	} /* end if (realname_getlast) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (realname_isextra) */


