/* varray */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VARRAY_INCLUDE
#define	VARRAY_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<lookaside.h>


/* object defines */

#define	VARRAY		struct varray_head
#define	VARRAY_CUR	struct varray_c
#define	VARRAY_FL	struct varray_flags
#define	VARRAY_DEFENTS	10


struct varray_head {
	void		**va ;
	LOOKASIDE	la ;
	int		esize ;		/* element size */
	int		c ;		/* count of items in array */
	int		n ;		/* extent of array */
	int		imax ;		/* maximum used */
} ;


typedef struct varray_head	varray ;


#if	(! defined(VARRAY_MASTER)) || (VARRAY_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int varray_start(varray *,int,int) ;
extern int varray_enum(varray *,int,void *) ;
extern int varray_acc(varray *,int,void *) ;
extern int varray_mk(varray *,int,void *) ;
extern int varray_del(varray *,int) ;
extern int varray_delall(varray *) ;
extern int varray_count(varray *) ;
extern int varray_find(varray *,void *) ;
extern int varray_search(varray *,void *,int (*)(),void *) ;
extern int varray_audit(varray *) ;
extern int varray_finish(varray *) ;

#ifdef	__cplusplus
}
#endif

#endif /* VARRAY_MASTER */

#endif /* VARRAY_INCLUDE */


