/* biblebook */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */


#ifndef	BIBLEBOOK_INCLUDE
#define	BIBLEBOOK_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<modload.h>
#include	<localmisc.h>


#define	BIBLEBOOK_MAGIC		0x99447242
#define	BIBLEBOOK		struct biblebook_head
#define	BIBLEBOOK_CALLS		struct biblebook_calls
#define	BIBLEBOOK_FL		struct biblebook_flags
#define	BIBLEBOOK_LEN		80 /* bible-book-name length */
#define	BIBLEBOOK_NBOOKS	66


struct biblebook_calls {
	int	(*open)(void *,const char *,const char *) ;
	int	(*count)(void *) ;
	int	(*max)(void *) ;
	int	(*lookup)(void *,char *,int,int) ;
	int	(*get)(void *,int,char *,int) ;
	int	(*match)(void *,const char *,int) ;
	int	(*size)(void *) ;
	int	(*audit)(void *) ;
	int	(*close)(void *) ;
} ;

struct biblebook_flags {
	int		localdb:1 ;	/* using local DB */
} ;

struct biblebook_head {
	uint		magic ;
	MODLOAD		loader ;
	BIBLEBOOK_CALLS	call ;
	BIBLEBOOK_FL	f ;
	void		*obj ;		/* object pointer */
	const char	**names ;
	const char	*namestrs ;
	int		objsize ;	/* object size */
	int		namesize ;	/* names-size */
} ;


#if	(! defined(BIBLEBOOK_MASTER)) || (BIBLEBOOK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	biblebook_open(BIBLEBOOK *,const char *,const char *) ;
extern int	biblebook_count(BIBLEBOOK *) ;
extern int	biblebook_max(BIBLEBOOK *) ;
extern int	biblebook_read(BIBLEBOOK *,char *,int,int) ;
extern int	biblebook_lookup(BIBLEBOOK *,char *,int,int) ;
extern int	biblebook_get(BIBLEBOOK *,int,char *,int) ;
extern int	biblebook_match(BIBLEBOOK *,const char *,int) ;
extern int	biblebook_size(BIBLEBOOK *) ;
extern int	biblebook_audit(BIBLEBOOK *) ;
extern int	biblebook_close(BIBLEBOOK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BIBLEBOOK_MASTER */

#endif /* BIBLEBOOK_INCLUDE */


