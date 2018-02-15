/* lu */


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


#ifndef	LU_INCLUDE
#define	LU_INCLUDE	1


#include	<sys/types.h>

#include	<bfile.h>
#include	<hdb.h>
#include	<localmisc.h>


#define	LU		struct lu_head
#define	LU_HEAD		struct lu_head
#define	LU_FL		struct lu_flags
#define	LU_CUR		struct hdb_cur
#define	LU_CURSOR	struct hdb_cur
#define	LU_VALUE	struct hdb_value


struct lu_flags {
	uint		open:1 ;
} ;

struct lu_value {
	LU_FL		f ;
	bfile		infile, outfile, errfile ;
} ;

struct lu_head {
	ulong		magic ;
	HDB_HEAD	lh ;
} ;


typedef struct lu_head	lu ;


#ifndef	LU_MASTER

extern int	lu_init(LU *) ;
extern int	lu_free(LU *) ;
extern int	lu_count(LU *) ;
extern int	lu_getkey(LU *,LU_CUR *,char **) ;
extern int	lu_delcur(LU *,LU_CUR *) ;

#endif /* LU_MASTER */


#endif /* LU_INCLUDE */


