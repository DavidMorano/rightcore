/* readkeycmd */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	READCMDKEY_INCLUDE
#define	READCMDKEY_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<uterm.h>


#define	READCMDKEY	struct readcmdkey


struct readcmdkey {
	int	type ;			/* key-type */
	int	inter ;			/* intermediate value (numeric) */
} ;


#if	(! defined(READCMDKEY_MASTER)) || (READCMDKEY_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int readcmdkey(READCMDKEY *,UTERM *,int,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* READCMDKEY_MASTER */


#endif /* READCMDKEY_INCLUDE */


