/* quote */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */


#ifndef	QUOTE_INCLUDE
#define	QUOTE_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vechand.h>
#include	<vecstr.h>

#include	"localmisc.h"



#define	QUOTE	struct quote_head

#define	QUOTE_OBJ	struct quote_obj
#define	QUOTE_CUR	struct quote_c

#define	QUOTE_OPREFIX	0x01		/* perform a "prefix" match */



/* this is the shared-object description */
struct quote_obj {
	char		*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct quote_c {
	ulong		magic ;
	void		*results ;
	uint		nresults ;
	int		i ;
} ;

struct quote_flags {
	uint		vind : 1 ;		/* index is loaded */
} ;

struct quote_head {
	ulong		magic ;
	const char	*pr ;
	const char	*tmpdname ;
	char		**dirnames ;
	char		*dirstrtab ;
	struct quote_flags	f ;
	vechand		dirs ;			/* directories */
	vecstr		tmpfiles ;
	int		nentries ;
	int		ncursors ;
} ;



#if	(! defined(QUOTE_MASTER)) || (QUOTE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int quote_open(QUOTE *,const char *,
			const char **,const char **) ;
extern int quote_count(QUOTE *) ;
extern int quote_curbegin(QUOTE *,QUOTE_CUR *) ;
extern int quote_lookup(QUOTE *,QUOTE_CUR *,int,const char **) ;
extern int quote_read(QUOTE *,QUOTE_CUR *,char *,int) ;
extern int quote_curend(QUOTE *,QUOTE_CUR *) ;
extern int quote_check(QUOTE *,time_t) ;
extern int quote_audit(QUOTE *) ;
extern int quote_close(QUOTE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* QUOTE_MASTER */


#endif /* QUOTE_INCLUDE */



