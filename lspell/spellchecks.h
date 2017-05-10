/* spellchecks */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */


#ifndef	SPELLCHECKS_INCLUDE
#define	SPELLCHECKS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vechand.h>
#include	<bits.h>
#include	<localmisc.h>


#define	SPELLCHECKS_MAGIC	0x99447246
#define	SPELLCHECKS_CONFNAME	"conf"
#define	SPELLCHECKS_SEARCHNAME	"spellchecks"
#define	SPELLCHECKS		struct spellchecks_head
#define	SPELLCHECKS_OBJ		struct spellchecks_obj
#define	SPELLCHECKS_CUR		struct spellchecks_c
#define	SPELLCHECKS_FL		struct spellchecks_flags


/* this is the shared-object description */
struct spellchecks_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct spellchecks_c {
	uint		magic ;
	void		*results ;
	uint		nresults ;
	int		i ;
} ;

struct spellchecks_flags {
	uint		vind:1 ;		/* index is loaded */
} ;

struct spellchecks_head {
	uint		magic ;
	const char	*pr ;
	const char	*tmpdname ;
	SPELLCHECKS_FL	f ;
	vechand		lists ;			/* spell-lists */
	int		nlists ;
	int		ncursors ;
} ;


#if	(! defined(SPELLCHECKS_MASTER)) || (SPELLCHECKS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int spellchecks_start(SPELLCHECKS *,const char *,const char *) ;
extern int spellchecks_count(SPELLCHECKS *) ;
extern int spellchecks_look(SPELLCHECKS *,const char *,int) ;
extern int spellchecks_looks(SPELLCHECKS *,BITS *,const char *,int) ;
extern int spellchecks_curbegin(SPELLCHECKS *,SPELLCHECKS_CUR *) ;
extern int spellchecks_enum(SPELLCHECKS *,SPELLCHECKS_CUR *,char *,int) ;
extern int spellchecks_curend(SPELLCHECKS *,SPELLCHECKS_CUR *) ;
extern int spellchecks_check(SPELLCHECKS *,time_t) ;
extern int spellchecks_audit(SPELLCHECKS *) ;
extern int spellchecks_finish(SPELLCHECKS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SPELLCHECKS_MASTER */


#endif /* SPELLCHECKS_INCLUDE */



