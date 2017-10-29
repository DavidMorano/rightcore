/* sysusernames */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSUSERNAMES_INCLUDE
#define	SYSUSERNAMES_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<pwd.h>

#include	<filemap.h>
#include	<localmisc.h>


/* object defines */

#define	SYSUSERNAMES		struct sysusernames_head
#define	SYSUSERNAMES_MAGIC	0x88776218
#define	SYSUSERNAMES_FNAME	"/sys/usernames"


struct sysusernames_head {
	uint		magic ;
	FILEMAP		b ;
} ;


typedef struct sysusernames_head	sysusernames ;


#if	(! defined(SYSUSERNAMES_MASTER)) || (SYSUSERNAMES_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sysusernames_open(SYSUSERNAMES *,const char *) ;
extern int sysusernames_close(SYSUSERNAMES *) ;
extern int sysusernames_readent(SYSUSERNAMES *,char *,int) ;
extern int sysusernames_reset(SYSUSERNAMES *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSUSERNAMES_MASTER */

#endif /* SYSUSERNAMES_INCLUDE */


