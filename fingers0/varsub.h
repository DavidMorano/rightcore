/* varsub */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VARSUB_INCLUDE
#define	VARSUB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vechand.h>
#include	<buffer.h>
#include	<bfile.h>
#include	<localmisc.h>


/* object defines */

#define	VARSUB_MAGIC		91827364
#define	VARSUB			struct varsub_head
#define	VARSUB_CUR		struct varsub_c
#define	VARSUB_FL		struct varsub_flags

/* initialization options */

#define	VARSUB_OBLANK		0	/* default, substitute blank */
#define	VARSUB_ONOBLANK		(1<<0)	/* substitute the key on failure */
#define	VARSUB_OBADNOKEY	(1<<1)	/* fail out on failure */
#define	VARSUB_OBRACE		(1<<2)	/* substitute on braces */
#define	VARSUB_OPAREN		(1<<3)	/* substitute on parentheses */

#define	VARSUB_MBLANK		VARSUB_OBLANK
#define	VARSUB_MNOBLANK		VARSUB_ONOBLANK
#define	VARSUB_MBADNOKEY	VARSUB_OBADNOKEY
#define	VARSUB_MBRACE		VARSUB_OBRACE
#define	VARSUB_MPAREN		VARSUB_OPAREN


struct varsub_flags {
	uint		badnokey:1 ;
	uint		noblank:1 ;
	uint		brace:1 ;
	uint		paren:1 ;
	uint		sorted:1 ;
} ;

struct varsub_c {
	int		i ;
} ;

struct varsub_head {
	uint		magic ;
	VARSUB_FL	f ;
	vechand		subs ;
	int		n ;		/* current allocated length */
	int		i ;		/* maximum length used */
	int		badline ;
} ;


typedef struct varsub_head	varsub ;


#if	(! defined(VARSUB_MASTER)) || (VARSUB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	varsub_start(VARSUB *,int) ;
extern int	varsub_add(VARSUB *,cchar *,int,cchar *,int) ;
extern int	varsub_addva(VARSUB *,cchar **) ;
extern int	varsub_addquick(VARSUB *,cchar *,int,cchar *,int) ;
extern int	varsub_addvaquick(VARSUB *,cchar **) ;
extern int	varsub_curbegin(VARSUB *,VARSUB_CUR *) ;
extern int	varsub_enum(VARSUB *,VARSUB_CUR *,cchar **,cchar **) ;
extern int	varsub_curend(VARSUB *,VARSUB_CUR *) ;
extern int	varsub_del(VARSUB *,const char *,int) ;
extern int	varsub_fetch(VARSUB *,cchar *,int,cchar **) ;
extern int	varsub_expand(VARSUB *,char *,int,cchar *,int) ;
extern int	varsub_expandbuf(VARSUB *,BUFFER *,cchar *,int) ;
extern int	varsub_expandfile(VARSUB *,bfile *,bfile *) ;
extern int	varsub_finish(VARSUB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* VARSUB_MASTER */

#endif /* VARSUB_INCLUDE */


