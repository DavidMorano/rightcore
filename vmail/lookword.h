/* lookword */


/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

#ifndef	LOOKWORD_INCLUDE
#define	LOOKWORD_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#define	LOOKWORD_MAGIC	0x97543218
#define	LOOKWORD	struct lookword_head
#define	LOOKWORD_CUR	struct lookword_cur
#define	LOOKWORD_FL	struct lookword_flags
#define	LOOKWORD_WORD	struct lookword_word
/* options */
#define	LOOKWORD_OFOLD	(1<<0)		/* fold characters for comparison */
#define	LOOKWORD_ODICT	(1<<1)		/* use "dictionary" comparison rules */
#define	LOOKWORD_OWORD	(1<<2)		/* whole-word comparison */


struct lookword_flags {
	uint		dict:1 ;
	uint		fold:1 ;
	uint		word:1 ;
} ;

struct lookword_head {
	uint		magic ;
	cchar		*md ;
	size_t		ms ;
	int		fd ;
	LOOKWORD_FL	f ;
} ;

struct lookword_word {
	cchar		*wp ;
	int		wl ;
} ;

struct lookword_cur {
	LOOKWORD_WORD	*ans ;
	int		n ;
	int		i ;		/* when enumerating */
} ;


#if	(! defined(LOOKWORD_MASTER)) || (LOOKWORD_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int lookword_open(LOOKWORD *,cchar *,int) ;
extern int lookword_close(LOOKWORD *) ;
extern int lookword_curbegin(LOOKWORD *,LOOKWORD_CUR *) ;
extern int lookword_lookup(LOOKWORD *,LOOKWORD_CUR *,cchar *) ;
extern int lookword_read(LOOKWORD *,LOOKWORD_CUR *,char *,int) ;
extern int lookword_curend(LOOKWORD *,LOOKWORD_CUR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* LOOKWORD_MASTER */

#endif /* LOOKWORD_INCLUDE */


