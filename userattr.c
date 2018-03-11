/* userattr */

/* access various user-attribute databases */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_UDOMAIN	0		/* call |udomain(3dam)| */


/* revision history:

	= 2001-06-05, David A­D­ Morano
        I wrote this so that both the old UDOMAIN facility and the newer
        user-attribute database capability of Sun Solaris® 2.6 (first that I
        have seen of it) can be included into a single database lookup
        mechanism.

	= 2017-05-01, David A­D­ Morano
        I just noticed that I too out the UDOMAIN function and forgot to make a
        note here of it. Well I removed it a few years ago now but have
        forgotten exactly where it was removed. I removed it because the old
        UDOMAIN data-base was no longer really needed.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object is used to access (lookup keynames) in various
	user-attribute-like databases.  Currently only two databases are
	possibly queried.  These are:

	UDOMAIN
	Solaris® user-attribiute

	The old UDOMAIN database only stored Internet domain names for various
	users.  But the newer Solaris® user-attribute has generalized the idea
	of storing attribute for users (usernames).  So the Solaris®
	user-attribute database can store both the Internet Domain name for a
	user as well as any other desired (made-up) user attributes.

	We always query Solaris® first and then fall-over to the UDOMAIN
	database for a query of an InterNet domain (query keyname 'dn') that
	fail with Solaris®.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<user_attr.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"userattr.h"


/* local defines */

#ifndef	UA_DN
#define	UA_DN		"dn"		/* INET domain-name */
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	udomain(const char *,char *,int,const char *) ;


/* local structures */


/* forward references */

static int	userattr_opensysdb(USERATTR *) ;
static int	userattr_sysdb(USERATTR *,char *,int,const char *) ;

#if	CF_UDOMAIN
static int	userattr_openudomain(USERATTR *) ;
static int	userattr_udomain(USERATTR *,char *,int) ;
#endif /* CF_UDOMAIN */


/* local variables */

#if	CF_UDOMAIN
static const char	*specials[] = {
	UA_DN,
	NULL
} ;
#endif /* CF_UDOMAIN */


/* exported subroutines */


int userattr_open(USERATTR *op,cchar *username)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (username == NULL) return SR_FAULT ;

	if (username[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("userattr_open: u=%s\n",username) ;
#endif

	memset(op,0,sizeof(USERATTR)) ;
	op->username = username ;
	op->magic = USERATTR_MAGIC ;

	return rs ;
}
/* end subroutine (userattr_open) */


int userattr_close(USERATTR *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != USERATTR_MAGIC) return SR_NOTOPEN ;

	if (op->domain != NULL) {
	    rs1 = uc_free(op->domain) ;
	    if (rs >= 0) rs = rs1 ;
	    op->domain = NULL ;
	}

	if (op->uap != NULL) {
	    rs1 = uc_freeuserattr(op->uap) ;
	    if (rs >= 0) rs = rs1 ;
	    op->uap = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (userattr_close) */


int userattr_lookup(USERATTR *op,char *rbuf,int rlen,cchar *keyname)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (keyname == NULL) return SR_FAULT ;

	if (op->magic != USERATTR_MAGIC) return SR_NOTOPEN ;
	if (keyname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("userattr_lookup: keyname=%s\n",keyname) ;
#endif

	rbuf[0] = '\0' ;
	rs = userattr_sysdb(op,rbuf,rlen,keyname) ;

#if	CF_UDOMAIN
	if ((rs == SR_NOTFOUND) && (matstr(specials,keyname,-1) >= 0)) {
	    rs = userattr_udomain(op,rbuf,rlen) ;
	}
#endif /* CF_UDOMAIN */

#if	CF_DEBUGS
	debugprintf("userattr_lookup: ret rs=%d\n",rs) ;
	if ((rs > 0) && (rbuf != NULL))
	debugprintf("userattr_lookup: rbuf=>%s<\n",rbuf) ;
#endif

	return rs ;
}
/* end subroutine (userattr_lookup) */


/* private subroutines */


static int userattr_opensysdb(USERATTR *op)
{
	int		rs = SR_OK ;

	if (! op->init.sysdb) {
	    op->init.sysdb = TRUE ;
	    rs = uc_getusernam(op->username,&op->uap) ;
	    op->have.sysdb = (rs >= 0) ;
	}

	return (rs >= 0) ? op->have.sysdb : rs ;
}
/* end subroutine (userattr_opensysdb) */


#if	CF_UDOMAIN
static int userattr_openudomain(USERATTR *op)
{
	int		rs = SR_OK ;

	if (! op->init.udomain) {
	    op->init.udomain = TRUE ;
	    op->have.udomain = TRUE ;	/* assumption, might change */
	}

	return (rs >= 0) ? op->have.udomain : rs ;
}
/* end subroutine (userattr_openudomain) */
#endif /* CF_UDOMAIN */


static int userattr_sysdb(USERATTR *op,char *rbuf,int rlen,cchar *keyname)
{
	int		rs = SR_OK ;

	if (! op->init.sysdb) {
	    rs = userattr_opensysdb(op) ;
	}

	if (rs >= 0) {
	    if (op->have.sysdb) {
	        cchar	*cp ;
	        if ((rs = uc_kvamatch(op->uap->attr,keyname,&cp)) >= 0) {
		    if (rbuf != NULL) {
		        rs = sncpy1(rbuf,rlen,cp) ;
		    } else {
		        rs = strlen(cp) ;
		    }
	        } /* end if (successful lookup) */
	    } else {
	        rs = SR_NOTFOUND ;
	    } /* end if (searching the system DB) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("userattr_sysdb: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (userattr_sysdb) */


#if	CF_UDOMAIN
static int userattr_udomain(USERATTR *op,char *rbuf,int rlen)
{
	int		rs = SR_OK ;

	if (! op->init.udomain) {
	    rs = userattr_openudomain(op) ;
	}

	if (rs >= 0) {
	    if (op->have.sysdb) {
	        if (op->domain == NULL) {
		    const int	dlen = MAXHOSTNAMELEN ;
	            char	dbuf[MAXHOSTNAMELEN + 1] ;
	            if ((rs = udomain(op->pr,dbuf,dlen,op->username)) >= 0) {
			const char	*cp ;
		        if ((rs = uc_mallocstrw(dbuf,rs,&cp)) >= 0) {
			    op->domain = cp ;
			}
		    }
		}
	        if ((rs >= 0) && (op->domain != NULL)) {
		    if (rbuf != NULL) {
		        rs = sncpy1(rbuf,rlen,op->domain) ;
		    } else {
		        rs = strlen(op->domain) ;
		    }
	        } /* end if (successful lookup) */
	    } else {
	        rs = SR_NOTFOUND ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("userattr_udomain: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (userattr_udomain) */
#endif /* CF_UDOMAIN */


