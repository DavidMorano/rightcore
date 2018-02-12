/* received */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	RECEIVED_INCLUDE
#define	RECEIVED_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>

#include	<localmisc.h>


#define	RECEIVED_MAGIC	0x97634587
#define	RECEIVED	struct received_head


enum received_keys {
	received_keyfrom,
	received_keyby,
	received_keywith,
	received_keyid,
	received_keyfor,
	received_keydate,
	received_keyvia,
	received_keyoverlast
} ;

struct received_head {
	uint		magic ;
	char		*a ;
	const char	*key[received_keyoverlast] ;
} ;


#if	(! defined(RECEIVED_MASTER)) || (RECEIVED_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int received_start(RECEIVED *,const char *,int) ;
extern int received_getkey(RECEIVED *,int,const char **) ;
extern int received_getitem(RECEIVED *,int,const char **) ;
extern int received_finish(RECEIVED *) ;

#ifdef	__cplusplus
}
#endif

#endif /* RECEIVED_MASTER */


#if	(! defined(RECEIVED_MASTER))

#ifdef	__cplusplus
extern "C" {
#endif

extern const char	*received_keys[] ;

#ifdef	__cplusplus
}
#endif

#endif /* RECEIVED_MASTER */

#endif /* RECEIVED_INCLUDE */


