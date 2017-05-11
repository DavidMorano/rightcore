/* uuname */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	UUNAME_INCLUDE
#define	UUNAME_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<modload.h>
#include	<localmisc.h>

#include	"uunames.h"


#define	UUNAME_MAGIC	0x99447243
#define	UUNAME		struct uuname_head
#define	UUNAME_CUR	struct uuname_c
#define	UUNAME_CALLS	struct uuname_calls


struct uuname_c {
	uint	magic ;
	void	*scp ;		/* SO-cursor pointer */
} ;

struct uuname_calls {
	int	(*open)(void *,const char *,const char *) ;
	int	(*count)(void *) ;
	int	(*exists)(void *,const char *,int) ;
	int	(*curbegin)(void *,void *) ;
	int	(*enumerate)(void *,void *,char *,int) ;
	int	(*curend)(void *,void *) ;
	int	(*audit)(void *) ;
	int	(*close)(void *) ;
} ;

struct uuname_head {
	uint		magic ;
	void		*obj ;		/* object pointer */
	MODLOAD		loader ;
	UUNAME_CALLS	call ;
	int		objsize ;	/* object size */
	int		cursize ;	/* cursor size */
} ;


#if	(! defined(UUNAME_MASTER)) || (UUNAME_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int uuname_open(UUNAME *,const char *,const char *) ;
extern int uuname_count(UUNAME *) ;
extern int uuname_exists(UUNAME *,const char *,int) ;
extern int uuname_curbegin(UUNAME *,UUNAME_CUR *) ;
extern int uuname_enum(UUNAME *,UUNAME_CUR *,char *,int) ;
extern int uuname_curend(UUNAME *,UUNAME_CUR *) ;
extern int uuname_audit(UUNAME *) ;
extern int uuname_close(UUNAME *) ;

#ifdef	__cplusplus
}
#endif

#endif /* UUNAME_MASTER */

#endif /* UUNAME_INCLUDE */


