/* rex */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	REX_INCLUDE
#define	REX_INCLUDE	1


#include	<netfile.h>


#define	REX_DEFEXECSERVICE		512

/* 
This flag is used in the same namespace as the 'open(2)'
open flags and therefor is pretty dangerous, but we live on the edge!
*/

#ifndef	O_KEEPALIVE
#define	O_KEEPALIVE	0x01000000
#endif /* O_KEEPALIVE */


#define	REX_FL		struct rex_flags
#define	REX_AUTH	struct rex_auth


struct rex_flags {
		unsigned int	keepalive:1 ;
		unsigned int	changedir:1 ;
} ;

struct rex_auth {
	const char	*restrict ;
	const char	*username ;
	const char	*password ;
	NETFILE_ENT	**machinev ;
} ;


#endif /* REX_INCLUDE */


