/* cachetime */


/* revision history:

	= 2004-09-10, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

#ifndef	CACHETIME_INCLUDE
#define	CACHETIME_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>

#include	<hdb.h>
#include	<ptm.h>
#include	<localmisc.h>


#define	CACHETIME_MAGIC		0x79854123
#define	CACHETIME		struct cachetime_head
#define	CACHETIME_CUR		struct cachetime_c
#define	CACHETIME_STATS		struct cachetime_s


struct cachetime_s {
	uint		req, hit, miss ;
} ;

struct cachetime_c {
	HDB_CUR		cur ;
} ;

struct cachetime_e {
	const char	*name ;
	time_t		mtime ;
} ;

struct cachetime_head {
	uint		magic ;
	HDB		db ;
	PTM		m ;
	uint		c_req ;
	uint		c_hit ;
	uint		c_miss ;
} ;


#if	(! defined(CACHETIME_MASTER)) || (CACHETIME_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int cachetime_start(CACHETIME *) ;
extern int cachetime_lookup(CACHETIME *,const char *,int,time_t *) ;
extern int cachetime_curbegin(CACHETIME *,CACHETIME_CUR *) ;
extern int cachetime_curend(CACHETIME *,CACHETIME_CUR *) ;
extern int cachetime_enum(CACHETIME *,CACHETIME_CUR *,char *,int,time_t *) ;
extern int cachetime_stats(CACHETIME *,CACHETIME_STATS *) ;
extern int cachetime_finish(CACHETIME *) ;

#ifdef	__cplusplus
extern "C" {
#endif

#endif /* CACHETIME_MASTER */

#endif /* CACHETIME_INCLUDE */


