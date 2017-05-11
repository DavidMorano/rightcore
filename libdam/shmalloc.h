/* SHMALLOC (Shared-Memory Allocator) */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SHMALLOC_INCLUDE
#define	SHMALLOC_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#define	SHMALLOC		struct shmalloc_head
#define	SHMALLOC_B		struct shmalloc_block
#define	SHMALLOC_ALIGNSIZE	(2*sizeof(int))
#define	SHMALLOC_BLOCKSIZE	(4* SHMALLOC_ALIGNSIZE)


struct shmalloc_block {
	int		size ;		/* block size */
	int		next ;		/* offset to next block */
} ;

struct shmalloc_head {
	int		str ;		/* string-area offset */
	int		used ;
	SHMALLOC_B	b ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int shmalloc_init(SHMALLOC *,char *,int) ;
extern int shmalloc_fini(SHMALLOC *) ;
extern int shmalloc_alloc(SHMALLOC *,int) ;
extern int shmalloc_free(SHMALLOC *,int) ;
extern int shmalloc_avail(SHMALLOC *) ;
extern int shmalloc_audit(SHMALLOC *) ;
extern int shmalloc_used(SHMALLOC *) ;
extern int shmalloc_already(SHMALLOC *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* SHMALLOC_INCLUDE */


