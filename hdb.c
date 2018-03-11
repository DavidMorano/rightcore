/* hdb */

/* general-purpose in-core hashing */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debugging print-outs */
#define	CF_DEBUGHASH	0		/* debug hash-function */
#define	CF_DEBUGDUMP	1		/* debug dump */
#define	CF_DEBUGMATCH	1		/* debug match */
#define	CF_ELFHASH	1		/* use ELF hash as default? */
#define	CF_AUDITENTRY	1		/* audit entries */
#define	CF_EXTEND	1		/* auto-extend hash-table */


/* revision history:

	= 1998-05-01, David A­D­ Morano
        I wrote this from stratch due to frustration with other types of these
        things. Why I have to wrote something like this just goes to show how
        little strctured containers are used now-a-days.

	= 2003-04-19, David A­D­ Morano
        I grabbed the previous version and modified it to use a look-aside
        buffer for the entry-node structures. This is used instead of
        'malloc(3c)' to get and release node strutures. The look-aside manager
        uses 'malloc(3c)' to extend the look-aside entries. Entries released
        back to the look-aside manager are not given back to 'malloc(3c)' (via
        'free(3c)' at the present time -- not so easy to do). Anyway, we get
        about an average of 27% to 29% speedup on a combination of allocating
        and freeing entries.

*/

/* Copyright © 1998,2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This package provides a reasonably general-purpose hashing object for
        use in cases calling for in-core hashing.

        One of the features sorely missed in other packages (code objects) like
        this is that this package can store multiple key-data pairs with the
        same keys.

        The "walk" type method, which I never liked in other packages, is
        deprecated in favor of the "enumerate" ('hdb_enum') type method instead.
        There are two types of "delete" methods. One deletes all entries with
        the same key (possibly useful for old timers who never knew what to do
        with muliple identical keys in the first place), and the other function
        deletes the entry under the current cursor.

        Oh, the "enumerate" and "fetch" functions require the concept of a
        CURSOR, which is, roughly, a pointer type of a thing to the current
        entry.

        In terms of implementation, these routines are just as inelegant, or
        worse, than the DBM, HDBM, or other type of implementations; namely, I
        implemented hashing with hash buckets (using a single table) with
        chained entries dangling. An additional dimension of chains may dangle
        from each of the previous described entries to facilitate muliple
        entries with the same key.

	Important user note:

        This package does not store the user supplied data into its own storage
        space. Only pointers from within the user supplied data are stored. For
        this reason, it is very important for the user to not free up any
        storage locations that are still linked inside of this database. Havoc
        will result! Also, when freeing entries in this database, the user's
        data is NOT freed! Further, the DB storage location(s) that were used to
        store the user's data IS freed! This means that the user should make
        pointers to or copies of any data that they have (if they ever want to
        access it again) BEFORE they delete the corresponding entry from this
        database! Got it? Do it!

	Implementation note:

        Whew! There may be more hairy code that what is in this object, but I
        don't know where it is! What am I trying to say? There may be bugs in
        this mess! It has held up solidly against testing and use already but
        there could still be some corners that are not quite right. Both the
        core of 'enumerate' and 'fetch' are very hairy. Fetching is especially
        hairy. Some of the hairyness is due to some efforts (minor) to not
        repeat executing some code unnecessarily. This appoach may have been a
        mistake given the complexity of this whole mess already. Maybe there is
        something to be said for simple data strctures after all! Enjoy!

	Synopsis:
	int hdb_start(op,n,at,hashfunc,cmpfunc)
	HDB		*op ;
	int		n ;
	int		at ;
	unsigned	(*hashfunc)() ;
	int		(*cmpfunc)() ;

	Arguments:
	op		object pointer
	n		starting number of entries (estimated)
	at		allocation-type:
				0=regular
				1=lookaside
	hashfunc	the hash function
	cmpfunc		the key-comparison function

	Returns:
	<0		error
	>=0		OK


*******************************************************************************/


#define	HDB_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<lookaside.h>
#include	<localmisc.h>

#include	"hdb.h"


/* local defines */

#define HDB_BADMAGIC(op)	((op)->magic != HDB_MAGIC)
#define	HDB_PRBUFLEN		20
#define HDB_KE			struct hdb_ke
#define HDB_VE			struct hdb_ve

#define	ENTRYINFO		struct entryinfo
#define	FETCHCUR		struct fetchcursor


/* external subroutines */

extern int	strnnlen(const char *,int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	mkhexstr(const char *,int,const void *,int) ;
extern int	isprintlatin(int) ;
#endif


/* local structures */

#ifdef	COMMENT

struct hdb_ke {
	struct hdb_ke	*next ;		/* next in hash chain */
	struct hdb_ve	*same ;		/* next w/ same key */
	uint		hv ;		/* hash-value of key */
} ;

struct hdb_ve {
	struct hdb_ve	*same ;		/* next w/ same key */
	HDB_DATUM	key ;
	HDB_DATUM	val ;
} ;

#endif /* COMMENT */

struct fetchcursor {
	uint		(*hfp)(const void *,int) ;
	HDB_CUR		*curp ;
	uint		hv ;
	int		htl ;
	int		f_ikeyed:1 ;
	int		f_jkeyed:1 ;
} ;

struct entryinfo {
	HDB_ENT		*pjep ;
	HDB_ENT		*pkep ;
	HDB_ENT		*ep ;
} ;


/* forward references */

static uint	defhashfunc(const void *,int) ;

int		hdb_delall(HDB *) ;
int		hdb_fetchrec(HDB *,HDB_DATUM,HDB_CUR *,
			HDB_DATUM *,HDB_DATUM *) ;
int		hdb_enum(HDB *,HDB_CUR *,HDB_DATUM *,HDB_DATUM *) ;

static int	hdb_entnew(HDB *,HDB_ENT **) ;
static int	hdb_entdel(HDB *,HDB_ENT *) ;

static int	hdb_findkeyentry(HDB *,FETCHCUR *,HDB_DATUM *,HDB_ENT **) ;
static int	hdb_get(HDB *,int,HDB_DATUM,HDB_CUR *,HDB_DATUM *,HDB_DATUM *) ;

static int	hdb_ext(HDB *) ;
static int	hdb_extinsert(HDB *,HDB_ENT **,int,HDB_ENT *) ;
static int	hdb_extkeyfree(HDB *,HDB_ENT *) ;
static int	hdb_getentry(HDB *,ENTRYINFO *,HDB_CUR *) ;

static int	entry_load(HDB_ENT *,uint,HDB_DATUM *,HDB_DATUM *) ;
static int	entry_match(HDB_ENT *,int (*)(const void *,const void *,int),
			uint,HDB_DATUM *) ;

#if	CF_AUDITENTRY && CF_DEBUGS
static int	entry_audit(HDB_ENT *) ;
#endif

static int	cursor_stabilize(HDB_CUR *) ;
static int	cursor_inc(HDB_CUR *) ;

static int	fetchcur_start(FETCHCUR *,uint (*)(const void *,int),
			int,HDB_CUR *,HDB_DATUM *) ;
static int	fetchcur_adv(FETCHCUR *) ;

static HDB_ENT	**getpoint(HDB *,uint,HDB_DATUM *) ;

#if	CF_DEBUGS
static int	mkprstr(char *,int,const char *,int) ;
static int	isweirdo(const char *,int) ;
#endif /* CF_DEBUGS */


/* local variables */

static const HDB_DATUM	nulldatum = {
	NULL, 0 
} ;

static const HDB_CUR	icur = { 
	-1, 0, 0 
} ;


/* exported subroutines */


int hdb_start(HDB *op,int n,int at,uint (* hashfunc)(),int (* cmpfunc)())
{
	int		rs = SR_OK ;
	int		size ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("hdb_start: n=%d hashfunc=%p\n",n,hashfunc) ;
#endif

	memset(op,0,sizeof(HDB)) ;
	op->at = at ;

	if (n < HDB_DEFSIZE)
	    n = HDB_DEFSIZE ;

	if (op->at > 0) {
	    const int	lan = MAX((n/6),6) ;
	    size = sizeof(HDB_ENT) ;
	    rs = lookaside_start(&op->es,size,lan) ;
	}

	if (rs >= 0) {
	    HDB_ENT	**hepp ;
	    size = n * sizeof(HDB_ENT *) ;
	    if ((rs = uc_malloc(size,&hepp)) >= 0) {
	        memset(hepp,0,size) ;
	        op->htlen = n ;
	        op->hashfunc = (hashfunc != NULL) ? hashfunc : defhashfunc ;
	        op->cmpfunc = (cmpfunc != NULL) ? cmpfunc : memcmp ;
	        op->htaddr = hepp ;
	        op->count = 0 ;
	        op->magic = HDB_MAGIC ;
	    } /* end if (memory-allocation) */
	    if (rs < 0) {
	        if (op->at > 0) lookaside_finish(&op->es) ;
	    }
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (hdb_start) */


/* delete the whole database */

/****

        Here we free all the memory associated with the DB, we erase the
        pointers to it, and invalidate the DB to prevent further use via any
        other possible pointers to it.

****/

int hdb_finish(HDB *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (HDB_BADMAGIC(op)) return SR_NOTOPEN ;

	rs1 = hdb_delall(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->htaddr != NULL) {
	    rs1 = uc_free(op->htaddr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->htaddr = NULL ;
	}

	if (op->at > 0) {
	    rs1 = lookaside_finish(&op->es) ;
	    if (rs >= 0) rs = rs1 ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (hdb_finish) */


int hdb_delall(HDB *op)
{
	HDB_ENT		*hp, *next, *nhp, *nnhp ;
	HDB_ENT		**hepp ;
	uint		idx ;

	if (op == NULL) return SR_FAULT ;

	if (HDB_BADMAGIC(op)) return SR_NOTOPEN ;

	hepp = op->htaddr ;
	for (idx = 0 ; idx < op->htlen ; idx += 1) {
	    for (hp = hepp[idx] ; hp != NULL ; hp = next) {
	        if (hp->same != NULL) {
	            nhp = hp->same ;
	            while (nhp->same != NULL) {
	                nnhp = nhp->same ;
	                hdb_entdel(op,nhp) ;
	                nhp = nnhp ;
	            } /* end while */
	            hdb_entdel(op,nhp) ;
	        } /* end if (freeing intermediate entries) */
	        next = hp->next ;
#ifdef	OPTIONAL
	        hp->next = NULL ;	/* optional */
#endif
	        hdb_entdel(op,hp) ;
	    } /* end for */
	    hepp[idx] = NULL ;
	} /* end for */

	op->count = 0 ;
	return SR_OK ;
}
/* end subroutine (hdb_delall) */


/* store an item into the DB */

/****

        Note that addresses in the key and the value DATUMs should point at
        memory that is not changed for the life of the item in the DB -- if the
        user wants it to be there when they access it!

        A "value" structure MUST always be passed to this subroutine from the
        caller. But the buffer address of a "value" can be NULL.

        Also, zero length values can be stored by specifying a value length of
        zero (buffer address of anything).

****/

int hdb_store(HDB *op,HDB_DATUM key,HDB_DATUM value)
{
	HDB_ENT		*ep, *ohp ;
	HDB_ENT		**nextp ;
	uint		hv ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	char		prbuf[HDB_PRBUFLEN + 1] ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (HDB_BADMAGIC(op)) return SR_NOTOPEN ;

	if (key.buf == NULL) return SR_FAULT ;

	if (key.len < 0)
	    key.len = strlen(key.buf) ;

#if	CF_DEBUGS
	mkprstr(prbuf,HDB_PRBUFLEN,key.buf,key.len) ;
	debugprintf("hdb_store: key=>%s< len=%d\n",
	    prbuf,key.len) ;
#endif

	if ((value.buf != NULL) && (value.len < 0))
	    value.len = strlen(value.buf) ;

/* should we resize the hash-table array? */

#if	CF_EXTEND
	if (op->count >= (op->htlen * 1)) {
	    rs = hdb_ext(op) ;
	}
#endif /* CF_EXTEND */

/* proceed to prepare and insert new entry */

	if (rs >= 0) {
	    if ((rs = hdb_entnew(op,&ep)) >= 0) {
	        hv = (*op->hashfunc)(key.buf,key.len) ;
	        if ((rs = entry_load(ep,hv,&key,&value)) >= 0) {
	            nextp = getpoint(op,hv,&key) ;
	            ohp = *nextp ;
	            if (ohp != NULL) {
	                while (ohp->same != NULL) {
	                    ohp = ohp->same ;
	                }
	                ohp->same = ep ;
	            } else {
	                *nextp = ep ;	/* append to hash chain */
	            } /* end if */
	            op->count += 1 ;
	        } else {
	            hdb_entdel(op,ep) ;
	        }
	    } /* end if (hdb_entnew) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("hdb_store: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (hdb_store) */


/* delete all entries with a specified key */

/****

	This subroutine deletes all entries with the specified "key".
	We return the number of entries deleted.

****/

int hdb_delkey(HDB *op,HDB_DATUM key)
{
	HDB_ENT		*hp, *nhp ;
	HDB_ENT		**nextp ;
	uint		hv ;
	int		rs = SR_OK ;
	int		n = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (key.buf == NULL) return SR_FAULT ;

	if (HDB_BADMAGIC(op)) return SR_NOTOPEN ;

	if (key.len < 0)
	    key.len = strlen(key.buf) ;

/* get the point of deletion (if there is one) */

	hv = (*op->hashfunc)(key.buf,key.len) ;
	nextp = getpoint(op,hv,&key) ;

	hp = *nextp ;
	if (hp != NULL) {
	    int	ocount = op->count ;

/* unlink this entry from the chain */

	    *nextp = hp->next ;			/* skip this entry */
#ifdef	OPTIONAL
	    hp->next = NULL ;			/* optional */
#endif

/* OK, we are isolated now from the chain */

	    while (hp->same != NULL) {
	        nhp = hp->same ;
	        op->count -= 1 ;
	        hdb_entdel(op,hp) ;
	        hp = nhp ;
	    } /* end while */

	    op->count -= 1 ;
	    hdb_entdel(op,hp) ;

	    n = (ocount - op->count) ;

	} else {
	    rs = SR_NOTFOUND ;
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (hdb_delkey) */


/* delete an entry by its cursor */

/****

	This subroutine deletes one entry specified by the given cursor.
	The cursor is set so that further fetches from the same cursor
	will work properly (the cursor is moved back)!

	f_adv=0		cursor backs up
	f_adv=1		cursor advances to net entry

****/

int hdb_delcur(HDB *op,HDB_CUR *curp,int f_adv)
{
	HDB_ENT		**hepp ;
	HDB_ENT		*ep, *pep ;
	HDB_ENT		*pjep = NULL ;
	HDB_ENT		*pkep = NULL ;
	HDB_CUR		ncur ;
	ENTRYINFO	ei ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (HDB_BADMAGIC(op)) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("hdb_delcur: ent ci=%d cj=%d ck=%d\n",
	    curp->i,curp->j,curp->k) ;
#endif

	cursor_stabilize(curp) ;

	if ((rs = hdb_getentry(op,&ei,curp)) >= 0) {
	    int		i, j ;

/* do it (delete the entry) */

	    hepp = op->htaddr ;
	    pjep = ei.pjep ;
	    pkep = ei.pkep ;
	    ep = ei.ep ;
	    ncur = *curp ;

/* determine any necessary cursor adjustment */

	    if (f_adv) {
	        if (ep->same == NULL) { /* code-reviewed */
	            ncur.k = 0 ;
	            if (ep->next == NULL) {
	                ncur.j = 0 ;
	                for (i = (ncur.i + 1) ; i < op->htlen ; i += 1) {
	                    if (hepp[i] != NULL) break ;
	                }
	                ncur.i = i ;
	            } else {
	                ncur.j += 1 ;
		    }
	        } /* end if */
	    } else {
	        if (curp->k == 0) {
	            int	k ;
	            if (curp->j == 0) {
	                ncur.j = 0 ;
	                ncur.k = 0 ;
	                for (i = (curp->i - 1) ; i >= 0 ; i -= 1) {
	                    if (hepp[i] != NULL) break ;
	                }
	                if (i >= 0) {
	                    pep = hepp[i] ;
	                    for (j = 0 ; pep->next != NULL ; j += 1) {
	                        pep = pep->next ;
	                    }
	                    ncur.j = j ;
	                    for (k = 0 ; pep->same != NULL ; k += 1) {
	                        pep = pep->same ;
	                    }
	                    ncur.k = k ;
	                }
	                ncur.i = i ;
	            } else {
	                ncur.j = (curp->j - 1) ;
	                pep = pjep ;
	                for (k = 0 ; pep->same != NULL ; k += 1) {
	                    pep = pep->same ;
	                }
	                ncur.k = k ;
	            }
	        } else {
	            ncur.k = (curp->k - 1) ;
	        }
	    } /* end if (cursor disposition) */

/* do all necessary list pointer adjustments */

	    if (curp->k == 0) { /* code-reviewed */

	        i = curp->i ;
	        if (curp->j == 0) {

	            if (ep->same != NULL) {
	                (ep->same)->next = ep->next ;
	                hepp[i] = ep->same ;
	            } else {
	                hepp[i] = ep->next ;
	            }

	        } else {

	            if (ep->same != NULL) {
	                (ep->same)->next = ep->next ;
	                pjep->next = ep->same ;
	            } else {
	                pjep->next = ep->next ;
		    }

	        } /* end if */

	    } else {
	        pkep->same = ep->same ;
	    }

/* update cursor */

	    *curp = ncur ;

/* delete the entry itself (return to free-memory pool) */

	    op->count -= 1 ;
	    rs = hdb_entdel(op,ep) ;

	} /* end if (hdb_getentry) */

#if	CF_DEBUGS
	debugprintf("hdb_delcur: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (hdb_delcur) */


/* advance the cursor to the next entry matching the key-cursor pair */
int hdb_nextrec(HDB *op,HDB_DATUM key,HDB_CUR *curp)
{
	int		rs ;

	rs = hdb_fetchrec(op,key,curp,NULL,NULL) ;

	return rs ;
}
/* end subroutine (hdb_nextrec) */


/* subroutine to fetch the next data corresponding to a key */
int hdb_fetch(HDB *op,HDB_DATUM key,HDB_CUR *curp,HDB_DATUM *valp)
{
	return hdb_fetchrec(op,key,curp,NULL,valp) ;
}
/* end subroutine (hdb_fetch) */


/* fetch the next whole record by key-cursor combination */
int hdb_fetchrec(op,key,curp,keyp,valp)
HDB		*op ;
HDB_DATUM	key ;
HDB_CUR		*curp ;
HDB_DATUM	*keyp ;
HDB_DATUM	*valp ;
{
	HDB_ENT		*ep ;
	HDB_CUR		ncur ;
	FETCHCUR	fc ;
	int		rs ;
	int		htlen ;

#if	CF_DEBUGS
	char	prbuf[HDB_PRBUFLEN + 1] ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (key.buf == NULL) return SR_FAULT ;

	if (HDB_BADMAGIC(op)) return SR_NOTOPEN ;

	if (key.len < 0)
	    key.len = strlen(key.buf) ;

#if	CF_DEBUGS
	mkprstr(prbuf,HDB_PRBUFLEN,key.buf,key.len) ;
	debugprintf("hdb_fetchrec: ent OK key=>%s<\n",
	    prbuf) ;
	debugprintf("hdb_fetchrec: kl=%u alt_key=>%t<\n",
	    key.len,key.buf,strnlen(key.buf,key.len)) ;
	if (curp != NULL)
	    debugprintf("hdb_fetchrec: CCUR ci=%d cj=%d ck=%d\n",
	        curp->i,curp->j,curp->k) ;
#endif

	if (curp != NULL) {
	    ncur = *curp ;
	} else {
	    ncur = icur ;
	}

/* find it if we have it */

#if	CF_DEBUGS
	debugprintf("hdb_fetchrec: fetchcur_start()\n") ;
#endif

	htlen = op->htlen ;
	if ((rs = fetchcur_start(&fc,op->hashfunc,htlen,&ncur,&key)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("hdb_fetchrec: fetchcur_start() rs=%d\n",rs) ;
	    debugprintf("hdb_fetchrec: NCUR ci=%d cj=%d ck=%d\n",
	        ncur.i,ncur.j,ncur.k) ;
#endif

	    rs = hdb_findkeyentry(op,&fc,&key,&ep) ;

#if	CF_DEBUGS
	    debugprintf("hdb_fetchrec: hdb_findkeyentry() rs=%d\n",rs) ;
#endif

	    if ((rs == SR_NOTFOUND) && (fetchcur_adv(&fc) >= 0)) {

#if	CF_DEBUGS
	        debugprintf("hdb_fetchrec: ADV ci=%d cj=%d ck=%d\n",
	            ncur.i,ncur.j,ncur.k) ;
#endif

	        rs = hdb_findkeyentry(op,&fc,&key,&ep) ;

#if	CF_DEBUGS
	        debugprintf("hdb_fetchrec: ADV hdb_findkeyentry() rs=%d\n",rs) ;
#endif

	    }

	    if (rs >= 0) {
	        if (curp != NULL) *curp = ncur ;
	        if (ep != NULL) {
	            if (keyp != NULL) *keyp = ep->key ;
	            if (valp != NULL) *valp = ep->val ;
	        }
	    }

#if	CF_DEBUGS
	    if ((rs >= 0) && (valp != NULL) && (valp->buf != NULL)) {
	        mkprstr(prbuf,HDB_PRBUFLEN,valp->buf,valp->len) ;
	        debugprintf("hdb_fetchrec: val=>%s<\n",prbuf) ;
	    }
#endif

	} /* end if (fetchcur_start) */

#if	CF_DEBUGS
	debugprintf("hdb_fetchrec: ret rs=%d ci=%d cj=%d ck=%d\n",
	    rs, ncur.i,ncur.j,ncur.k) ;
#endif

	if (rs < 0) {
	    if (keyp != NULL) *keyp = nulldatum ;
	    if (valp != NULL) *valp = nulldatum ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("hdb_fetchrec: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (hdb_fetchrec) */


/* get the current record under the CURRENT cursor regardless of key */
int hdb_getrec(op,curp,keyp,valp)
HDB		*op ;
HDB_CUR		*curp ;
HDB_DATUM	*keyp ;
HDB_DATUM	*valp ;
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("hdb_getrec: ent\n") ;
#endif

	rs = hdb_get(op,FALSE,nulldatum,curp,keyp,valp) ;

	return rs ;
}
/* end subroutine (hdb_getrec) */


/* get the current record under the cursor but only if it matches the key */
int hdb_getkeyrec(op,key,curp,keyp,valp)
HDB		*op ;
HDB_DATUM	key ;
HDB_CUR		*curp ;
HDB_DATUM	*keyp ;
HDB_DATUM	*valp ;
{
	int		rs ;

	rs = hdb_get(op,TRUE,key,curp,keyp,valp) ;

	return rs ;
}
/* end subroutine (hdb_getkeyrec) */


/* advance the cursor to the next entry regardless of key */
int hdb_next(HDB *op,HDB_CUR *curp)
{
	int		rs ;

	rs = hdb_enum(op,curp,NULL,NULL) ;

	return rs ;
}
/* end subroutine (hdb_next) */


/* subroutine to enumerate all entries */

/****

	This subroutine will return all entries in the DB using the given
	cursor to sequence through it all.  Specifically, it differs from
	'hdb_getrec()' in that it "fetches" the NEXT entry after the
	one under the current cursor!  It then updates the cursor to
	the returned entry.

****/

int hdb_enum(HDB *op,HDB_CUR *curp,HDB_DATUM *keyp,HDB_DATUM *valp)
{
	HDB_ENT		*ep = NULL ;
	HDB_ENT		*jep ;
	HDB_ENT		**hepp ;
	HDB_CUR		ncur ;
	int		rs = SR_NOTFOUND ;
	int		j, k ;

	if (op == NULL) return SR_FAULT ;

	if (HDB_BADMAGIC(op)) return SR_NOTOPEN ;

	if (curp != NULL) {
	    ncur = *curp ;
	} else {
	    ncur = icur ;
	}

#if	CF_DEBUGS
	debugprintf("hdb_enum: ent ci=%d cj=%d ck=%d\n",
	    curp->i,curp->j,curp->k) ;
#endif

	cursor_inc(&ncur) ;

	hepp = op->htaddr ;
	while (ncur.i < op->htlen) {

	    if ((jep = hepp[ncur.i]) != NULL) {

	        for (j = 0 ; j < ncur.j ; j += 1) {
	            if (jep->next == NULL) break ;
	            jep = jep->next ;
	        } /* end for */

	        if (j == ncur.j) {

	            while (jep != NULL) {

	                ep = jep ;
	                for (k = 0 ; k < ncur.k ; k += 1) {
	                    if (ep->same == NULL) break ;
	                    ep = ep->same ;
	                } /* end for */

	                if (k == ncur.k) {
	                    rs = SR_OK ;
	                    break ;
	                }

	                jep = jep->next ;
	                ncur.j += 1 ;
	                ncur.k = 0 ;

	            } /* end while */

	            if (rs >= 0) break ;
	        } /* end if */

	    } /* end if (have_i) */

	    ncur.i += 1 ;
	    ncur.j = 0 ;
	    ncur.k = 0 ;

	} /* end while */

	if ((rs >= 0) && (ep != NULL)) {
	    if (curp != NULL) *curp = ncur ;
	    if (keyp != NULL) *keyp = ep->key ;
	    if (valp != NULL) *valp = ep->val ;
	} /* end if (found an entry) */

#if	CF_DEBUGS
	debugprintf("hdb_enum: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (hdb_enum) */


#ifdef	COMMENT

/* walk the database (deprecated) */

/****

	Visit each entry by calling 'nodefunc()' with 'key', 'data' and
	'argp' as arguments.

****/

int hdb_walk(op,nodefunc,argp)
HDB		*op ;
int		(*nodefunc)() ;
void		*argp ;
{
	HDB_ENT		*hp, *nhp ;
	HDB_ENT		**hepp ;
	unsigned	idx ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (HDB_BADMAGIC(op)) return SR_NOTOPEN ;

	hepp = op->htaddr ;
	for (idx = 0 ; idx < op->htlen ; idx += 1) {
	    for (hp = hepp[idx] ; hp != NULL; hp = hp->next) {
	        if ((rs = (*nodefunc)(hp->key,hp->val,argp)) >= 0) {
	            nhp = hp ;
	            while (nhp->same != NULL) {
	                nhp = nhp->same ;
	                rs = (*nodefunc)(nhp->key,nhp->val,argp) ;
	                if (rs < 0) break ;
	            } /* end while */
	        } /* end if (ok) */
	        if (rs < 0) break ;
	    } /* end for (inner) */
	    if (rs < 0) break ;
	} /* end for (outer) */

	return rs ;
}
/* end subroutine (hdb_walk) */

#endif /* COMMENT */


/* count of items in container */
int hdb_count(HDB *op)
{

	if (op == NULL) return SR_FAULT ;

	if (HDB_BADMAGIC(op)) return SR_NOTOPEN ;

	return op->count ;
}
/* end subroutine (hdb_count) */


/* initialize a cursor */
int hdb_curbegin(HDB *hdbp,HDB_CUR *curp)
{

	if ((hdbp == NULL) || (curp == NULL)) return SR_FAULT ;

	if (HDB_BADMAGIC(hdbp)) return SR_NOTOPEN ;

	*curp = icur ;
	return SR_OK ;
}
/* end subroutine (hdb_curbegin) */


/* free up a cursor (for now just really the same a initialization!) */
int hdb_curend(HDB *hdbp,HDB_CUR *curp)
{

	if ((hdbp == NULL) || (curp == NULL)) return SR_FAULT ;

	if (HDB_BADMAGIC(hdbp)) return SR_NOTOPEN ;

	*curp = icur ;
	return SR_OK ;
}
/* end subroutine (hdb_curend) */


int hdb_hashtablen(HDB *op,uint *rp)
{

	if (op == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (HDB_BADMAGIC(op)) return SR_NOTOPEN ;

	*rp = op->htlen ;
	return SR_OK ;
}
/* end subroutine (hdb_hashtablen) */


int hdb_hashtabcounts(HDB *op,uint *rp,uint n)
{
	HDB_ENT		**hepp ;
	HDB_ENT		*hp ;
	uint		hi ;
	int		tc = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (HDB_BADMAGIC(op)) return SR_NOTOPEN ;

	if (n < op->htlen) return SR_OVERFLOW ;

	for (hi = 0 ; hi < op->htlen ; hi += 1) {
	    int		c = 0 ;
	    hepp = op->htaddr + hi ;
	    for (hp = *hepp ; hp != NULL ; hp = hp->next) {
	        c += 1 ;
	        if (hp->same != NULL) {
	            HDB_ENT	*sep ;
	            for (sep = hp->same ; sep != NULL ; sep = sep->same) {
	                c += 1 ;
	            }
	        } /* end if (same keys) */
	    } /* end for */
	    rp[hi] = c ;
	    tc += c ;
	} /* end for */

	return tc ;
}
/* end subroutine (hdb_hashtabcounts) */


int hdb_audit(HDB *op)
{
	HDB_ENT		*ep, **hepp ;
	int		rs = SR_OK ;
	int		i, j, k ;
	int		n = 0 ;

#if	CF_DEBUGS
	char		prbuf[HDB_PRBUFLEN + 1] ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (HDB_BADMAGIC(op)) return SR_NOTOPEN ;

	hepp = op->htaddr ;
	for (i = 0 ; i < op->htlen ; i += 1) {

	    j = 0 ;
	    for (ep = hepp[i] ; ep != NULL ; (ep = ep->next),(j += 1)) {

	        k = 0 ;
	        n += 1 ;

#if	CF_AUDITENTRY && CF_DEBUGS
	        rs = entry_audit(ep) ;
#endif

	        while ((rs >= 0) && (ep->same != NULL)) {

	            k += 1 ;
	            n += 1 ;

#if	CF_AUDITENTRY && CF_DEBUGS
	            rs = entry_audit(ep) ;
	            if (rs < 0) break ;
#endif

	            ep = ep->same ;

	        } /* end while */

	        if (rs < 0) {

#if	CF_DEBUGS
	            debugprintf("hdb_audit: BAD i=%d j=%d k=%d\n",
	                i,j,k) ;
#endif

	            break ;
	        }

	    } /* end for */

	    if (rs < 0) break ;
	} /* end for */

	if (rs >= 0) {

	    if (n != op->count) {

#if	CF_DEBUGS
	        debugprintf("hdb_audit: BAD count\n") ;
#endif

	        rs = SR_BADFMT ;
	    }

	}

	return rs ;
}
/* end subroutine (hdb_audit) */


/* private subroutines */


static int hdb_entnew(HDB *op,HDB_ENT **epp)
{
	int		rs ;
	if (op->at == 0) {
	    rs = uc_malloc(sizeof(HDB_ENT),epp) ;
	} else {
	    rs = lookaside_get(&op->es,epp) ;
	}
	return rs ;
}
/* end subroutine (hdb_entnew) */


static int hdb_entdel(HDB *op,HDB_ENT *ep)
{
	int		rs = SR_OK ;
	if (op->at == 0) {
	    rs = uc_free(ep) ;
	} else {
	    rs = lookaside_release(&op->es,ep) ;
	}
	return rs ;
}
/* end subroutine (hdb_entdel) */


/* find an entry that matches our key */
static int hdb_findkeyentry(HDB *op,FETCHCUR *fcp,HDB_DATUM *kp,HDB_ENT **epp)
{
	HDB_CUR		*curp = fcp->curp ;
	HDB_ENT		*jep, *ep ;
	int		rs = SR_NOTFOUND ;
	int		rs1 ;
	int		i ;

#if	CF_DEBUGS
	char		prbuf[HDB_PRBUFLEN + 1] ;
#endif

/* search for our key */

	if ((jep = op->htaddr[curp->i]) != NULL) {

	    fcp->f_ikeyed = TRUE ;
	    for (i = 0 ; i < curp->j ; i += 1) {
	        if (jep->next == NULL) break ;
	        jep = jep->next ;
	    } /* end for */

#if	CF_DEBUGS
	    debugprintf("hdb_findkeyentry: curj=%d j=%d\n",curp->j,i) ;
#endif

	    if (i == curp->j) {

	        for ( ; jep != NULL ; jep = jep->next) {

#if	CF_DEBUGS
	            mkprstr(prbuf,HDB_PRBUFLEN,kp->buf,kp->len) ;
	            debugprintf("hdb_findkeyentry: dbkey=>%s< len=%d\n",
	                prbuf,kp->len) ;
#endif

	            rs1 = entry_match(jep,op->cmpfunc,fcp->hv,kp) ;

#if	CF_DEBUGS
	            debugprintf("hdb_findkeyentry: entry_match() rs=%d\n",rs1) ;
#endif

	            if (rs1 > 0) {
	                rs = SR_OK ;
	                break ;
	            }

	            curp->j += 1 ;

	        } /* end for */

	    } /* end if */

#if	CF_DEBUGS
	    debugprintf("hdb_findkeyentry: j-keyed rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

	        fcp->f_jkeyed = TRUE ;
	        ep = jep ;
	        for (i = 0 ; i < curp->k ; i += 1) {
	            if (ep->same == NULL) break ;
	            ep = ep->same ;
	        } /* end for */

#if	CF_DEBUGS
	        debugprintf("hdb_findkeyentry: curk=%d k=%d\n",curp->k,i) ;
#endif

	        if (i < curp->k)
	            rs = SR_NOTFOUND ;

	        if ((rs >= 0) && (epp != NULL))
	            *epp = ep ;

	    } /* end if (j-keyed) */

	} /* end if (was found) */

#if	CF_DEBUGS
	debugprintf("hdb_findkeyentry: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (hdb_findkeyentry) */


static int hdb_get(op,f,key,curp,keyp,valp)
HDB		*op ;
int		f ;
HDB_DATUM	key ;
HDB_CUR		*curp ;
HDB_DATUM	*keyp ;
HDB_DATUM	*valp ;
{
	ENTRYINFO	ei ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (HDB_BADMAGIC(op)) return SR_NOTOPEN ;

	if (curp == NULL) return SR_INVALID ;

	if (f && (key.buf == NULL)) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("hdb_get: ent htlen=%u f=%u ci=%d cj=%d ck=%d\n",
	    op->htlen,f,curp->i,curp->j,curp->k) ;
#endif

	cursor_stabilize(curp) ;

	if ((rs = hdb_getentry(op,&ei,curp)) >= 0) {
	    HDB_ENT	*ep = ei.ep ;

/* compare the keys (if asked to do so) */

	    if (f && (key.buf != NULL)) {
	        int rc = 1 ;
	        if (key.len == ep->key.len) {
	            rc = (*op->cmpfunc)(key.buf,ep->key.buf,key.len) ;
	        }
	        if (rc != 0) rs = SR_NOTFOUND ;
	    } /* end if (key comparison) */

	    if (rs >= 0) {
	        if (keyp != NULL) *keyp = ep->key ;
	        if (valp != NULL) *valp = ep->val ;
	    }

	} /* end if (entry found) */

	if (rs < 0) {
	    if (keyp != NULL) *keyp = nulldatum ;
	    if (valp != NULL) *valp = nulldatum ;
	}

#if	CF_DEBUGS
	debugprintf("hdb_get: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (hdb_get) */


/* extend the hash-table */
static int hdb_ext(HDB *op)
{
	HDB_ENT		**nhtaddr = NULL ;
	HDB_ENT		**htaddr = NULL ;
	HDB_ENT		*hep, *nhep ;
	int		rs = SR_OK ;
	int		nhtlen ;
	int		n ;
	int		size ;
	int		i = 0 ;

	htaddr = op->htaddr ;
	nhtlen = (op->htlen * 2) ;
	size = nhtlen * sizeof(HDB_ENT *) ;
	if ((rs = uc_malloc(size,&nhtaddr)) >= 0) {

	    memset(nhtaddr,0,size) ;

	    for (i = 0 ; i < op->htlen ; i += 1) {
	        if (htaddr[i] != NULL) {
	            for (hep = htaddr[i] ; hep != NULL ; hep = nhep) {
	                nhep = hep->next ;
	                rs = hdb_extinsert(op,nhtaddr,nhtlen,hep) ;
	                if (rs < 0) break ;
	            } /* end for */
	            if (rs >= 0) {
	                for (hep = htaddr[i] ; hep != NULL ; hep = nhep) {
	                    nhep = hep->next ;
	                    hdb_entdel(op,hep) ;
	                } /* end for */
	            }
	            if (rs < 0) break ;
	        }
	    } /* end for */

	    if (rs >= 0) {
	        uc_free(op->htaddr) ;
	        op->htaddr = nhtaddr ;
	        op->htlen = nhtlen ;
	    } else {
	        n = MIN(i,nhtlen) ;
	        for (i = 0 ; i < n ; i += 1) {
	            if (nhtaddr[i] != NULL) {
	                for (hep = nhtaddr[i] ; hep != NULL ; hep = nhep) {
	                    nhep = hep->next ;
	                    hdb_extkeyfree(op,hep) ;
	                } /* end for */
	            }
	        } /* end for */
	        uc_free(nhtaddr) ;
	    } /* end block (success) */

	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (hdb_extend) */


static int hdb_extinsert(HDB *op,HDB_ENT **htaddr,int htlen,HDB_ENT *hep)
{
	HDB_ENT		*ep, *nhep = NULL ;
	int		rs ;

/* get a new uninitialized entry from free-store */

	if ((rs = hdb_entnew(op,&nhep)) >= 0) {
	    uint		hv ;
	    int		hi ;

/* copy existing to new */

	    *nhep = *hep ;			/* memberwise copy */
	    nhep->next = NULL ;		/* this is newly determined */

/* determine how to insert the new entry */

	    hv = hep->hv ;
	    hi = hv % htlen ;
	    if (htaddr[hi] != NULL) {
	        ep = htaddr[hi] ;
	        htaddr[hi] = nhep ;
	        nhep->next = ep ;
	    } else
	        htaddr[hi] = nhep ;

	} /* end if (hdb_entnew) */

	return rs ;
}
/* end subroutine (hdb_extinsert) */


static int hdb_extkeyfree(HDB *op,HDB_ENT *shep)
{
	HDB_ENT		*hep, *nhep ;
	int		rs = SR_OK ;

	for (hep = shep ; hep != NULL ; hep = nhep) {
	    nhep = hep->same ;
	    hdb_entdel(op,hep) ;
	} /* end for */

	return rs ;
}
/* end subroutine (hdb_extkeyfree) */


/* yes, it is recursive! */
static int hdb_getentry(HDB *op,ENTRYINFO *eip,HDB_CUR *curp)
{
	int		rs = SR_OK ;
	int		i = curp->i ;

	if ((i >= 0) && (i < op->htlen)) {
	    HDB_ENT	**hepp = op->htaddr ;
	    HDB_ENT	*pjep = NULL ;
	    HDB_ENT	*pkep = NULL ;
	    HDB_ENT	*ep = hepp[i] ;
	    int		f_notdone = TRUE ;

	    while ((i < op->htlen) && ((ep = hepp[i]) == NULL)) {
	        i += 1 ;
	    }

	    if (curp->i != i) {
	        curp->i = i ;
	        curp->j = 0 ;
	        curp->k = 0 ;
	        if (ep != NULL) f_notdone = FALSE ;
	    }

	    if (f_notdone) {
	        if (ep != NULL) {
		    int	j ;

/* find pointers to this cursor entry */

	            for (j = 0 ; j < curp->j ; j += 1) { /* code-reviewed */
	                if (ep->next == NULL) break ;
	                pjep = ep ;
	                ep = ep->next ;
	            } /* end for */

	            if (j < curp->j) {
	                curp->k = 0 ;
	                curp->j = 0 ;
	                curp->i += 1 ;
	                rs = hdb_getentry(op,eip,curp) ;
	            }

	            if (rs >= 0) {
			int	k ;

	                for (k = 0 ; k < curp->k ; k += 1) { /* code-reviewed */
	                    if (ep->same == NULL) break ;
	                    pkep = ep ;
	                    ep = ep->same ;
	                } /* end for */

	                if (k < curp->k) {
	                    curp->k = 0 ;
	                    curp->j += 1 ;
	                    rs = hdb_getentry(op,eip,curp) ;
	                }

	                if (rs >= 0) {
	                    eip->pjep = pjep ;
	                    eip->pkep = pkep ;
	                    eip->ep = ep ;
	                }

	            } /* end if (ok) */

	        } else {
	            rs = SR_NOTFOUND ;
	        } /* end if (no-null) */
	    } /* end if (f_notdone) */

	} else {
	    rs = SR_NOTFOUND ;
	} /* end if (in bounds) */

	return rs ;
}
/* end subroutine (hdb_getentry) */


#if	CF_DEBUGS && CF_DEBUGDUMP
int hdb_debugdump(HDB *op)
{
	HDB_ENT		*ep ;
	HDB_ENT		*jep ;
	HDB_ENT		**hepp ;
	HDB_DATUM	*kp, *vp ;
	int		i ;
	char		prbuf[HDB_PRBUFLEN + 1] ;
	if (op == NULL) return SR_FAULT ;
	if (HDB_BADMAGIC(op)) return SR_NOTOPEN ;
	hepp = op->htaddr ;
	for (i = 0 ; i < op->htlen ; i += 1) {
	    if ((jep = hepp[i]) != NULL) {
	        debugprintf("hdb_debugdump: hi=%u\n",i) ;
	        for (jep = hepp[i] ; jep != NULL ; jep = jep->next) {
	            kp = &jep->key ;
	            mkprstr(prbuf,HDB_PRBUFLEN,kp->buf,kp->len) ;
	            debugprintf("hdb_debugdump: dbkey=>%s< len=%d\n",
	                prbuf,kp->len) ;
	            for (ep = jep ; ep != NULL ; ep = ep->same) {
	                vp = &ep->val ;
	                mkprstr(prbuf,HDB_PRBUFLEN,vp->buf,vp->len) ;
	                debugprintf("hdb_debugdump: val=>%s< len=%d\n",
	                    prbuf,vp->len) ;
	            } /* end for */
	        } /* end for */
	    } /* end if */
	} /* end for */
	return SR_OK ;
}
/* end subroutine (hdb_debugdump) */
#endif /* CF_DEBUGS */


/* initialize a new hash entry */
static int entry_load(ep,hv,keyp,valp)
HDB_ENT		*ep ;
uint		hv ;
HDB_DATUM	*keyp, *valp ;
{

#ifdef	OPTIONAL
	memset(ep,0,sizeof(HDB_ENT)) ;
#endif

	ep->next = NULL ;
	ep->same = NULL ;
	ep->hv = hv ;
	ep->key = *keyp ;
	ep->val = *valp ;
	return SR_OK ;
}
/* end subroutine (entry_load) */


static int entry_match(ep,cmpfunc,hv,keyp)
HDB_ENT		*ep ;
int		(*cmpfunc)(const void *,const void *,int) ;
uint		hv ;
HDB_DATUM	*keyp ;
{
	int		ekeylen ;
	int		f = TRUE ;
	const void	*ekeybuf ;

#if	CF_DEBUGS
	char		prbuf[HDB_PRBUFLEN + 1] ;
#endif

#if	CF_DEBUGS && CF_DEBUGMATCH
	debugprintf("hdb/entry_match: e_hv=%08X c_hv=%08X\n",
	    ep->hv,hv) ;
#endif

	ekeybuf = ep->key.buf ;
	ekeylen = ep->key.len ;

#if	CF_DEBUGS && CF_DEBUGMATCH
	debugprintf("hdb/entry_match: e_klen=%u c_klen=%u\n",
	    ekeylen,keyp->len) ;
	mkprstr(prbuf,HDB_PRBUFLEN,ekeybuf,ekeylen) ;
	debugprintf("hdb/entry_match: e_key=>%s< e_klen=%d\n",
	    prbuf,ekeylen) ;
	mkprstr(prbuf,HDB_PRBUFLEN,keyp->buf,keyp->len) ;
	debugprintf("hdb/entry_match: c_key=>%s< c_klen=%d\n",
	    prbuf,keyp->len) ;
#endif /* CF_DEBUGS */

	f = f && (ep->hv == hv) ;
	f = f && (ekeylen == keyp->len) ;
	f = f && ((*cmpfunc)(ekeybuf,keyp->buf,keyp->len) == 0) ;

	return f ;
}
/* end subroutine (entry_match) */


#if	CF_AUDITENTRY && CF_DEBUGS

static int entry_audit(ep)
HDB_ENT		*ep ;
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	char	prbuf[HDB_PRBUFLEN + 1] ;
#endif

	if (isweirdo(ep->key.buf,ep->key.len))
	    rs = SR_BADFMT ;

#if	CF_DEBUGS
	if (rs < 0) {
	    mkprstr(prbuf,HDB_PRBUFLEN,ep->key.buf,ep->key.len) ;
	    debugprintf("hdb/entry_audit: BAD key=>%s<\n",
	        prbuf) ;
	}
#endif

	return rs ;
}
/* end subroutine (entry_audit) */

#endif /* CF_AUDITENTRY */


static int cursor_stabilize(HDB_CUR *curp)
{

#ifdef	OPTIONAL
	if (curp == NULL) return SR_FAULT ;
#endif

	if (curp->i >= 0) {
	    if (curp->j < 0) {
	        curp->j = 0 ;
	        curp->k = 0 ;
	    } else {
	        if (curp->k < 0) {
	            curp->k = 0 ;
	        }
	    }
	}

	return SR_OK ;
}
/* end subroutine (cursor_stabilize) */


static int cursor_inc(HDB_CUR *curp)
{

#ifdef	OPTIONAL
	if (curp == NULL) return SR_FAULT ;
#endif

	if (curp->i < 0) {
	    curp->i = 0 ;
	    curp->j = 0 ;
	    curp->k = 0 ;
	} else {
	    if (curp->j < 0) {
	        curp->j = 0 ;
	        curp->k = 0 ;
	    } else {
	        if (curp->k <= 0) {
	            curp->k = 1 ;
	        } else {
	            curp->k += 1 ;
	        }
	    }
	}

	return SR_OK ;
}
/* end subroutine (cursor_inc) */


static int fetchcur_start(fcp,hfp,htl,curp,kp)
FETCHCUR	*fcp ;
uint		(*hfp)(const void *,int) ;
int		htl ;
HDB_CUR		*curp ;
HDB_DATUM	*kp ;
{
	int		rs = SR_OK ;

	if (curp->i < htl) {

#if	CF_DEBUGS
	    debugprintf("hdb/fetchcur_start: htl=%u\n",htl) ;
	    debugprintf("hdb/fetchcur_start: c=(%d,%d,%d)\n",
	        curp->i,curp->j,curp->k) ;
#endif

	    fcp->hfp = hfp ;
	    fcp->htl = htl ;
	    fcp->curp = curp ;

	    fcp->f_ikeyed = FALSE ;
	    fcp->f_jkeyed = FALSE ;

/* the hash on the key is always needed for key-matching */

#if	CF_DEBUGS
	    debugprintf("hdb/fetchcur_start: key=>%t<\n",
	        kp->buf,strnnlen(kp->buf,kp->len,40)) ;
#endif

	    fcp->hv = (*fcp->hfp)(kp->buf,kp->len) ;

#if	CF_DEBUGS
	    debugprintf("hdb/fetchcur_start: hv=%08x\n",fcp->hv) ;
#endif

/* continue */

	    if (curp->i < 0) {
	        curp->i = fcp->hv % fcp->htl ;
	        fcp->f_ikeyed = TRUE ;
	        curp->j = 0 ;
	        curp->k = 0 ;
	    } else if (curp->j < 0) {
	        curp->j = 0 ;
	        curp->k = 0 ;
	    } else if (curp->k <= 0) {
	        curp->k = 1 ;
	    } else {
	        curp->k += 1 ;
	    }

	} else {
	    rs = SR_NOTFOUND ;
	}

#if	CF_DEBUGS
	debugprintf("hdb/fetchcur_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (fetchcur_start) */


static int fetchcur_adv(FETCHCUR *fcp)
{
	HDB_CUR		*curp = fcp->curp ;
	int		rs = SR_ALREADY ;
	int		ci ;

	if (! fcp->f_ikeyed) {
#ifdef	COMMENT /* the hash on the key is always calculated before here */
	    fcp->hv = (*fcp->hfp)(kp->buf,kp->len) ;
#endif
	    ci = fcp->hv % fcp->htl ;
	    if (curp->i < ci) {
	        rs = SR_OK ;
	        curp->i = ci ;
	        curp->j = 0 ;
	        curp->k = 0 ;
	        fcp->f_ikeyed = TRUE ;
	        fcp->f_jkeyed = FALSE ;
	    }
	} /* end if (needed) */

	return (rs >= 0) ? curp->i : rs ;
}
/* end subroutine (fetchcur_adv) */


/* default hash function */
static uint defhashfunc(const void *buf,int buflen)
{
	unsigned	h = 0, g ;
	const char	*s = (const char *) buf ;

#if	CF_DEBUGS && CF_DEBUGHASH
	char		prbuf[HDB_PRBUFLEN + 1] ;
#endif

#if	CF_DEBUGS && CF_DEBUGHASH
	mkprstr(prbuf,HDB_PRBUFLEN,buf,buflen) ;
	debugprintf("hdb/defhashfunc: ent key=>%s< len=%d\n",
	    prbuf,buflen) ;
#endif

#if	CF_ELFHASH
	while (buflen-- > 0) {
	    h <<= 4 ;
	    h += *s++ ;
	    if ((g = (h & 0xF0000000)) != 0) {
	        h ^= (g >> 24) ;
	        h ^= g ;
	    }
	} /* end while */
#else /* CF_ELFHASH */
	while (buflen-- > 0) {
	    h = ((h << 1) + *s++) ;
	} /* end while */
#endif /* CF_ELFHASH */

#if	CF_DEBUGS && CF_DEBUGHASH
	debugprintf("hdb/defhashfunc: ret hv=%u\n",h) ;
#endif

	return h ;
}
/* end subroutine (defhashfunc) */


/* see if an entry is in the database */

/****
	The returned value is the address of the pointer that refers to
	the found object.  The pointer may be NULL if the object was
	not found.  If so, this pointer should be updated with the
	address of the object to be inserted, if insertion is desired.

****/

static HDB_ENT **getpoint(op,hv,keyp)
HDB		*op ;
uint		hv ;
HDB_DATUM	*keyp ;
{
	HDB_ENT		*ep ;
	HDB_ENT		*pep ;
	HDB_ENT		**hepp ;
	int		hi ;
	int		keylen = keyp->len ;
	const char	*keydat = keyp->buf ;
	const char	*hpkeydat ;

#if	CF_DEBUGS
	char		prbuf[HDB_PRBUFLEN + 1] ;
#endif

#if	CF_DEBUGS && 0
	mkprstr(prbuf,HDB_PRBUFLEN,keyp->buf,keyp->len) ;
	debugprintf("hdb/getpoint: ent key=>%s<\n",prbuf) ;
#endif

	hi = hv % op->htlen ;

#if	CF_DEBUGS
	debugprintf("hdb/getpoint: hv=%08x hi=%d\n",hv,hi) ;
#endif

	hepp = op->htaddr + hi ;
	pep = NULL ;
	for (ep = *hepp ; ep != NULL ; ep = ep->next) {
	    int		f = TRUE ;

#if	CF_DEBUGS && 0
	    debugprintf("hdb/getpoint: top of loop\n") ;
#endif

	    hpkeydat = ep->key.buf ;
	    if (hpkeydat == NULL) continue ;

	    f = f && (ep->hv == hv) && (ep->key.len == keylen) ;
	    f = f && ((*op->cmpfunc)(hpkeydat,keydat,keylen) == 0) ;
	    if (f) break ;

	    pep = ep ;

#if	CF_DEBUGS && 0
	    debugprintf("hdb/getpoint: bottom of loop\n") ;
#endif

	} /* end for */

#if	CF_DEBUGS
	debugprintf("hdb/getpoint: ret found=%s\n",
	    ((pep != NULL) ? "YES" : "NO")) ;
#endif

	return ((pep != NULL) ? &pep->next : hepp) ;
}
/* end subroutine (getpoint) */


#ifdef	COMMENT

static int ngetpoint(htaddr,htlen,cmpfunc,keyp,rpp)
HDB_ENT		**htaddr ;
int		htlen ;
int		(*cmpfunc)() ;
HDB_DATUM	*keyp ;
HDB_DATUM	**rpp ;
{
	HDB_ENT		*ep ;
	HDB_ENT		*pep ;
	HDB_ENT		**hepp ;
	uint		hv = keyp->hv ;
	int		hi ;
	int		keylen = keyp->len ;
	int		f = FALSE ;
	const char	*keydat = keyp->buf ;
	const char	*hpkeydat ;

	hi = hv % htlen ;
	hepp = htaddr + hi ;
	pep = NULL ;
	for (ep = *hepp ; ep != NULL ; ep = ep->next) {

	    hpkeydat = ep->key.buf ;
	    if (hpkeydat == NULL) continue ;

	    f = (ep->hv == hv) ;
	    f = f && (ep->key.len == keylen) ;
	    f = f && ((*cmpfunc)(hpkeydat,keydat,keylen) == 0) ;
	    if (f)
	        break ;

	    pep = ep ;

	} /* end for */

	*rpp = (pep != NULL) ? &pep->next : hepp ;
	return f ;
}
/* end subroutine (ngetpoint) */

#endif /* COMMENT */


/* debugging subroutines */


#if	CF_DEBUGS

static int mkprstr(char *buf,int buflen,cchar *s,int slen)
{
	int		n = 0 ;
	int		i ;

	if (buf == NULL) return SR_FAULT ;
	if (s == NULL) return SR_FAULT ;

	if (slen < 0)
	    slen = strlen(s) ;

	buf[0] = '\0' ;
	if (buflen < 0)
	    buflen = INT_MAX ;

	for (i = 0 ; (i < slen) && s[i] ; i += 1) {
	    const int	ch = MKCHAR(s[i]) ;
	    if (n >= buflen) break ;
	    buf[n] = ('?' + 128) ;
	    if (isprintlatin(ch)) {
	        buf[n] = ch ;
	    }
	    n += 1 ;
	} /* end for */

	buf[n] = '\0' ;
	return n ;
}
/* end subroutine (mkprstr) */


/* check for weirdoness */
static int isweirdo(s,slen)
const char	s[] ;
int		slen ;
{
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; (i < slen) && s[i] ; i += 1) {
	    f = (! isprintlatin(s[i])) ;
	    if (f) break ;
	} /* end for */

	return f ;
}
/* end subroutine (isweirdo) */

#endif /* CF_DEBUGS */


