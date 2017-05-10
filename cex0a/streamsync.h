/* streamsync */

/* data stream synchronization mechanism */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	STREAMSYNC_INCLUDE
#define	STREAMSYNC_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* object defines */

#define	STREAMSYNC_MAGIC	0x96817463
#define	STREAMSYNC		struct streamsync_head


struct streamsync_head {
	uint		magic ;
	char		*st ;		/* sequence to test against */
	char		*data ;		/* the data stream */
	int		stlen ;		/* sequence length needed */
	int		i ;		/* current length */
} ;


#if	(! defined(STREAMSYNC_MASTER)) || (STREAMSYNC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int streamsync_start(STREAMSYNC *,const char *,int) ;
extern int streamsync_test(STREAMSYNC *,int) ;
extern int streamsync_finish(STREAMSYNC *) ;

#ifdef	__cplusplus
}
#endif

#endif /* STREAMSYNC_MASTER */

#endif /* STREAMSYNC_INCLUDE */


