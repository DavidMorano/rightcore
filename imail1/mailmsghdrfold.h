/* mailmsghdrfold */


/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILMSGHDRFOLD_INCLUDE
#define	MAILMSGHDRFOLD_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<localmisc.h>


/* object defines */
#define	MAILMSGHDRFOLD_MAGIC	0x88431773
#define	MAILMSGHDRFOLD		struct mailmsghdrfold_head
#define	MAILMSGHDRFOLD_FL	struct mailmsghdrfold_flags


/* options */
#define	MAILMSGHDRFOLD_MCARRIAGE	0x0001


/* constants */

#ifdef	LINE_MAX
#define	MAILMSGHDRFOLD_BUFLEN	MAX(LINE_MAX,4096)
#else
#define	MAILMSGHDRFOLD_BUFLEN	4096
#endif


struct mailmsghdrfold_flags {
	uint		dummy:1 ;
} ;

struct mailmsghdrfold_head {
	uint		magic ;
	MAILMSGHDRFOLD_FL	f ;
	const char	*sp ;
	int		sl ;
	int		mcols ;		/* message columns (usually 76) */
	int		ln ;		/* line within header instance */
} ;


#if	(! defined(MAILMSGHDRFOLD_MASTER)) || (MAILMSGHDRFOLD_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailmsghdrfold_start(MAILMSGHDRFOLD *,int,int,const char *,int) ;
extern int mailmsghdrfold_get(MAILMSGHDRFOLD *,int,const char **) ;
extern int mailmsghdrfold_finish(MAILMSGHDRFOLD *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILMSGHDRFOLD_MASTER */

#endif /* MAILMSGHDRFOLD_INCLUDE */


