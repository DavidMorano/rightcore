/* strpack */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	STRPACK_INCLUDE
#define	STRPACK_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vechand.h>
#include	<localmisc.h>


/* object defines */

#define	STRPACK_MAGIC		0x42114683
#define	STRPACK			struct strpack_head
#define	STRPACK_CHUNK		struct strpack_chunk
#define	STRPACK_CHUNKSIZE	512


struct strpack_chunk {
	char		*cdata ;
	int		csize ;		/* allocated buffer length */
	int		i ;		/* index length */
	int		c ;		/* item count within chunk */
} ;

struct strpack_head {
	uint		magic ;
	STRPACK_CHUNK	*ccp ;	/* current chunk pointer */
	vechand		chunks ;
	int		chunksize ;
	int		totalsize ;
	int		c ;		/* total count */
} ;


typedef struct strpack_head	strpack ;


#if	(! defined(STRPACK_MASTER)) || (STRPACK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	strpack_start(STRPACK *,int) ;
extern int	strpack_store(STRPACK *,cchar *,int,cchar **) ;
extern int	strpack_count(STRPACK *) ;
extern int	strpack_size(STRPACK *) ;
extern int	strpack_finish(STRPACK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* STRPACK_MASTER */

#endif /* STRPACK_INCLUDE */


