/* finduid */

/* find a username given a UID by various means */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_TMPXEARLY	0		/* open TMPX early */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object is used to find a username by a given UID.


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


/* local structures */


/* forward references */

static int	finduid_tmpxopen(FINDUID *) ;
static int	finduid_tmpxclose(FINDUID *) ;


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
	    const char	*cp ;
	    char	pwd[MAXPATHLEN+1] ;
	    char	tmpfname[MAXPATHLEN+1] ;
	    if ((dbfname[0] != '\0') && (dbfname[0] != '/')) {
		if ((rs = getpwd(pwd,MAXPATHLEN)) >= 0) {
		    rs = mkpath2(tmpfname,pwd,dbfname) ;
		    pl = rs ;
		    dbfname = tmpfname ;
		}
	    }
	    if (rs >= 0) {
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
	time_t		ti_create = 0 ;
	int		rs ;
	int		rs1 = SR_OK ;
	int		ul = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;

	if (op->magic != FINDUID_MAGIC) return SR_NOTOPEN ;

	op->s.total += 1 ;

	ubuf[0] = '\0' ;
	if ((rs = ptm_lock(&op->m)) >= 0) {

	    if (! op->open.ut) rs = finduid_tmpxopen(op) ;

	    if (rs >= 0) {
	        struct passwd	pw ;
	        TMPX_CUR	uc ;
	        TMPX_ENT	ue ;
	        const int	pwlen = getbufsize(getbufsize_pw) ;
	        char		*pwbuf ;
		if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	            const int	utype = TMPX_TUSERPROC ;
	            if ((rs = tmpx_curbegin(&op->ut,&uc)) >= 0) {
		        PWCACHE		*pwc = &op->uc ;
	                time_t		ut ;
	                const int	ulen = USERNAMELEN ;
	                char		tubuf[USERNAMELEN+1] ;

	                while (rs >= 0) { /* loop finding latest */
		            rs1 = tmpx_enum(&op->ut,&uc,&ue) ;
		            if (rs1 == SR_NOTFOUND) break ;

		            ut = ue.ut_tv.tv_sec ;
	                    if ((ue.ut_type == utype) && (ut > ti_create)) {
		                strwcpy(tubuf,ue.ut_user,USERNAMELEN) ;
		                rs = pwcache_lookup(pwc,&pw,pwbuf,pwlen,tubuf) ;
		                if (rs >= 0) {
				    if (pw.pw_uid == uid) {
				        ti_create = ut ;
			                ul = strwcpy(ubuf,tubuf,ulen) - ubuf ;
			            }
		                } else if (rs == SR_NOTFOUND) {
				    rs = SR_OK ;
			        }
		            } /* end if (got a user process) */

	                } /* end while (finding latest entry) */
	                if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;

		        rs1 = tmpx_curend(&op->ut,&uc) ;
			if (rs >= 0) rs = rs1 ;
	            } /* end if (TMPX cursor) */
		    rs1 = uc_free(pwbuf) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (m-a) */
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


