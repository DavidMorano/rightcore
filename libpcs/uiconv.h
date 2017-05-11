/* uiconv */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	UICONV_INCLUDE
#define	UICONV_INCLUDE	1


#include	<sys/types.h>

#include	<localmisc.h>


#define	UICONV		struct uiconv_head
#define	UICONV_MAGIC	0x67298362


struct uiconv_head {
	uint		magic ;
	void		*cdp ;
} ;


#if	(! defined(UICONV_MASTER)) || (UICONV_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	uiconv_open(UICONV *,cchar *,cchar *) ;
extern int	uiconv_trans(UICONV *,cchar **,int *,char **,int *) ;
extern int	uiconv_close(UICONV *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(UICONV_MASTER)) || (UICONV_MASTER == 0) */

#endif /* UICONV_INCLUDE */


