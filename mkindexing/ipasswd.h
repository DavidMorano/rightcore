/* ipasswd */


/* revision history:

	= 1998-03-31, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	IPASSWD_INCLUDE
#define	IPASSWD_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<realname.h>
#include	<localmisc.h>


/* object defines */

#define	IPASSWD			struct ipasswd_head
#define	IPASSWD_OBJ		struct ipasswd_obj
#define	IPASSWD_CUR		struct ipasswd_c
#define	IPASSWD_INFO		struct ipasswd_i
#define	IPASSWD_ENT		struct ipasswd_e
#define	IPASSWD_FL		struct ipasswd_flags

#define	IPASSWD_SUF		"pwi"
#define	IPASSWD_MAGIC		0x23456787
#define	IPASSWD_CURMAGIC	0x23456788

#define	IPASSWD_USERNAMELEN	32
#define	IPASSWD_NINDICES	5	/* number of indices */

/* database ("read") options */
#define	IPASSWD_ROSEC		0x01	/* use second hash */
#define	IPASSWD_RORANDLC	0x02	/* use 'randlc()' */

/* fetch options */
#define	IPASSWD_FOLASTFULL	0x01	/* require full last name for match */


struct ipasswd_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct ipasswd_c {
	uint		magic ;
	int		wi ;
	int		i[IPASSWD_NINDICES] ;
} ;

struct ipasswd_i {
	time_t		writetime ;	/* time DB written */
	uint		writecount ;	/* write counter */
	uint		entries ;	/* total number of entries */
	uint		version ;
	uint		encoding ;
	uint		type ;
	uint		collisions ;
} ;

struct ipasswd_e {
	uint		username ;
	uint		last ;
	uint		first ;
	uint		m1 ;
	uint		m2 ;
} ;

struct ipasswd_flags {
	uint		remote:1 ;
	uint		cursor:1 ;
	uint		cursorlockbroken:1 ;
	uint		cursoracc:1 ;
	uint		held:1 ;
	uint		lockedread:1 ;
	uint		lockedwrite:1 ;
} ;

struct ipasswd_head {
	uint		magic ;
	const char	*fname ;
	caddr_t		mapdata ;
	const char	*stab ;
	uint		(*recind[IPASSWD_NINDICES])[2] ;
	IPASSWD_ENT	*rectab ;
	IPASSWD_FL	f ;
	time_t		mtime ;
	time_t		ti_open ;
	time_t		ti_access ;
	time_t		ti_map ;
	size_t		mapsize ;
	uint		pagesize ;
	uint		filesize ;
	uint		stcount ;
	uint		stsize ;
	uint		rtlen ;
	uint		rtsize ;
	uint		rilen ;
	uint		collisions ;
	uint		ropts ;
	int		fd ;
	int		oflags ;
	int		operm ;
} ;


#if	(! defined(IPASSWD_MASTER)) || (IPASSWD_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ipasswd_open(IPASSWD *,const char *) ;
extern int	ipasswd_info(IPASSWD *,IPASSWD_INFO *) ;
extern int	ipasswd_curbegin(IPASSWD *,IPASSWD_CUR *) ;
extern int	ipasswd_curend(IPASSWD *,IPASSWD_CUR *) ;
extern int	ipasswd_enum(IPASSWD *,IPASSWD_CUR *,char *,cchar **,
			char *,int) ;
extern int	ipasswd_fetcher(IPASSWD *,IPASSWD_CUR *,int,char *,cchar **,
			int) ;
extern int	ipasswd_fetch(IPASSWD *,REALNAME *,IPASSWD_CUR *,int,char *) ;
extern int	ipasswd_close(IPASSWD *) ;

#ifdef	__cplusplus
}
#endif

#endif /* IPASSWD_MASTER */

#endif /* IPASSWD_INCLUDE */


