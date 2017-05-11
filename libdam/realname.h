/* REALNAME */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	REALNAME_INCLUDE
#define	REALNAME_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<dstr.h>
#include	<localmisc.h>


/* object defines */

#define	REALNAME		struct realname_head
#define	REALNAME_NNAMES		5
#define	REALNAME_NPARTS		REALNAME_NNAMES

#if	defined(REALNAMELEN)
#define	REALNAME_STORELEN	MAX(REALNAMELEN,100)
#else
#define	REALNAME_STORELEN	100
#endif /* defined(REALNAMELEN) */


struct realname_abv {
	uint		first:1 ;
	uint		m1:1 ;
	uint		m2:1 ;
	uint		m3:1 ;
	uint		last:1 ;
} ;

struct realname_len {
	uchar		first ;
	uchar		m1 ;
	uchar		m2 ;
	uchar		m3 ;
	uchar		last ;
	uchar		store ;
} ;

struct realname_head {
	const char	*first ;
	const char	*m1 ;
	const char	*m2 ;
	const char	*m3 ;
	const char	*last ;
	struct realname_len	len ;
	struct realname_abv	abv ;
	char		store[REALNAME_STORELEN + 1] ;
} ;


typedef struct realname_head	realname ;


#if	(! defined(REALNAME_MASTER)) || (REALNAME_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	realname_start(REALNAME *,const char *,int) ;
extern int	realname_startparts(REALNAME *,DSTR *) ;
extern int	realname_startpieces(REALNAME *,const char **,int) ;
extern int	realname_startparse(REALNAME *,const char *,int) ;
extern int	realname_getlast(REALNAME *,const char **) ;
extern int	realname_getfirst(REALNAME *,const char **) ;
extern int	realname_getm1(REALNAME *,const char **) ;
extern int	realname_getm2(REALNAME *,const char **) ;
extern int	realname_getm3(REALNAME *,const char **) ;
extern int	realname_getpieces(REALNAME *,const char **) ;
extern int	realname_fullname(REALNAME *,char *,int) ;
extern int	realname_name(REALNAME *,char *,int) ;
extern int	realname_mailname(REALNAME *,char *,int) ;
extern int	realname_mat(REALNAME *,REALNAME *) ;
extern int	realname_matlast(REALNAME *,char *,int) ;
extern int	realname_matfirst(REALNAME *,char *,int) ;
extern int	realname_matm1(REALNAME *,char *,int) ;
extern int	realname_matm2(REALNAME *,char *,int) ;
extern int	realname_finish(REALNAME *) ;

#ifdef	__cplusplus
}
#endif

#endif /* REALNAME_MASTER */

#endif /* REALNAME_INCLUDE */


