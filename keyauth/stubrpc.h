/* stubrpc */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	STUBRPC_INCLUDE
#define	STUBRPC_INCLUDE	1


#ifndef	MAXNETNAMELEN
#define	MAXNETNAMELEN	255	/* maximum length of network user's name */
#endif

#ifndef	HEXKEYBYTES
#define	HEXKEYBYTES	48
#endif


typedef char			keybuf[HEXKEYBYTES] ;
typedef char			*netnamestr ;

struct key_netstarg {
	keybuf		st_priv_key ;
	keybuf		st_pub_key ;
	netnamestr	st_netname ;
} ;

typedef struct key_netstarg	key_netstarg ;


#if	(! defined(STUBRPC_MASTER)) || (STUBRPC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	key_secretkey_is_set(void) ;
extern int	key_setnet(struct key_netstarg *) ;
extern int	getnetname(char *) ;
extern int	getsecretkey(cchar *,keybuf,cchar *) ;


#ifdef	__cplusplus
}
#endif

#endif /* STUBRPC_MASTER */

#endif /* STUBRPC_INCLUDE */


