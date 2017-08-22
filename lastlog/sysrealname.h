/* sysrealname */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSREALNAME_INCLUDE
#define	SYSREALNAME_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<modload.h>
#include	<ipasswd.h>
#include	<localmisc.h>


/* object defines */

#define	SYSREALNAME		struct sysrealname_head
#define	SYSREALNAME_CUR		struct sysrealname_c
#define	SYSREALNAME_INFO	struct sysrealname_i
#define	SYSREALNAME_CA		struct sysrealname_calls
#define	SYSREALNAME_MAGIC	0x88776216
#define	SYSREALNAME_CURMAGIC	0x88776217
#define	SYSREALNAME_PR		"/usr/extra"
#define	SYSREALNAME_DBNAME	"/sys/realname"


struct sysrealname_c {
	uint		magic ;
	void		*scp ;
	const char	**sa ;
	int		sn ;
	int		fo ;		/* options */
} ;

struct sysrealname_i {
	time_t		writetime ;	/* time DB written */
	uint		writecount ;	/* write counter */
	uint		entries ;	/* total number of entries */
	uint		version ;
	uint		encoding ;
	uint		type ;
	uint		collisions ;
} ;

struct sysrealname_calls {
	int	(*open)(void *,const char *) ;
	int	(*info)(void *,IPASSWD_INFO *) ;
	int	(*curbegin)(void *,IPASSWD_CUR *) ;
	int	(*curend)(void *,IPASSWD_CUR *) ;
	int	(*enumerate)(void *,IPASSWD_CUR *,char *,cchar **,char *,int) ;
	int	(*fetcher)(void *,IPASSWD_CUR *,int,char *,cchar **,int) ;
	int	(*audit)(void *) ;
	int	(*close)(void *) ;
} ;

struct sysrealname_head {
	uint		magic ;
	MODLOAD		loader ;
	SYSREALNAME_CA	call ;
	void		*obj ;		/* object pointer */
	int		objsize ;	/* object size */
	int		cursize ;	/* cursor size */
} ;


typedef struct sysrealname_head	sysrealname ;


#if	(! defined(SYSREALNAME_MASTER)) || (SYSREALNAME_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sysrealname_open(SYSREALNAME *,const char *) ;
extern int sysrealname_curbegin(SYSREALNAME *,SYSREALNAME_CUR *) ;
extern int sysrealname_curend(SYSREALNAME *,SYSREALNAME_CUR *) ;
extern int sysrealname_look(SYSREALNAME *,SYSREALNAME_CUR *,int,
		cchar *,int) ;
extern int sysrealname_lookparts(SYSREALNAME *,SYSREALNAME_CUR *,int,
		cchar **,int) ;
extern int sysrealname_lookread(SYSREALNAME *,SYSREALNAME_CUR *,char *) ;
extern int sysrealname_enum(SYSREALNAME *,SYSREALNAME_CUR *,char *,
		cchar **,char *,int) ;
extern int sysrealname_audit(SYSREALNAME *) ;
extern int sysrealname_close(SYSREALNAME *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSREALNAME_MASTER */

#endif /* SYSREALNAME_INCLUDE */


