/* sysuserattr */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSUSERATTR_INCLUDE
#define	SYSUSERATTR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<user_attr.h>

#include	<filemap.h>
#include	<localmisc.h>


/* object defines */

#define	SYSUSERATTR		struct sysuserattr_head
#define	SYSUSERATTR_MAGIC	0x88776217
#define	SYSUSERATTR_FNAME	"/sys/userattr"


struct sysuserattr_head {
	uint		magic ;
	FILEMAP		b ;
} ;


typedef struct sysuserattr_head	sysuserattr ;


#if	(! defined(SYSUSERATTR_MASTER)) || (SYSUSERATTR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sysuserattr_open(SYSUSERATTR *,const char *) ;
extern int sysuserattr_close(SYSUSERATTR *) ;
extern int sysuserattr_readent(SYSUSERATTR *,userattr_t *,char *,int) ;
extern int sysuserattr_reset(SYSUSERATTR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSUSERATTR_MASTER */

#endif /* SYSUSERATTR_INCLUDE */


