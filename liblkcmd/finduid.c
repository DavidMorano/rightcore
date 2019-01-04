/* finduid */

/* find a username given a UID by various means */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_TMPXEARLY	0		/* open TMPX early */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was originally written.

	= 2018-09-26, David A.D. Morano
	Fixed up string-copy to caller destination to avoid any garbage in the
	TMPX username field if it is smaller than USERNAMELEN in length. Also
	refactored for some pretty-up.

	= 2018-09-27, David A.D. Morano
	Moved two variable declarations closer to their uses.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object is used to find a username by a given UID.

	Algorithm: This is not as smart as we would like, as we pretty much only
	have the system UTMPX database to look into. We try to find a user in
	there with the same UID as the one we are given by our caller and we
	take the latest UTMPX entry as being most authuritative. This is not
	always correct, but it is really the best we can do with what we are
	given (only a UID).

	Notes:
	
	+ Yes. of course, we are thread-safe.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<localmisc.h>

#include	"finduid.h"


/* local defines */

#define	INTUTOPEN	(1*60*60)
#define	INTUTCHECK	(5*60)


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	getpwd(char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,cchar *,int) ;


/* local structures */


/* forward references */

static int	finduid_tmpxopen(FINDUID *) ;
static int	finduid_tmpxclose(FINDUID *) ;
static int	finduid_lookuper(FINDUID *,char *,uid_t) ;


/* local variables */


/* exported subroutines */


int finduid_start(FINDUID *op,cchar *dbfname,int max,int ttl)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (max < FINDUID_DEFMAX)
	    max = FINDUID_DEFMAX ;

	if (ttl < FINDUID_DEFTTL)
	    ttl = FINDUID_DEFTTL ;

	memset(op,0,sizeof(FINDUID)) ;

	if (dbfname != NULL) {
	    int		pl = -1 ;
	    char	tmpfname[MAXPATHLEN+1] ;
	    if ((dbfname[0] != '\0') && (dbfname[0] != '/')) {
		char	pwd[MAXPATHLEN+1] ;
		if ((rs = getpwd(pwd,MAXPATHLEN)) >= 0) {
		    rs = mkpath2(tmpfname,pwd,dbfname) ;
		    pl = rs ;
		    dbfname = tmpfname ;
		}
	    }
	    if (rs >= 0) {
		const char	*cp ;
	        if ((rs = uc_mallocstrw(dbfname,pl,&cp)) >= 0) {
	            op->dbfname = cp ;
		} /* end if (memory-allocation) */
	    }
	} /* end if (dbfname) */

	if (rs >= 0) {
	    if ((rs = ptm_create(&op->m,NULL)) >= 0) {
		if ((rs = pwcache_start(&op->uc,max,ttl)) >= 0) {
#if	CF_TMPXEARLY
		    rs = finduid_tmpxopen(op) ;
#endif
		    if (rs >= 0) {
			op->max = max ;
			op->ttl = ttl ;
			op->magic = FINDUID_MAGIC ;
		    }
		}
		if (rs < 0)
		    ptm_destroy(&op->m) ;
	    } /* end if (ptm-create) */
	    if ((rs < 0) && (op->dbfname != NULL)) {
		uc_free(op->dbfname) ;
		op->dbfname = NULL ;
	    }
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (finduid_start) */


int finduid_finish(FINDUID *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != FINDUID_MAGIC) return SR_NOTOPEN ;

	if (op->open.ut) {
	    op->open.ut = FALSE ;
	    rs1 = finduid_tmpxclose(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = pwcache_finish(&op->uc) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ptm_destroy(&op->m) ;
	if (rs >= 0) rs = rs1 ;

	if (op->dbfname != NULL) {
	    rs1 = uc_free(op->dbfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbfname = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (finduid_finish) */


int finduid_lookup(FINDUID *op,char *ubuf,uid_t uid)
{

	int		rs ;
	int		rs1 = SR_OK ;
	int		ul = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;

	if (op->magic != FINDUID_MAGIC) return SR_NOTOPEN ;

	ubuf[0] = '\0' ;
	if ((rs = ptm_lock(&op->m)) >= 0) {
	    if (! op->open.ut) rs = finduid_tmpxopen(op) ;
	    if (rs >= 0) {
		op->s.total += 1 ;
		rs = finduid_lookuper(op,ubuf,uid) ;
		ul = rs ;
	    } /* end if (ok) */
	    rs1 = ptm_unlock(&op->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (lock) */

/* "not-found" is indicated by a zero-length 'ul' value */

	return (rs >= 0) ? ul : rs ;
}
/* end subroutine (finduid_lookup) */


int finduid_check(FINDUID *op,time_t daytime)
{
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != FINDUID_MAGIC) return SR_NOTOPEN ;

	if (daytime == 0) daytime = time(NULL) ;

	if ((rs = ptm_lock(&op->m)) >= 0) {

	    if (op->open.ut) {
	        if ((daytime - op->ti_utopen) >= INTUTOPEN) {
	            rs = finduid_tmpxclose(op) ;
		}
	    }

	    if ((rs >= 0) && op->open.ut) {
	        if ((daytime - op->ti_utcheck) >= INTUTCHECK) {
	            op->ti_utcheck = daytime ;
	            rs = tmpx_check(&op->ut,daytime) ;
	            f = (rs > 0) ;
	        }
	    }

	    rs1 = ptm_unlock(&op->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (lock) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (finduid_check) */


int finduid_stats(FINDUID *op,FINDUID_STATS *sp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != FINDUID_MAGIC) return SR_NOTOPEN ;

	*sp = op->s ;
	return rs ;
}
/* end subroutine (finduid_stats) */


/* private subroutines */


static int finduid_tmpxopen(FINDUID *op)
{
	int		rs = SR_OK ;

	if (! op->open.ut) {
	    TMPX	*txp = &op->ut ;
	    const int	of = O_RDONLY ;
	    op->ti_utopen = time(NULL) ;
	    if ((rs = tmpx_open(txp,op->dbfname,of)) >= 0) {
	        op->open.ut = TRUE ;
	    }
	}

	return rs ;
}
/* end subroutine (finduid_tmpxopen) */


static int finduid_tmpxclose(FINDUID *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->open.ut) {
	    op->open.ut = FALSE ;
	    rs1 = tmpx_close(&op->ut) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (finduid_tmpxclose) */


/* this is called w/ the object already locked */
static int finduid_lookuper(FINDUID *op,char *ubuf,uid_t uid)
{
	int		rs ;
	int		rs1 ;
	int		ul = 0 ;
	
	    if ((rs = getbufsize(getbufsize_pw)) >= 0) {
	        struct passwd	pw ;
	        const int	pwlen = rs ;
	        char		*pwbuf ;
		if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
		    TMPX_CUR	uc ;
	            TMPX_ENT	ue ;
	            const int	utype = TMPX_TUSERPROC ;
	            if ((rs = tmpx_curbegin(&op->ut,&uc)) >= 0) {
		        PWCACHE		*pwc = &op->uc ;
			time_t		ti_create = 0 ;
	                time_t		ut ;
	                const int	tulen = USERNAMELEN ;
	                char		tubuf[USERNAMELEN+1] ;

	                while ((rs1 = tmpx_enum(&op->ut,&uc,&ue)) >= 0) {
		            ut = ue.ut_tv.tv_sec ;
	                    if ((ue.ut_type == utype) && (ut > ti_create)) {
				const int	tl = TMPX_LUSER ;
		                strdcpy1w(tubuf,tulen,ue.ut_user,tl) ;
		                rs = pwcache_lookup(pwc,&pw,pwbuf,pwlen,tubuf) ;
		                if (rs >= 0) {
				    if (pw.pw_uid == uid) {
				        ti_create = ut ;
			                ul = strwcpy(ubuf,tubuf,tulen) - ubuf ;
			            }
		                } else if (rs == SR_NOTFOUND) {
				    rs = SR_OK ;
			        }
		            } /* end if (got a user process) */
			    if (rs < 0) break ;
	                } /* end while (finding latest entry) */
	                if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;

		        rs1 = tmpx_curend(&op->ut,&uc) ;
			if (rs >= 0) rs = rs1 ;
	            } /* end if (TMPX cursor) */
		    rs1 = uc_free(pwbuf) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (m-a-f) */
	    } /* end if (getbufsize) */
	
	return (rs >= 0) ? ul : rs ;
}
/* end subroutine (finduid_lookuper) */

