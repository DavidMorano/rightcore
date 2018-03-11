/* inetaddrparse */

/* this little thing parses an INET address into parts */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	INETADDRPARSE_INCLUDE
#define	INETADDRPARSE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */


/* local object defines */

#define	INETADDRPARSE		struct inetaddrparse
#define	INETADDRPARSE_SS	struct inetaddrparse_ss


struct inetaddrparse_ss {
	const char	*sp ;
	int		sl ;
} ;

struct inetaddrparse {
	INETADDRPARSE_SS	af, host, port ;
} ;


#if	(! defined(INETADDRPARSE_MASTER)) || (INETADDRPARSE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int inetaddrparse_load(INETADDRPARSE *,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* INETADDRPARSE_MASTER */

#endif /* INETADDRPARSE_INCLUDE */


