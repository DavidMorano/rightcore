/* lookaddr */

/* Addres-Look-List */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 2000-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object provider address look-up for both the system and user
        white-lists and black-lists.

	Usage:

	int lookaddr_usercheck(op,up,ema,f_spam)
	LOOKADDR	*op ;
	LOOKADDR_USER	*up ;
	cchar		*ema ;
	int		f_spam ;

	Arguments:

	op		object pointer
	up		user-cursor pointer
	ema		e-mail-address
	f_spam		spam-flag:
				1=spam
				0=not_spam

	Returns:

	f_spam		spam-flag:
				1=spam
				0=not_spam
				<0=error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>

#include	"lookaddr.h"


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */
#undef	COMMENT

#define	LISTDNAME	"etc/mail"
#define	WLFNAME		"whitelist"
#define	BLFNAME		"blacklist"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	vecstr_envadd(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
extern int	hasuc(const char *,int) ;
extern int	isprintlatin(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int lookaddr_loadvars(LOOKADDR *,cchar *,cchar *) ;
static int lookaddr_swl(LOOKADDR *,cchar *) ;
static int lookaddr_sbl(LOOKADDR *,cchar *) ;
static int lookaddr_uwl(LOOKADDR *,LOOKADDR_USER *,cchar *) ;
static int lookaddr_ubl(LOOKADDR *,LOOKADDR_USER *,cchar *) ;


/* local variables */

/* addrlist file search (for system lists) */
static const char	*sched2[] = {
	    "%p/etc/%n/%n.%f",
	    "%p/etc/%n/%f",
	    "%p/etc/%n.%f",
	    "%p/etc/mail/%n.%f",
	    "%p/etc/mail/%f",
	    "%p/etc/mail.%f",
	    "%p/etc/%f",
	    NULL
} ;

/* addrlist file search (for local-user lists) */
static const char	*sched3[] = {
	    "%h/etc/%n/%n.%f",
	    "%h/etc/%n/%f",
	    "%h/etc/%n.%f",
	    "%h/etc/mail/%n.%f",
	    "%h/etc/mail/%f",
	    "%h/etc/mail.%f",
	    "%h/etc/%f",
	    NULL
} ;


/* exported subroutines */


int lookaddr_start(LOOKADDR *op,cchar *pr,cchar *sn)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;
	if (sn == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("lookaddr_start: ent pr=%s\n",pr) ;
#endif

	memset(op,0,sizeof(LOOKADDR)) ;

	if ((rs = vecstr_start(&op->sv,2,0)) >= 0) {
	    if ((rs = lookaddr_loadvars(op,pr,sn)) >= 0) {
	        op->magic = LOOKADDR_MAGIC ;
	    }
	    if (rs < 0)
	        vecstr_finish(&op->sv) ;
	} /* end if (vecstr_start) */

#if	CF_DEBUGS
	debugprintf("lookaddr_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (lookaddr_start) */


int lookaddr_finish(LOOKADDR *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("lookaddr_finish: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LOOKADDR_MAGIC) return SR_NOTOPEN ;

	if (op->open.swl) {
	    rs1 = whitelist_close(&op->swl) ;
	    if (rs >= 0) rs = rs1 ;
	    op->open.swl = FALSE ;
	}

	if (op->open.sbl) {
	    rs1 = whitelist_close(&op->sbl) ;
	    if (rs >= 0) rs = rs1 ;
	    op->open.sbl = FALSE ;
	}

	rs1 = vecstr_finish(&op->sv) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("lookaddr_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (lookaddr_finish) */


int lookaddr_userbegin(LOOKADDR *op,LOOKADDR_USER *up,cchar *un)
{
	const int	hlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	char		hbuf[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	debugprintf("lookaddr_userbegin: ent\n") ;
	debugprintf("lookaddr_userbegin: un=%s\n",un) ;
#endif

	if (op == NULL) return SR_FAULT ;
#if	CF_DEBUGS
	debugprintf("lookaddr_userbegin: 1\n") ;
#endif
	if (up == NULL) return SR_FAULT ;
#if	CF_DEBUGS
	debugprintf("lookaddr_userbegin: 2\n") ;
#endif
	if (un == NULL) return SR_FAULT ;
#if	CF_DEBUGS
	debugprintf("lookaddr_userbegin: 3\n") ;
#endif

	if (op->magic != LOOKADDR_MAGIC) return SR_NOTOPEN ;
#if	CF_DEBUGS
	debugprintf("lookaddr_userbegin: 4\n") ;
#endif

	memset(up,0,sizeof(LOOKADDR_USER)) ;

	if ((rs = getuserhome(hbuf,hlen,un)) >= 0) {
	    struct ustat	sb ;
	    const int	hl = rs ;
	    if ((rs = uc_stat(hbuf,&sb)) >= 0) {
	        if (S_ISDIR(sb.st_mode)) {
	            if ((rs = perm(hbuf,-1,-1,NULL,X_OK)) >= 0) {
	                cchar	*cp ;
	                if ((rs = uc_mallocstrw(hbuf,hl,&cp)) >= 0) {
	                    up->dname = cp ;
#if	CF_DEBUGS
	                    debugprintf("lookaddr_userbegin: home=%s\n",up->dname) ;
#endif
	                }
	            } else if (isNotPresent(rs))
	                rs = SR_OK ;
	        } else
	            rs = SR_NOTDIR ;
	    } else if (isNotPresent(rs))
	        rs = SR_OK ;
	} else if (isNotPresent(rs))
	    rs = SR_OK ;

	if (rs >= 0) up->magic = LOOKADDR_MAGIC ;

#if	CF_DEBUGS
	debugprintf("lookaddr_userbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (lookaddr_userbegin) */


int lookaddr_userend(LOOKADDR *op,LOOKADDR_USER *up)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (up == NULL) return SR_FAULT ;

	if (op->magic != LOOKADDR_MAGIC) return SR_NOTOPEN ;
	if (up->magic != LOOKADDR_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("lookaddr_userend: ent\n") ;
#endif

	if (up->open.uwl) {
	    rs1 = whitelist_close(&up->uwl) ;
	    if (rs >= 0) rs = rs1 ;
	    up->open.uwl = FALSE ;
	}

	if (up->open.ubl) {
	    rs1 = whitelist_close(&up->ubl) ;
	    if (rs >= 0) rs = rs1 ;
	    up->open.ubl = FALSE ;
	}

	if (up->dname != NULL) {
	    rs1 = uc_free(up->dname) ;
	    if (rs >= 0) rs = rs1 ;
	    up->dname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("lookaddr_userend: ret rs=%d\n",rs) ;
#endif

	up->magic = 0 ;
	return rs ;
}
/* end subroutine (lookaddr_userend) */


/* result: 0=ok, 1=bad */
int lookaddr_usercheck(LOOKADDR *op,LOOKADDR_USER *up,cchar *ema,int f)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (up == NULL) return SR_FAULT ;
	if (ema == NULL) return SR_FAULT ;

	if (op->magic != LOOKADDR_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("lookaddr_usercheck: ent a=%s f=%u\n",ema,f) ;
#endif

	if (f) {
	    if ((rs = lookaddr_swl(op,ema)) == 0) {
	        if ((rs = lookaddr_uwl(op,up,ema)) > 0) {
	            f = FALSE ;
	        }
	    } else if (rs > 0) {
	        f = FALSE ;
	    }
	} else {
	    if ((rs = lookaddr_sbl(op,ema)) > 0) {
#if	CF_DEBUGS
	        debugprintf("lookaddr_usercheck: 0-1\n") ;
#endif
	        f = TRUE ;
	        if ((rs = lookaddr_swl(op,ema)) == 0) {
	            if ((rs = lookaddr_uwl(op,up,ema)) > 0) {
	                f = FALSE ;
	            }
	        } else if (rs > 0) {
	            f = FALSE ;
	        }
	    } else {
#if	CF_DEBUGS
	        debugprintf("lookaddr_usercheck: 0-0\n") ;
#endif
	        if ((rs = lookaddr_ubl(op,up,ema)) > 0) {
#if	CF_DEBUGS
	            debugprintf("lookaddr_usercheck: 0-0-1\n") ;
#endif
	            f = TRUE ;
	            if ((rs = lookaddr_uwl(op,up,ema)) > 0) {
#if	CF_DEBUGS
	                debugprintf("lookaddr_usercheck: 0-0-1-1\n") ;
#endif
	                f = FALSE ;
	            }
	        }
#if	CF_DEBUGS
	        debugprintf("lookaddr_usercheck: 0-0-out rs=%d\n",rs) ;
#endif
	    }
	} /* end if (spam or not) */

#if	CF_DEBUGS
	debugprintf("lookaddr_usercheck: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (lookaddr_usercheck) */


/* private subroutines */


static int lookaddr_loadvars(LOOKADDR *op,cchar *pr,cchar *sn)
{
	int		rs ;
	if ((rs = vecstr_envset(&op->sv,"p",pr,-1)) >= 0) {
	    rs = vecstr_envset(&op->sv,"n",sn,-1) ;
	}
	return rs ;
}
/* end subroutine (lookaddr_loadvars) */


static int lookaddr_swl(LOOKADDR *op,cchar *ema)
{
	int		rs = SR_OK ;
#if	CF_DEBUGS
	debugprintf("lookaddr_swl: ent a=%s\n",ema) ;
#endif
	if (! op->init.swl) {
	    VECSTR	*svp = &op->sv ;
	    const int	tlen = MAXPATHLEN ;
	    cchar	*fn = WLFNAME ;
	    char	tbuf[MAXPATHLEN+1] ;
	    op->init.swl = TRUE ;
	    if ((rs = permsched(sched2,svp,tbuf,tlen,fn,R_OK)) >= 0) {
#if	CF_DEBUGS
	        debugprintf("lookaddr_swl: fn=%s\n",tbuf) ;
#endif
	        if ((rs = whitelist_open(&op->swl,tbuf)) >= 0) {
	            rs = SR_OK ;
	            op->open.swl = TRUE ;
	        }
	    } else if (isNotPresent(rs))
	        rs = SR_OK ;
	} /* end if (init-needed) */
	if ((rs >= 0) && op->open.swl) {
	    if ((rs = whitelist_prematch(&op->swl,ema)) > 0) {
	        rs = 1 ;
	    }
	}
#if	CF_DEBUGS
	debugprintf("lookaddr_swl: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (lookaddr_swl) */


static int lookaddr_sbl(LOOKADDR *op,cchar *ema)
{
	int		rs = SR_OK ;
#if	CF_DEBUGS
	debugprintf("lookaddr_sbl: ent a=%s\n",ema) ;
#endif
	if (! op->init.sbl) {
	    VECSTR	*svp = &op->sv ;
	    const int	tlen = MAXPATHLEN ;
	    cchar	*fn = BLFNAME ;
	    char	tbuf[MAXPATHLEN+1] ;
	    op->init.sbl = TRUE ;
	    if ((rs = permsched(sched2,svp,tbuf,tlen,fn,R_OK)) >= 0) {
#if	CF_DEBUGS
	        debugprintf("lookaddr_sbl: fn=%s\n",tbuf) ;
#endif
	        if ((rs = whitelist_open(&op->sbl,tbuf)) >= 0) {
	            rs = SR_OK ;
	            op->open.sbl = TRUE ;
	        }
	    } else if (isNotPresent(rs))
	        rs = SR_OK ;
	} /* end if (init-needed) */
	if ((rs >= 0) && op->open.sbl) {
	    if ((rs = whitelist_prematch(&op->sbl,ema)) > 0) {
	        rs = 1 ;
	    }
	}
#if	CF_DEBUGS
	debugprintf("lookaddr_sbl: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (lookaddr_sbl) */


static int lookaddr_uwl(LOOKADDR *op,LOOKADDR_USER *up,cchar *ema)
{
	int		rs = SR_OK ;
#if	CF_DEBUGS
	debugprintf("lookaddr_uwl: ent a=%s\n",ema) ;
#endif
	if (up->dname != NULL) {
	    if (! up->init.uwl) {
	        VECSTR		*svp = &op->sv ;
	        const int	tlen = MAXPATHLEN ;
	        cchar		*fn = WLFNAME ;
	        char		tbuf[MAXPATHLEN+1] ;
	        up->init.uwl = TRUE ;
	        if ((rs = vecstr_envset(svp,"h",up->dname,-1)) >= 0) {
	            if ((rs = permsched(sched3,svp,tbuf,tlen,fn,R_OK)) >= 0) {
#if	CF_DEBUGS
	                debugprintf("lookaddr_uwl: fn=%s\n",tbuf) ;
#endif
	                if ((rs = whitelist_open(&up->uwl,tbuf)) >= 0) {
	                    rs = SR_OK ;
	                    up->open.uwl = TRUE ;
	                }
	            } else if (isNotPresent(rs))
	                rs = SR_OK ;
	        } /* end if (vecstr_envset) */
	    } /* end if (init-needed) */
	    if ((rs >= 0) && up->open.uwl) {
	        if ((rs = whitelist_prematch(&up->uwl,ema)) > 0) {
	            rs = 1 ;
	        }
	    }
	} /* end if (have-real-user) */
#if	CF_DEBUGS
	debugprintf("lookaddr_uwl: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (lookaddr_uwl) */


static int lookaddr_ubl(LOOKADDR *op,LOOKADDR_USER *up,cchar *ema)
{
	int		rs = SR_OK ;
#if	CF_DEBUGS
	debugprintf("lookaddr_ubl: ent a=%s\n",ema) ;
#endif
	if (up->dname != NULL) {
	    if (! up->init.ubl) {
	        VECSTR		*svp = &op->sv ;
	        const int	tlen = MAXPATHLEN ;
	        cchar		*fn = BLFNAME ;
	        char		tbuf[MAXPATHLEN+1] ;
	        up->init.ubl = TRUE ;
	        if ((rs = vecstr_envset(svp,"h",up->dname,-1)) >= 0) {
	            if ((rs = permsched(sched3,svp,tbuf,tlen,fn,R_OK)) >= 0) {
#if	CF_DEBUGS
	                debugprintf("lookaddr_ubl: fn=%s\n",tbuf) ;
#endif
	                if ((rs = whitelist_open(&up->ubl,tbuf)) >= 0) {
	                    rs = SR_OK ;
	                    up->open.ubl = TRUE ;
	                }
	            } else if (isNotPresent(rs))
	                rs = SR_OK ;
	        } /* end if (vecstr_envset) */
#if	CF_DEBUGS
	        debugprintf("lookaddr_ubl: init-out rs=%d\n",rs) ;
#endif
	    } /* end if (init-needed) */
#if	CF_DEBUGS
	    debugprintf("lookaddr_ubl: mid rs=%d\n",rs) ;
#endif
	    if ((rs >= 0) && up->open.ubl) {
#if	CF_DEBUGS
	        debugprintf("lookaddr_ubl: prematch() \n") ;
#endif
	        if ((rs = whitelist_prematch(&up->ubl,ema)) > 0) {
	            rs = 1 ;
	        }
	    }
	} /* end if (have-real-user) */
#if	CF_DEBUGS
	debugprintf("lookaddr_ubl: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (lookaddr_ubl) */


