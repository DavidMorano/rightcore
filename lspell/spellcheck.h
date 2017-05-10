/* spellcheck */


/* revision history:

	- 2008-10-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	SPELLCHECK_INCLUDE
#define	SPELLCHECK_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>
#include	<modload.h>
#include	<bits.h>

#include	"spellchecks.h"


#define	SPELLCHECK_MAGIC	0x97677247
#define	SPELLCHECK		struct spellcheck_head
#define	SPELLCHECK_CUR		struct spellcheck_c
#define	SPELLCHECK_CS		struct spellcheck_calls


struct spellcheck_c {
	uint		magic ;
	void		*scp ;
} ;

struct spellcheck_calls {
	int	(*start)(void *,const char *,const char *) ;
	int	(*count)(void *) ;
	int	(*look)(void *,const char *,int) ;
	int	(*looks)(void *,BITS *,const char **,int) ;
	int	(*curbegin)(void *,SPELLCHECKS_CUR *) ;
	int	(*enumerate)(void *,SPELLCHECKS_CUR *,char *,int) ;
	int	(*curend)(void *,SPELLCHECKS_CUR *) ;
	int	(*audit)(void *) ;
	int	(*finish)(void *) ;
} ;

struct spellcheck_head {
	uint		magic ;
	MODLOAD		loader ;
	void		*obj ;		/* object pointer */
	SPELLCHECK_CS	call ;
	int		objsize ;
	int		cursize ;
} ;


#if	(! defined(SPELLCHECK_MASTER)) || (SPELLCHECK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	spellcheck_start(SPELLCHECK *,const char *,const char *) ;
extern int	spellcheck_count(SPELLCHECK *) ;
extern int	spellcheck_look(SPELLCHECK *,const char *,int) ;
extern int	spellcheck_looks(SPELLCHECK *,BITS *,const char **,int) ;
extern int	spellcheck_curbegin(SPELLCHECK *,SPELLCHECK_CUR *) ;
extern int	spellcheck_enum(SPELLCHECK *,SPELLCHECK_CUR *,char *,int) ;
extern int	spellcheck_curend(SPELLCHECK *,SPELLCHECK_CUR *) ;
extern int	spellcheck_audit(SPELLCHECK *) ;
extern int	spellcheck_finish(SPELLCHECK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SPELLCHECK_MASTER */

#endif /* SPELLCHECK */


