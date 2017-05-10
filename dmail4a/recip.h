/* recip */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	RECIP_INCLUDE
#define	RECIP_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<vecitem.h>
#include	<localmisc.h>


#ifndef	UINT
#define	UINT	unsigned int
#endif


#define	RECIP_MAGIC	0x73169284
#define	RECIP		struct recip_head
#define	RECIP_FL	struct recip_flags
#define	RECIP_ENT	struct recip_ent


#ifndef	LOGNAMELEN
#ifdef	PASS_MAX
#define	LOGNAMELEN	PASS_MAX
#else
#define	LOGNAMELEN	32
#endif
#endif


struct recip_ent {
	int		moff ;
	int		mlen ;
} ;

struct recip_flags {
	uint		user:1 ;	/* is an actual user */
} ;

struct recip_head {
	uint		magic ;
	RECIP_FL	f ;		/* flags */
	vecitem		mds ;		/* message delivery entries */
	uid_t		uid ;
	int		mbo ;		/* mailbox-message offset */
	int		n ;		/* number of deliveries */
	int		ds ;		/* delivery status */
	const char	*recipient ;	/* recipient name */
	const char	*name ;		/* recipient real-name (if known) */
	const char	*maildname ;	/* recipient maildir */
} ;


#if	(! defined(RECIP_MASTER)) || (RECIP_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	recip_start(RECIP *,const char *,int) ;
extern int	recip_get(RECIP *,const char **) ;
extern int	recip_setuser(RECIP *,uid_t) ;
extern int	recip_setname(RECIP *,cchar *,int) ;
extern int	recip_setmailspool(RECIP *,cchar *,int) ;
extern int	recip_mbo(RECIP *,int) ;
extern int	recip_ds(RECIP *,int) ;
extern int	recip_mo(RECIP *,int,int) ;
extern int	recip_match(RECIP *,const char *,int) ;
extern int	recip_getmbo(RECIP *) ;
extern int	recip_getmo(RECIP *,int,int *) ;
extern int	recip_getname(RECIP *,cchar **) ;
extern int	recip_getmailspool(RECIP *,cchar **) ;
extern int	recip_getuser(RECIP *,uid_t *) ;
extern int	recip_isuser(RECIP *) ;
extern int	recip_finish(RECIP *) ;

#ifdef	__cplusplus
}
#endif

#endif /* RECIP_MASTER */

#endif /* RECIP_INCLUDE */


