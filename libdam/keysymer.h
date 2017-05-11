/* keysymer */

/* create and cache message content files */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	KEYSYMER_INCLUDE
#define	KEYSYMER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<mapstrint.h>
#include	<localmisc.h>


#define	KEYSYMER_MAGIC		0x24282138
#define	KEYSYMER		struct keysymer_head
#define	KEYSYMER_KE		struct keysymer_e
#define	KEYSYMER_CUR		struct keysymer_c
#define	KEYSYMER_FL		struct keysymer_flags
#define	KEYSYMER_NAMELEN	60


struct keysymer_flags {
	uint		dummy:1 ;	/* dummy */
} ;

struct keysymer_e {
	int		keynum ;
	char		keyname[KEYSYMER_NAMELEN+1] ;
} ;

struct keysymer_c {
	MAPSTRINT_CUR	c ;
} ;

struct keysymer_head {
	uint		magic ;
	mapstrint	map ;
	KEYSYMER_FL	f ;
} ;


#if	(! defined(KEYSYMER_MASTER)) || (KEYSYMER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int keysymer_open(KEYSYMER *,cchar *) ;
extern int keysymer_count(KEYSYMER *) ;
extern int keysymer_lookup(KEYSYMER *,const char *,int) ;
extern int keysymer_curbegin(KEYSYMER *,KEYSYMER_CUR *) ;
extern int keysymer_curend(KEYSYMER *,KEYSYMER_CUR *) ;
extern int keysymer_enum(KEYSYMER *,KEYSYMER_CUR *,KEYSYMER_KE *) ;
extern int keysymer_close(KEYSYMER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* KEYSYMER_MASTER */

#endif /* KEYSYMER_INCLUDE */


