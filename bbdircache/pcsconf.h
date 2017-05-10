/* pcsconf */


/* revision history:

	= 1992-03-10, David A­D­ Morano
	This module was originally written.

	= 1998-04-03, David A­D­ Morano
	This was modified for more general use.

	= 2008-10-07, David A­D­ Morano
        This was modified to allow for the main part to be a loadable
        module.

*/

/* Copyright © 1992,1998,2008 David A­D­ Morano.  All rights reserved. */

#ifndef	PCSCONF_INCLUDE
#define	PCSCONF_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<modload.h>
#include	<ptm.h>
#include	<localmisc.h>

#include	"pcsconfs.h"


#define	PCSCONF		struct pcsconf_head
#define	PCSCONF_CUR	struct pcsconf_c
#define	PCSCONF_CALLS	struct pcsconf_calls
#define	PCSCONF_FL	struct pcsconf_flags
#define	PCSCONF_MAGIC	0x97677246
#define	PCSCONF_USER	"pcs"


struct pcsconf_calls {
	int	(*start)(void *,cchar *,cchar **,cchar *) ;
	int	(*curbegin)(void *,void *) ;
	int	(*fetch)(void *,cchar *,int,void *,char *,int) ;
	int	(*enumerate)(void *,void *,char *,int,char *,int) ;
	int	(*curend)(void *,void *) ;
	int	(*audit)(void *) ;
	int	(*finish)(void *) ;
} ;

struct pcsconf_c {
	uint		magic ;
	void		*scp ;		/* SO-cursor pointer */
} ;

struct pcsconf_flags {
	uint		defaults:1 ;
} ;

struct pcsconf_head {
	uint		magic ;
	MODLOAD		loader ;
	PTM		m ;
	void		*obj ;		/* object pointer */
	void		*cookmgr ;	/* cookie-manager */
	cchar	*pr ;		/* supplied program-root */
	cchar	**envv ;	/* supplied environment */
	cchar	*pcsusername ;	/* calculated */
	PCSCONF_CALLS	call ;
	PCSCONF_FL	f ;
	uid_t		uid_pcs ;
	gid_t		gid_pcs ;
	int		objsize ;	/* object size */
	int		cursize ;	/* cursor size */
} ;


#if	(! defined(PCSCONF_MASTER)) || (PCSCONF_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pcsconf_start(PCSCONF *,cchar *,cchar **,cchar *) ;
extern int pcsconf_curbegin(PCSCONF *,PCSCONF_CUR *) ;
extern int pcsconf_fetch(PCSCONF *,cchar *,int,PCSCONF_CUR *,char *,int) ;
extern int pcsconf_enum(PCSCONF *,PCSCONF_CUR *,char *,int,char *,int) ;
extern int pcsconf_curend(PCSCONF *,PCSCONF_CUR *) ;
extern int pcsconf_fetchone(PCSCONF *,cchar *,int,char *,int) ;
extern int pcsconf_audit(PCSCONF *) ;
extern int pcsconf_getpcsuid(PCSCONF *) ;
extern int pcsconf_getpcsgid(PCSCONF *) ;
extern int pcsconf_getpcsusername(PCSCONF *,char *,int) ;
extern int pcsconf_getpr(PCSCONF *,cchar **) ;
extern int pcsconf_getenvv(PCSCONF *,cchar ***) ;
extern int pcsconf_finish(PCSCONF *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PCSCONF_MASTER */

#endif /* PCSCONF_INCLUDE */


