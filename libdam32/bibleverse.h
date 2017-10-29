/* bibleverse */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written for hardware CAD support.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	BIBLEVERSE_INCLUDE
#define	BIBLEVERSE_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<modload.h>
#include	<localmisc.h>

#include	"bibleverses.h"


#define	BIBLEVERSE_MAGIC	0x99447246
#define	BIBLEVERSE		struct bibleverse_head
#define	BIBLEVERSE_QUERY	struct bibleverse_q
#define	BIBLEVERSE_Q		struct bibleverse_q
#define	BIBLEVERSE_CITE		struct bibleverse_q
#define	BIBLEVERSE_CUR		struct bibleverse_c
#define	BIBLEVERSE_INFO		struct bibleverse_i
#define	BIBLEVERSE_CALLS	struct bibleverse_calls
#define	BIBLEVERSE_C		struct bibleverse_calls


struct bibleverse_i {
	time_t	dbtime ;		/* db-time */
	time_t	vitime ;		/* vi-time */
	uint	maxbook ;
	uint	maxchapter ;
	uint	nverses ;
	uint	nzverses ;
} ;

struct bibleverse_q {
	uchar	b, c, v ;
} ;

struct bibleverse_c {
	uint	magic ;
	void	*scp ;
} ;

struct bibleverse_calls {
	int	(*open)(void *,const char *,const char *) ;
	int	(*count)(void *) ;
	int	(*read)(void *,char *,int,BIBLEVERSES_QUERY *) ;
	int	(*get)(void *,BIBLEVERSES_QUERY *,char *,int) ;
	int	(*curbegin)(void *,BIBLEVERSES_CUR *) ;
	int	(*enumerate)(void *,BIBLEVERSES_CUR *,
			BIBLEVERSES_QUERY *,char *,int) ;
	int	(*curend)(void *,BIBLEVERSES_CUR *) ;
	int	(*audit)(void *) ;
	int	(*info)(void *,BIBLEVERSES_INFO *) ;
	int	(*chapters)(void *,int,uchar *,int) ;
	int	(*close)(void *) ;
} ;

struct bibleverse_head {
	uint		magic ;
	MODLOAD		loader ;
	BIBLEVERSE_C	call ;
	void		*obj ;		/* object pointer */
	int		objsize ;	/* object size */
	int		cursize ;	/* cursor size */
} ;


#if	(! defined(BIBLEVERSE_MASTER)) || (BIBLEVERSE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bibleverse_open(BIBLEVERSE *,cchar *,cchar *) ;
extern int	bibleverse_count(BIBLEVERSE *) ;
extern int	bibleverse_read(BIBLEVERSE *,char *,int,BIBLEVERSE_Q *) ;
extern int	bibleverse_get(BIBLEVERSE *,BIBLEVERSE_Q *,char *,int) ;
extern int	bibleverse_curbegin(BIBLEVERSE *,BIBLEVERSE_CUR *) ;
extern int	bibleverse_enum(BIBLEVERSE *,BIBLEVERSE_CUR *,
			BIBLEVERSE_Q *,char *,int) ;
extern int	bibleverse_curend(BIBLEVERSE *,BIBLEVERSE_CUR *) ;
extern int	bibleverse_audit(BIBLEVERSE *) ;
extern int	bibleverse_info(BIBLEVERSE *,BIBLEVERSE_INFO *) ;
extern int	bibleverse_chapters(BIBLEVERSE *,int,uchar *,int) ;
extern int	bibleverse_close(BIBLEVERSE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BIBLEVERSE_MASTER */

#endif /* BIBLEVERSE_INCLUDE */


