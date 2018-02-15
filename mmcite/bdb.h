/* bdb */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This module was changed to serve in the REFERM program.

	= 2005-10-01, David A­D­ Morano
        This was changed to work in the MMCITE program. The old REFERM program
        is really obsolete. It used a database lookup strategy to remote
        databases. The high-level problem is: what to do if the cited BIB entry
        isn't found? How does a maintainer of the present (local) document know
        what that BIB entry was? The new strategy (implemented by the MMCITE
        program) is more like what is done with BibTeX in the TeX (or LaTeX)
        world. All BIB databases are really expected to be maintained by the
        document creator -- not some centralized entiry. The older centralized
        model reflected more the use in the corporate world (where different
        people create BIB entries) than in the more "modern"
        personal-responsibility type of world! :-) Anyway, this is the way the
        gods seem to now want to do things. Deal with it!

*/

/* Copyright © 1998,2005 David A­D­ Morano.  All rights reserved. */


#ifndef	BDB_INCLUDE
#define	BDB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vecobj.h>
#include	<hdb.h>
#include	<localmisc.h>


#define	BDB		struct bdb_head

#define	BDB_CUR		struct bdb_cur
#define	BDB_ENT	struct bdb_ent

#define	BDB_QUERYKEY	"Q"
#define	BDB_CITEKEYLEN	100
#define	BDB_BUFLEN	(3 * MAXPATHLEN)

/* operational options */
#define	BDB_OUNIQ	0x01		/* queries must be unique */


struct bdb_cur {
	HDB_CUR		cur ;
} ;

/* we return this to the caller upon key-query */
struct bdb_ent {
	const char	*(*keyvals)[2] ;
	const char	*query ;
	int		nkeys ;
	int		size ;
	int		fi ;		/* file index */
} ;

struct bdb_head {
	uint		magic ;
	uint		opts ;
	const char	*qkbuf ;	/* query key name (usually 'Q') */
	VECOBJ		files ;
	HDB		keys ;
	int		unindexed ;	/* number of unindexed files */
	int		qklen ;
} ;


#if	(! defined(BDB_MASTER)) || (BDB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bdb_start(BDB *,const char *,int) ;
extern int	bdb_finish(BDB *) ;
extern int	bdb_add(BDB *,const char *) ;
extern int	bdb_count(BDB *) ;
extern int	bdb_query(BDB *,const char *,BDB_ENT *,char *,int) ;
extern int	bdb_curbegin(BDB *,BDB_CUR *) ;
extern int	bdb_curend(BDB *,BDB_CUR *) ;
extern int	bdb_delcur(BDB *,BDB_CUR *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* BDB_MASTER */


#endif /* BDB_INCLUDE */



