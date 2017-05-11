/* biblepara */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	BIBLEPARA_INCLUDE
#define	BIBLEPARA_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<modload.h>
#include	<localmisc.h>

#include	"bibleparas.h"


#define	BIBLEPARA_MAGIC		0x99447246
#define	BIBLEPARA		struct biblepara_head
#define	BIBLEPARA_QUERY		struct biblepara_q
#define	BIBLEPARA_CITE		struct biblepara_q
#define	BIBLEPARA_Q		struct biblepara_q
#define	BIBLEPARA_CUR		struct biblepara_c
#define	BIBLEPARA_INFO		struct biblepara_i
#define	BIBLEPARA_CALLS		struct biblepara_calls


struct biblepara_i {
	time_t	dbtime ;		/* db-time */
	time_t	vitime ;		/* vi-time */
	uint	maxbook ;
	uint	maxchapter ;
	uint	nverses ;
	uint	nzverses ;
} ;

struct biblepara_q {
	uchar	b, c, v ;
} ;

struct biblepara_c {
	uint	magic ;
	void	*scp ;
} ;

struct biblepara_calls {
	int	(*open)(void *,const char *,const char *) ;
	int	(*count)(void *) ;
	int	(*ispara)(void *,BIBLEPARAS_Q *) ;
	int	(*curbegin)(void *,BIBLEPARAS_CUR *) ;
	int	(*enumerate)(void *,BIBLEPARAS_CUR *,BIBLEPARAS_Q *) ;
	int	(*curend)(void *,BIBLEPARAS_CUR *) ;
	int	(*audit)(void *) ;
	int	(*info)(void *,BIBLEPARAS_INFO *) ;
	int	(*close)(void *) ;
} ;

struct biblepara_head {
	uint		magic ;
	MODLOAD		loader ;
	BIBLEPARA_CALLS	call ;
	void		*obj ;			/* object pointer */
	int		objsize ;		/* object size */
	int		cursize ;		/* cursor size */
} ;


#if	(! defined(BIBLEPARA_MASTER)) || (BIBLEPARA_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	biblepara_open(BIBLEPARA *,const char *,const char *) ;
extern int	biblepara_count(BIBLEPARA *) ;
extern int	biblepara_ispara(BIBLEPARA *,BIBLEPARA_Q *) ;
extern int	biblepara_curbegin(BIBLEPARA *,BIBLEPARA_CUR *) ;
extern int	biblepara_enum(BIBLEPARA *,BIBLEPARA_CUR *,BIBLEPARA_Q *) ;
extern int	biblepara_curend(BIBLEPARA *,BIBLEPARA_CUR *) ;
extern int	biblepara_audit(BIBLEPARA *) ;
extern int	biblepara_info(BIBLEPARA *,BIBLEPARA_INFO *) ;
extern int	biblepara_close(BIBLEPARA *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BIBLEPARA_MASTER */

#endif /* BIBLEPARA_INCLUDE */


