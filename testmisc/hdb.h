/* hdb */

/* general-purpose in-core hashing, DBM-like interface */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	Orginally written due to frustration with other types of these things.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	HDB_INCLUDE
#define	HDB_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<lookaside.h>
#include	<localmisc.h>


#define HDB_MAGIC	314159261
#define	HDB_DEFSIZE	10
#define	HDB		struct hdb_head
#define HDB_DATUM	struct hdb_d
#define	HDB_CUR		struct hdb_c
#define HDB_ENT		struct hdb_e


struct hdb_d {
	const void	*buf ;
	int		len ;
} ;

/* this one is for internal use only */
struct hdb_e {
	HDB_ENT		*next ;		/* next in hash chain */
	HDB_ENT		*same ;		/* next w/ same key */
	HDB_DATUM	key ;
	HDB_DATUM	val ;
	uint		hv ;		/* hash-value of key */
} ;

struct hdb_c {
	int		i, j, k ;
} ;

struct hdb_head {
	uint		magic ;
	HDB_ENT		**htaddr ;	/* array HDB_ENT pointers */
	unsigned int	(*hashfunc)() ;
	int		(*cmpfunc)() ;
	LOOKASIDE	es ;		/* key-entries */
	uint		htlen ;
	uint		count ;
	int		at ;		/* allocation-type */
} ;


typedef struct hdb_head		hdb ;
typedef struct hdb_d		hdb_datum ;
typedef struct hdb_e		hdb_ent ;
typedef struct hdb_c		hdb_cur ;


#if	(! defined(HDB_MASTER)) || (HDB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int hdb_start(HDB *,int,int,unsigned int (*)(),int (*)()) ;
extern int hdb_store(HDB *,HDB_DATUM,HDB_DATUM) ;
extern int hdb_curbegin(HDB *,HDB_CUR *) ;
extern int hdb_curend(HDB *,HDB_CUR *) ;
extern int hdb_fetch(HDB *,HDB_DATUM,HDB_CUR *,HDB_DATUM *) ;
extern int hdb_fetchrec(HDB *,HDB_DATUM,HDB_CUR *,
			HDB_DATUM *,HDB_DATUM *) ;
extern int hdb_getkeyrec(HDB *,HDB_DATUM,HDB_CUR *,
			HDB_DATUM *,HDB_DATUM *) ;
extern int hdb_nextrec(HDB *,HDB_DATUM,HDB_CUR *) ;
extern int hdb_delkey(HDB *,HDB_DATUM) ;
extern int hdb_enum(HDB *,HDB_CUR *,HDB_DATUM *,HDB_DATUM *) ;
extern int hdb_getrec(HDB *,HDB_CUR *,HDB_DATUM *,HDB_DATUM *) ;
extern int hdb_next(HDB *,HDB_CUR *) ;
extern int hdb_delcur(HDB *,HDB_CUR *,int) ;
extern int hdb_delall(HDB *) ;
extern int hdb_count(HDB *) ;
extern int hdb_hashtablen(HDB *,uint *) ;
extern int hdb_hashtabcounts(HDB *,uint *,uint) ;
extern int hdb_audit(HDB *) ;
extern int hdb_finish(HDB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* HDB_MASTER */

#endif /* HDB_INCLUDE */


