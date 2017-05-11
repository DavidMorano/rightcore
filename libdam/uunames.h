/* uunames */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	UUNAMES_INCLUDE
#define	UUNAMES_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vecobj.h>
#include	<localmisc.h>


#define	UUNAMES_MAGIC	0x99889298
#define	UUNAMES		struct uunames_head
#define	UUNAMES_OBJ	struct uunames_obj
#define	UUNAMES_CUR	struct uunames_c
#define	UUNAMES_FL	struct uunames_flags


struct uunames_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct uunames_c {
	int		i ;
} ;

struct uunames_flags {
	uint		varind:1 ;
} ;

struct uunames_head {
	uint		magic ;
	const char	*pr ;
	const char	*dbname ;
	const char	*indfname ;		/* index file-name */
	caddr_t		indfmap ;		/* index file-map */
	UUNAMES_FL	f ;
	vecobj		list ;
	time_t		ti_mod ;		/* DB file modification */
	time_t		ti_map ;		/* map */
	time_t		ti_lastcheck ;
	int		indfsize ;		/* index file-size */
	int		ncursors ;
} ;


#if	(! defined(UUNAMES_MASTER)) || (UUNAMES_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int uunames_open(UUNAMES *,const char *,const char *) ;
extern int uunames_count(UUNAMES *) ;
extern int uunames_exists(UUNAMES *,const char *,int) ;
extern int uunames_curbegin(UUNAMES *,UUNAMES_CUR *) ;
extern int uunames_enum(UUNAMES *,UUNAMES_CUR *,char *,int) ;
extern int uunames_curend(UUNAMES *,UUNAMES_CUR *) ;
extern int uunames_audit(UUNAMES *) ;
extern int uunames_close(UUNAMES *) ;

#ifdef	__cplusplus
extern "C" {
#endif

#endif /* UUNAMES_MASTER */

#endif /* UUNAMES_INCLUDE */


