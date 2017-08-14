/* lu */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	LU_INCLUDE
#define	LU_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<bfile.h>
#include	<hdb.h>
#include	<localmisc.h>


struct lu_flags {
	uint		open:1 ;
} ;

struct lu_value {
	struct lu_flags	f ;
	bfile		infile, outfile, errfile ;
} ;

struct lu_head {
	ulong		magic ;
	HDB_HEAD	lh ;
} ;


typedef struct lu_head	lu ;


#define	LU		struct lu_head

#define	LU_HEAD		struct lu_head
#define	LU_CUR		struct hdb_cur
#define	LU_CURSOR	struct hdb_cur
#define	LU_VALUE	struct hdb_value


#ifndef	LU_MASTER

#ifdef	__cplusplus
extern "C" {
#endif

extern int	lu_init(LU *) ;
extern int	lu_free(LU *) ;
extern int	lu_count(LU *) ;
extern int	lu_getkey(LU *,LU_CUR *,char **) ;
extern int	lu_delcur(LU *,LU_CUR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* LU_MASTER */


#endif /* LU_INCLUDE */



