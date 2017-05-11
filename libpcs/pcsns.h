/* pcsns */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */


#ifndef	PCSNS_INCLUDE
#define	PCSNS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<modload.h>
#include	<localmisc.h>

#include	"pcsnso.h"
#include	"pcsnsreq.h"


#define	PCSNS_MAGIC	0x99447244
#define	PCSNS		struct pcsns_head
#define	PCSNS_CUR	struct pcsns_c
#define	PCSNS_CALLS	struct pcsns_calls

/* query options */

#define	PCSNS_ONOSERV	PCSNSO_ONOSERV	/* do not call the server */
#define	PCSNS_OPREFIX	PCSNSO_OPREFIX	/* prefix match */


struct pcsns_c {
	uint	magic ;
	void	*scp ;		/* SO-cursor pointer */
} ;

struct pcsns_calls {
	int	(*open)(void *,cchar *) ;
	int	(*setopts)(void *,int) ;
	int	(*get)(void *,char *,int,cchar *,int) ;
	int	(*curbegin)(void *,void *) ;
	int	(*enumerate)(void *,void *,char *,int,int) ;
	int	(*curend)(void *,void *) ;
	int	(*audit)(void *) ;
	int	(*close)(void *) ;
} ;

struct pcsns_head {
	uint		magic ;
	MODLOAD		loader ;
	PCSNS_CALLS	call ;
	void		*obj ;		/* object pointer */
	int		objsize ;	/* object size */
	int		cursize ;	/* cursor size */
} ;


#if	(! defined(PCSNS_MASTER)) || (PCSNS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pcsns_open(PCSNS *,cchar *) ;
extern int pcsns_setopts(PCSNS *,int) ;
extern int pcsns_get(PCSNS *,char *,int,cchar *,int) ;
extern int pcsns_curbegin(PCSNS *,PCSNS_CUR *) ;
extern int pcsns_enum(PCSNS *,PCSNS_CUR *,char *,int,int) ;
extern int pcsns_curend(PCSNS *,PCSNS_CUR *) ;
extern int pcsns_audit(PCSNS *) ;
extern int pcsns_close(PCSNS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PCSNS_MASTER */

#endif /* PCSNS_INCLUDE */


