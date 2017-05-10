/* ema */


/* revision history:

	= 1998-02-03, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	EMA_INCLUDE
#define	EMA_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<vechand.h>
#include	<localmisc.h>		/* additional types */


#ifndef	UINT
#define	UINT	unsigned int
#endif


#define	EMA		struct ema_head
#define	EMA_ENT		struct ema_e
#define	EMA_FL		struct ema_flags
#define	EMA_MAGIC	0x73169284
#define	EMADEFENTS	4

/* mailing list types */
enum emnatypes {
	ematype_reg,			/* regular */
	ematype_pcs,			/* PCS list */
	ematype_lalias,			/* local alias */
	ematype_salias,			/* system alias */
	ematype_group,			/* RFC-822 mail-address "group" */
	ematype_overlast
} ;

enum emaparts {
	emapart_address,
	emapart_route,
	emapart_comment,
	emapart_overlast
} ;

struct ema_head {
	uint		magic ;
	vechand		list ;
	int		n ;
} ;

struct ema_flags {
	UINT		error:1 ;	/* address parse error */
	UINT		expanded:1 ;	/* list has been expanded */
} ;

struct ema_s {
	const char	*pp ;
	int		pl ;
} ;

struct ema_e {
	const char	*op ;		/* original address */
	const char	*ap ;		/* regular address part */
	const char	*rp ;		/* route-address part (if any) */
	const char	*cp ;		/* comment */
	EMA		*listp ;
	EMA_FL		f ;
	int		type ;		/* mailing list type */
	int		n ;		/* number in list */
	int		ol, al, rl, cl ;
} ;


#if	(! defined(EMA_MASTER)) || (EMA_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int ema_starter(EMA *,cchar *,int) ;
extern int ema_start(EMA *) ;
extern int ema_parse(EMA *,const char *,int) ;
extern int ema_addent(EMA *,EMA_ENT *) ;
extern int ema_get(EMA *,int,EMA_ENT **) ;
extern int ema_getbestaddr(EMA *,int,cchar **) ;
extern int ema_count(EMA *) ;
extern int ema_finish(EMA *) ;

#ifdef	__cplusplus
}
#endif

#endif /* EMA_MASTER */

#endif /* EMA_INCLUDE */


