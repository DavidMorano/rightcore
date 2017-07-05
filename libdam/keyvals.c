/* keyvals */

/* key-values file operations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGHDB	0		/* eval HDB performance */
#define	CF_DEBUGFILE	1		/* debug adding entries */
#define	CF_SAFE		1		/* run safer */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This object was originally written.

	- 2004-05-25, David A­D­ Morano
	This subroutine was adopted for use as a general key-value file reader.

*/

/* Copyright © 1998,2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object processes a file containing key-values.


*******************************************************************************/


#define	KEYVALS_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"keyvals.h"


/* local defines */

#define	KEYVALS_KEY	struct keyvals_key
#define	KEYVALS_ENT	struct keyvals_e


/* external subroutines */

extern unsigned int	hashelf(const char *,int) ;

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct keyvals_key {
	const char	*kp ;
	int		kl ;
	int		count ;
} ;

struct keyvals_e {
	KEYVALS_KEY	*kep ;
	const char	*vname ;
	int		vlen ;
	int		fi ;		/* file index */
	int		ki ;		/* key index */
} ;


/* forward references */

static unsigned int	hashkeyval(KEYVALS_ENT *,int) ;

static int	keyvals_keyadd(KEYVALS *,KEYVALS_KEY *,KEYVALS_KEY **) ;
static int	keyvals_keyfins(KEYVALS *) ;
static int	keyvals_keyget(KEYVALS *,const char *,KEYVALS_KEY **) ;
static int	keyvals_keydel(KEYVALS *,int) ;
static int	keyvals_already(KEYVALS *,KEYVALS_ENT *) ;
static int	keyvals_addentry(KEYVALS *,KEYVALS_ENT *) ;
static int	keyvals_entfins(KEYVALS *) ;

static int	key_start(KEYVALS_KEY *,const char *) ;
static int	key_increment(KEYVALS_KEY *) ;
static int	key_decrement(KEYVALS_KEY *) ;
static int	key_finish(KEYVALS_KEY *) ;
static int	key_mat(KEYVALS_KEY *,const char *,int) ;

static int	entry_start(KEYVALS_ENT *,int,int,KEYVALS_KEY *,cchar *,int) ;
static int	entry_matkey(KEYVALS_ENT *,const char *,int) ;
static int	entry_finish(KEYVALS_ENT *) ;

static int	vcmpkey() ;
static int	vcmpentry(KEYVALS_ENT *,KEYVALS_ENT *,int) ;


/* local variables */


/* exported subroutines */


int keyvals_start(op,ndef)
KEYVALS		*op ;
int		ndef ;
{
	int		rs ;
	int		n ;
	int		opts ;
	int		size ;

	if (op == NULL) return SR_FAULT ;

	if (ndef < KEYVALS_DEFENTS) ndef = KEYVALS_DEFENTS ;

	memset(op,0,sizeof(KEYVALS)) ;

	n = (ndef / 10) ;

	size = sizeof(KEYVALS_KEY) ;
	opts = (VECOBJ_OSTATIONARY | VECOBJ_OREUSE) ;
	if ((rs = vecobj_start(&op->keys,size,n,opts)) >= 0) {
	    uint	(*hk)(KEYVALS_ENT *,int) = hashkeyval ;
	    int		(*vkv)() = vcmpentry ;
	    if ((rs = hdb_start(&op->bykeyval,ndef,0,hk,vkv)) >= 0) {
	        if ((rs = hdb_start(&op->bykey,ndef,0,NULL,NULL)) >= 0) {
	            op->magic = KEYVALS_MAGIC ;
	        }
		if (rs < 0)
	    	    hdb_finish(&op->bykeyval) ;
	    } /* end if (hdb_start) */
	    if (rs < 0)
		vecobj_finish(&op->keys) ;
	} /* end if (vecobj_start) */

#if	CF_DEBUGS
	debugprintf("keyvals_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (keyvals_start) */


/* free up the resources occupied by an KEYVALS list object */
int keyvals_finish(op)
KEYVALS		*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != KEYVALS_MAGIC) return SR_NOTOPEN ;

	rs1 = keyvals_entfins(op) ;
	if (rs >= 0) rs = rs1 ;

/* secondary items */

	rs1 = keyvals_keyfins(op) ;
	if (rs >= 0) rs = rs1 ;

/* free up the rest of the main object data */

	rs1 = hdb_finish(&op->bykey) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = hdb_finish(&op->bykeyval) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->keys) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (keyvals_finish) */


/* add a key-value pair */
int keyvals_add(op,fi,kp,vp,vl)
KEYVALS		*op ;
int		fi ;
const char	*kp ;
const char	*vp ;
int		vl ;
{
	KEYVALS_KEY	*kep = NULL ;
	int		rs ;
	int		rs1 ;
	int		ki ;
	int		c_added = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	if (op->magic != KEYVALS_MAGIC) return SR_NOTOPEN ;

	if (*kp == '\0') return SR_INVALID ;

	if (vl < 0) vl = strlen(vp) ;

#if	CF_DEBUGS
	debugprintf("keyvals_add: ent fi=%u\n",fi) ;
	debugprintf("keyvals_add: k=%s\n",kp) ;
#endif

	if ((rs = keyvals_keyget(op,kp,&kep)) >= 0) {
	    KEYVALS_ENT	ve ;
	    int	f = TRUE ;
	    ki = rs ;

#if	CF_DEBUGS && CF_DEBUGFILE
	    debugprintf("keyvals_add: key=%s val=>%t<\n",
	        kep->kp,vp,vl) ;
#endif

	    if ((rs = entry_start(&ve,fi,ki,kep,vp,vl)) >= 0) {

#if	CF_DEBUGS 
	    debugprintf("keyvals_add: 1 k=%s\n",kep->kp) ;
	    debugprintf("keyvals_add: 2 k=%s\n",ve.kep->kp) ;
#endif

	        if ((rs = keyvals_already(op,&ve)) == SR_NOTFOUND) {
	            if ((rs = keyvals_addentry(op,&ve)) >= 0) {
	                f = FALSE ;
	                c_added += 1 ;
	            }
	        } /* end if (new entry) */

	        if (f) {
	            rs1 = entry_finish(&ve) ;
		    if (rs1 != INT_MAX) {
			rs1 = keyvals_keydel(op,rs1) ;
			if (rs >= 0) rs = rs1 ;
		    }
		}

	    } /* end if (entry initialized) */

	} /* end if */

#if	CF_DEBUGS
	debugprintf("keyvals_add: ret rs=%d added=%u\n",
	    rs,c_added) ;
#endif

	return (rs >= 0) ? c_added : rs ;
}
/* end subroutine (keyvals_add) */


int keyvals_count(op)
KEYVALS		*op ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != KEYVALS_MAGIC) return SR_NOTOPEN ;

	rs = hdb_count(&op->bykey) ;

	return rs ;
}
/* end subroutine (keyvals_count) */


/* cursor manipulations */
int keyvals_curbegin(op,curp)
KEYVALS		*op ;
KEYVALS_CUR	*curp ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != KEYVALS_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	rs = hdb_curbegin(&op->bykey,&curp->ec) ;

	return rs ;
}
/* end subroutine (keyvals_curbegin) */


int keyvals_curend(op,curp)
KEYVALS		*op ;
KEYVALS_CUR	*curp ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != KEYVALS_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	rs = hdb_curend(&op->bykey,&curp->ec) ;

	return rs ;
}
/* end subroutine (keyvals_curend) */


/* enumerate a key */
int keyvals_enumkey(op,curp,kpp)
KEYVALS		*op ;
KEYVALS_CUR	*curp ;
const char	**kpp ;
{
	KEYVALS_KEY	*kep = NULL ;
	int		rs = SR_OK ;
	int		oi ;
	int		kl = 0 ;
	const char	*kp = NULL ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != KEYVALS_MAGIC) return SR_NOTOPEN ;

	oi = (curp->i >= 0) ? (curp->i + 1) : 0 ;

/* CONSTCOND */

	while (TRUE) {
	    rs = vecobj_get(&op->keys,oi,&kep) ;
	    if (rs < 0) break ;
	    if (kep != NULL) break ;
	    oi += 1 ;
	} /* end while */

	if (rs >= 0) {
	    kp = kep->kp ;
	    kl = kep->kl ;
	    curp->i = oi ;
	}

	if (kpp != NULL)
	    *kpp = kp ;

#if	CF_DEBUGS
	debugprintf("keyvals_enumkey: ret rs=%d kl=%u\n",rs,kl) ;
#endif

	return (rs >= 0) ? kl : rs ;
}
/* end subroutine (keyvals_enumkey) */


/* enumerate key-value pairs */
int keyvals_enum(op,curp,kpp,vpp)
KEYVALS		*op ;
KEYVALS_CUR	*curp ;
const char	**kpp ;
const char	**vpp ;
{
	KEYVALS_ENT	*ep ;
	HDB_DATUM	key, val ;
	HDB_CUR		cur ;
	int		rs ;
	int		kl = 0 ;
	const char	*kp = NULL ;
	const char	*vp = NULL ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != KEYVALS_MAGIC) return SR_NOTOPEN ;

	cur = curp->ec ;
	if ((rs = hdb_enum(&op->bykey,&cur,&key,&val)) >= 0) {

	    kp = (const char *) key.buf ;
	    kl = key.len ;

	    ep = (KEYVALS_ENT *) val.buf ;
	    vp = ep->vname ;

	    curp->ec = cur ;

	} /* end if (had an entry) */

	if (kpp != NULL)
	    *kpp = kp ;

	if (vpp != NULL)
	    *vpp = vp ;

#if	CF_DEBUGS
	debugprintf("keyvals_enum: ret k=%s\n",kp) ;
	debugprintf("keyvals_enum: ret rs=%d kl=%u\n",rs,kl) ;
#endif

	return (rs >= 0) ? kl : rs ;
}
/* end subroutine (keyvals_enum) */


int keyvals_fetch(op,keybuf,curp,vpp)
KEYVALS		*op ;
const char	keybuf[] ;
KEYVALS_CUR	*curp ;
const char	**vpp ;
{
	KEYVALS_ENT	*ep ;
	HDB_DATUM	key, val ;
	HDB_CUR		cur ;
	int		rs ;
	int		kl ;
	int		vl = 0 ;
	const char	*kp ;
	const char	*vp = NULL ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (keybuf == NULL) return SR_FAULT ;

	if (op->magic != KEYVALS_MAGIC) return SR_NOTOPEN ;

	kp = (const char *) keybuf ;
	if (keybuf[0] == '\0') kp = "default" ;
	kl = strlen(kp) ;

#if	CF_DEBUGS
	debugprintf("keyvals_fetch: key=%s\n",kp) ;
#endif

	key.buf = kp ;
	key.len = kl ;
	cur = curp->ec ;
	if ((rs = hdb_fetch(&op->bykey,key,&cur,&val)) >= 0) {
	    ep = (KEYVALS_ENT *) val.buf ;
	    vp = ep->vname ;
	    vl = ep->vlen ;
	    curp->ec = cur ;
	} /* end if (had an entry) */

	if (vpp != NULL)
	    *vpp = vp ;

#if	CF_DEBUGS
	debugprintf("keyvals_fetch: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (keyvals_fetch) */


/* delete up all entries associated w/ a file */
int keyvals_delset(op,fi)
KEYVALS		*op ;
int		fi ;
{
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != KEYVALS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("keyvals_delset: delete all fi=%d\n",fi) ;
#endif

/* delete all keyvals w/ this file */

	if ((rs = hdb_curbegin(&op->bykey,&cur)) >= 0) {
	    KEYVALS_ENT	*ep ;
	    while (hdb_enum(&op->bykey,&cur,&key,&val) >= 0) {
	        ep = (KEYVALS_ENT *) val.buf ;
	        if ((ep->fi == fi) || (fi < 0)) {
		    c += 1 ;
	            hdb_delcur(&op->bykey,&cur,0) ;
	        } /* end if (found matching entry) */
	    } /* end while (looping through entries) */
	    hdb_curend(&op->bykey,&cur) ;
	} /* end if (cursor) */

	if (rs >= 0) {
	    if ((rs = hdb_curbegin(&op->bykeyval,&cur)) >= 0) {
	        KEYVALS_ENT	*ep ;
	        while (hdb_enum(&op->bykeyval,&cur,&key,&val) >= 0) {
	            ep = (KEYVALS_ENT *) val.buf ;
	            if ((ep->fi == fi) || (fi < 0)) {
	                hdb_delcur(&op->bykeyval,&cur,0) ;
		        entry_finish(ep) ;
		        uc_free(ep) ;
	            } /* end if (key-match) */
	        } /* end while (looping through entries) */
	        hdb_curend(&op->bykeyval,&cur) ;
	    } /* end if (cursor) */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("keyvals_delset: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (keyvals_delset) */


int keyvals_delkey(op,kp,kl)
KEYVALS		*op ;
const char	*kp ;
int		kl ;
{
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;
	int		rs ;
	int		rs1 ;
	int		ki = 0 ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (op->magic != KEYVALS_MAGIC) return SR_NOTOPEN ;

/* delete all keyvals w/ this key */

	if ((rs = hdb_curbegin(&op->bykey,&cur)) >= 0) {
	    key.buf = kp ;
	    key.len = strlen(kp) ;
	    while (hdb_fetch(&op->bykey,key,&cur,&val) >= 0) {
		c += 1 ;
	        hdb_delcur(&op->bykey,&cur,0) ;
	    } /* end while */
	    hdb_curend(&op->bykey,&cur) ;
	} /* end if (cursor) */

	if (rs >= 0) {
	    if ((rs = hdb_curbegin(&op->bykeyval,&cur)) >= 0) {
	        KEYVALS_ENT	*ep ;
	        while (hdb_enum(&op->bykeyval,&cur,&key,&val) >= 0) {
	            ep = (KEYVALS_ENT *) val.buf ;
	            if (entry_matkey(ep,kp,kl) >= 0) {
	                hdb_delcur(&op->bykeyval,&cur,0) ;
		        entry_finish(ep) ;
		        uc_free(ep) ;
	            } /* end if (key-match) */
	        } /* end while (looping through entries) */
	        hdb_curend(&op->bykeyval,&cur) ;
	    } /* end if (cursor) */
	} /* end if */

/* delete the key from the key-store */

	if ((rs >= 0) && (ki >= 0)) {
	    rs = vecobj_del(&op->keys,ki) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (keyvals_delkey) */


/* private subroutines */


static int keyvals_keyadd(op,kep,rpp)
KEYVALS		*op ;
KEYVALS_KEY	*kep ;
KEYVALS_KEY	**rpp ;
{
	int		rs ;
	int		ki = INT_MAX ;

	if ((rs = vecobj_add(&op->keys,kep)) >= 0) {
	    ki = rs ;
	    rs = vecobj_get(&op->keys,ki,rpp) ;
	}

#if	CF_DEBUGS
	debugprintf("keyvals_keyadd: ret rk=%s\n",(*rpp)->kp) ;
	debugprintf("keyvals_keyadd: ret rs=%d ki=%u\n",rs,ki) ;
#endif

	return (rs >= 0) ? ki : rs ;
}
/* end subroutine (keyvals_keyadd) */


static int keyvals_keyfins(op)
KEYVALS		*op ;
{
	KEYVALS_KEY	*kep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(&op->keys,i,&kep) >= 0 ; i += 1) {
	    if (kep == NULL) continue ;
	    rs1 = key_finish(kep) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end for */

	return rs ;
}
/* end subroutine (keyvals_keyfins) */


/* get a pointer to the current key (make it as necessary) */
static int keyvals_keyget(op,keybuf,kpp)
KEYVALS		*op ;
const char	keybuf[] ;
KEYVALS_KEY	**kpp ;
{
	KEYVALS_KEY	ke ;
	KEYVALS_KEY	*kep = NULL ;
	int		rs, rs1 ;
	int		ki = 0 ;

	if ((rs = key_start(&ke,keybuf)) >= 0) {
	    int	f = TRUE ;
	    rs = vecobj_search(&op->keys,&ke,vcmpkey,&kep) ;
	    ki = rs ;
	    if (rs == SR_NOTFOUND) {
		if ((rs = keyvals_keyadd(op,&ke,&kep)) >= 0) {
		    f = FALSE ;
	    	    ki = rs ;
		}
	    } 
	    if (f) key_finish(&ke) ;
	} /* end if (needed to enter new key) */

	if (kpp != NULL) {
	    *kpp = (rs >= 0) ? kep : NULL ;
	}

	return (rs >= 0) ? ki : rs ;
}
/* end subroutine (keyvals_keyget) */


static int keyvals_keydel(op,ki)
KEYVALS		*op ;
int		ki ;
{
	int		rs ;

	rs = vecobj_del(&op->keys,ki) ;

	return rs ;
}
/* end subroutine (keyvals_keydel) */


/* do we have this entry already */
static int keyvals_already(op,nep)
KEYVALS		*op ;
KEYVALS_ENT	*nep ;
{
	HDB_DATUM	key, val ;
	int		rs ;

#if	CF_DEBUGS
	{
	    KEYVALS_KEY	*kep = nep->kep ;
	    debugprintf("keyvals_already: ent key=%s val=>%s<\n",
	        kep->kp,nep->vname) ;
	}
#endif

	key.buf = nep ;
	key.len = sizeof(KEYVALS_ENT) ;
	rs = hdb_fetch(&op->bykeyval,key,NULL,&val) ;

#if	CF_DEBUGS
	debugprintf("keyvals_already: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (keyvals_already) */


/* add an entry to entry list */
static int keyvals_addentry(op,nep)
KEYVALS		*op ;
KEYVALS_ENT	*nep ;
{
	KEYVALS_ENT	*ep ;
	int		rs ;
	int		size ;

#if	CF_DEBUGS
	{
	    KEYVALS_KEY	*kep = nep->kep ;
	    debugprintf("keyvals_addentry: key=%s val=>%s<\n",
	        kep->kp,nep->vname) ;
	}
#endif

	size = sizeof(KEYVALS_ENT) ;
	if ((rs = uc_malloc(size,&ep)) >= 0) {
	    KEYVALS_KEY	*kep ;
	    HDB_DATUM	key, val ;
	    *ep = *nep ; /* copy */
	    kep = ep->kep ;
		key.buf = kep->kp ;
		key.len = kep->kl ;
	    val.buf = ep ;
	    val.len = sizeof(KEYVALS_ENT) ;
	    if ((rs = hdb_store(&op->bykey,key,val)) >= 0) {
		key.buf = ep ;
		key.len = sizeof(KEYVALS_ENT) ;
	        rs = hdb_store(&op->bykeyval,key,val) ;
		if (rs < 0) {
	    	    HDB_CUR	cur ;
	    	    int		rs1 ;
	    	    hdb_curbegin(&op->bykey,&cur) ;
	    	    {
	        	    rs1 = hdb_fetch(&op->bykey,key,&cur,&val) ;
	        	    if (rs1 >= 0)
	            	    hdb_delcur(&op->bykey,&cur,0) ;
	    	    }
	    	    hdb_curend(&op->bykey,&cur) ;
		}
	    }
	    if (rs < 0)
		uc_free(ep) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("keyvals_addentry: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (keyvals_addentry) */


static int keyvals_entfins(KEYVALS *op)
{
	HDB		*elp = &op->bykeyval ;
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;
	int		rs ;
	int		rs1 ;

	if ((rs = hdb_curbegin(elp,&cur)) >= 0) {
	    KEYVALS_ENT	*ep ;
	    while (hdb_enum(elp,&cur,&key,&val) >= 0) {
	        ep = (KEYVALS_ENT *) val.buf ;
	        rs1 = entry_finish(ep) ;
		if (rs >= 0) rs = rs1 ;
	        rs1 = uc_free(ep) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end while */
	    hdb_curend(elp,&cur) ;
	} /* end if (cursor) */

	return rs ;
}
/* end subroutine (keyvals_entfins) */


static int key_start(kep,kname)
KEYVALS_KEY	*kep ;
const char	kname[] ;
{
	int		rs = SR_OK ;
	int		kl = 0 ;

	memset(kep,0,sizeof(KEYVALS_KEY)) ;

	if (kname != NULL) {
	    const char	*kp ;
	    kl = strlen(kname) ;
	    if ((rs = uc_mallocstrw(kname,kl,&kp)) >= 0) {
		kep->kp = kp ;
	        kep->kl = kl ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("key_start: k=%t\n",kep->kp,kep->kl) ;
	debugprintf("key_start: ret rs=%d kl=%u\n",rs,kl) ;
#endif

	return (rs >= 0) ? kl : rs ;
}
/* end subroutine (key_start) */


static int key_increment(kep)
KEYVALS_KEY	*kep ;
{

	if (kep == NULL) return SR_FAULT ;

	kep->count += 1 ;
	return SR_OK ;
}
/* end subroutine (key_increment) */


static int key_decrement(kep)
KEYVALS_KEY	*kep ;
{

	if (kep == NULL) return SR_FAULT ;

	if (kep->count > 0)
	    kep->count -= 1 ;

#ifdef	COMMENT
	if ((kep->count == 0) && (kep->kp != NULL)) {
	    uc_free(kep->kp) ;
	    kep->kp = NULL ;
	    kep->kl = 0 ;
	}
#endif /* COMMENT */

	return kep->count ;
}
/* end subroutine (key_decrement) */


static int key_finish(kep)
KEYVALS_KEY	*kep ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (kep->kp != NULL) {
	    rs1 = uc_free(kep->kp) ;
	    if (rs >= 0) rs = rs1 ;
	    kep->kp = NULL ;
	}

	kep->kl = 0 ;
	kep->count = 0 ;
	return rs ;
}
/* end subroutine (key_finish) */


static int key_mat(kep,kp,kl)
KEYVALS_KEY	*kep ;
const char	*kp ;
int		kl ;
{
	int		f = FALSE ;

	if (kl >= 0) {
	    f = (kep->kl == kl) ;
	    if (f) {
	 	f = (strncmp(kep->kp,kp,kl) == 0) ;
	    }
	} else {
	 	f = (strcmp(kep->kp,kp) == 0) ;
	} /* end if */

	return f ;
}
/* end subroutine (key_mat) */


static int entry_start(ep,fi,ki,kep,vp,vl)
KEYVALS_ENT	*ep ;
int		fi, ki ;
KEYVALS_KEY	*kep ;
const char	*vp ;
int		vl ;
{
	const int	kl = kep->kl ;
	int		rs ;
	const char	*cp ;

#if	CF_DEBUGS
	debugprintf("entry_start: ent k=%s\n",kep->kp) ;
	debugprintf("entry_start: ent v=>%t<\n",vp,vl) ;
#endif

	memset(ep,0,sizeof(KEYVALS_ENT)) ;
	ep->fi = fi ;
	ep->ki = ki ;
	ep->kep = kep ;
	ep->vlen = vl ;

	if ((rs = uc_mallocstrw(vp,vl,&cp)) >= 0) {
	    ep->vname = cp ;
	    key_increment(kep) ;
	}

#if	CF_DEBUGS
	debugprintf("entry_start: ret k=%s\n",kep->kp) ;
	debugprintf("entry_start: ret rs=%d kl=%u\n",rs,kl) ;
#endif

	return (rs >= 0) ? kl : rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(ep)
KEYVALS_ENT	*ep ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		nkeys ;
	int		rc = 0 ;

	if (ep->vname == NULL) return SR_OK ;

	if (ep->kep == NULL) return SR_NOANODE ;

	nkeys = key_decrement(ep->kep) ;

	if (ep->vname != NULL) {
	    rs1 = uc_free(ep->vname) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->vname = NULL ;
	}

	rc = (nkeys == 0) ? ep->ki : INT_MAX ;
	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (entry_finish) */


static int entry_matkey(ep,kp,kl)
KEYVALS_ENT	*ep ;
const char	*kp ;
int		kl ;
{
	int		rs ;
	int		ki ;

	rs = key_mat(ep->kep,kp,kl) ;
	ki = (rs > 0) ? ep->ki : SR_NOTFOUND ;
	return (rs >= 0) ? ki : rs ;
}
/* end subroutine (entry_matkey) */


/* ARGSUSED */
static unsigned int hashkeyval(ep,len)
KEYVALS_ENT	*ep ;
int		len ;
{
	KEYVALS_KEY	*kep = ep->kep ;
	uint		hv = 0 ;

	hv += hashelf(kep->kp,kep->kl) ;

	hv += hashelf(ep->vname,-1) ;

	hv += (ep->fi << 4) ;

	return hv ;
}
/* end subroutine (hashkeyval) */


/* ARGSUSED */
static int vcmpkey(e1pp,e2pp)
KEYVALS_KEY	**e1pp, **e2pp ;
{
	int	rc ;


	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

#if	CF_DEBUGS
	debugprintf("keyvals/vcmpkey: k1(%p)=%s k2(%p)=%s\n",
	    (*e1pp)->kp,
	    (*e1pp)->kp,
	    (*e2pp)->kp,
	    (*e2pp)->kp) ;
#endif

	rc = (*e1pp)->kp[0] - (*e2pp)->kp[0] ;
	if (rc == 0)
	    rc = strcmp((*e1pp)->kp,(*e2pp)->kp) ;

#if	CF_DEBUGS
	debugprintf("keyvals/vcmpkey: ret rc=%d\n",rc) ;
#endif

	return rc ;
}
/* end subroutine (vcmpkey) */


/* ARGSUSED */
static int vcmpentry(e1p,e2p,len)
KEYVALS_ENT	*e1p ;
KEYVALS_ENT	*e2p ;
int		len ;
{
	int	rc = (e1p->fi - e2p->fi) ;

	if (rc == 0)
	    rc = strcmp(e1p->kep->kp,e2p->kep->kp) ;

	if (rc == 0)
	    rc = strcmp(e1p->vname,e2p->vname) ;

	return rc ;
}
/* end subroutine (vcmpentry) */



