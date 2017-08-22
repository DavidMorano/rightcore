/* citedb */


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


#ifndef	CITEDB_INCLUDE
#define	CITEDB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vecobj.h>
#include	<hdb.h>
#include	<localmisc.h>		/* extra types */


#define	CITEDB			struct citedb_head
#define	CITEDB_STORE		struct citedb_store
#define	CITEDB_ENT		struct citedb_e
#define	CITEDB_OFF		struct citedb_offset
#define	CITEDB_CUR		struct citedb_cur

#define	CITEDB_CITESTRLEN	3
#define	CITEDB_CITEKEYLEN	80


struct citedb_cur {
	int		i ;
} ;

/* user visible structure */
struct citedb_e {
	uint		coff ;
	int		fi ;
	int		ci ;		/* index of this citation */
	int		n ;		/* total number of this citation */
	char		citekey[CITEDB_CITEKEYLEN + 1] ;
	char		citestr[CITEDB_CITESTRLEN + 1] ;
} ;

/* store key-offset in list */
struct citedb_offset {
	CITEDB_STORE	*sp ;
	uint		coff ;		/* file offset of citation */
	int		fi ;		/* file index */
	int		ci ;		/* citation index of this citation */
} ;

/* store keyname in hash table */
struct citedb_store {
	const char	*citekey ;
	int		n ;		/* total number of this citation */
	char		citestr[CITEDB_CITESTRLEN + 1] ;
} ;

struct citedb_head {
	uint		magic ;
	HDB		store ;
	VECOBJ		list ;
	int		citestrindex ;	/* incremented as necessary */
} ;


#if	(! defined(CITEDB_MASTER)) || (CITEDB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	citedb_start(CITEDB *) ;
extern int	citedb_finish(CITEDB *) ;
extern int	citedb_adds(CITEDB *,int,uint,const char *,int) ;
extern int	citedb_curbegin(CITEDB *,CITEDB_CUR *) ;
extern int	citedb_curend(CITEDB *,CITEDB_CUR *) ;
extern int	citedb_enum(CITEDB *,CITEDB_CUR *,CITEDB_ENT *) ;
extern int	citedb_fetch(CITEDB *,const char *,
			CITEDB_CUR *,CITEDB_ENT *) ;
extern int	citedb_audit(CITEDB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CITEDB_MASTER */


#endif /* CITEDB_INCLUDE */



