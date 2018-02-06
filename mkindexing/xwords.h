/* xwords */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	XWORDS_INCLUDE
#define	XWORDS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<localmisc.h>


#define	XWORDS		struct xwords_head
#define	XWORDS_WORD	struct xwords_word
#define	XWORDS_MAX	3


struct xwords_word {
	const char	*wp ;
	int		wl ;
} ;

struct xwords_head {
	XWORDS_WORD	words[XWORDS_MAX] ;
	XWORDS_WORD	*xa ;
	int		nwords ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	xwords_start(XWORDS *,const char *,int) ;
extern int	xwords_get(XWORDS *,int,const char **) ;
extern int	xwords_finish(XWORDS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* XWORDS_INCLUDE */


