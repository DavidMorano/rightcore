/* sysvar */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSVAR_INCLUDE
#define	SYSVAR_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecstr.h>
#include	<modload.h>
#include	<localmisc.h>

#include	"sysvars.h"


#define	SYSVAR_MAGIC	0x99447243
#define	SYSVAR		struct sysvar_head
#define	SYSVAR_CUR	struct sysvar_c
#define	SYSVAR_CALLS	struct sysvar_calls
#define	SYSVAR_FL	struct sysvar_flags

#define	SYSVAR_OPREFIX	SYSVARS_OPREFIX		/* prefix match */


struct sysvar_c {
	uint	magic ;
	void	*scp ;		/* SO-cursor pointer */
} ;

struct sysvar_calls {
	int	(*open)(void *,const char *,const char *) ;
	int	(*count)(void *) ;
	int	(*curbegin)(void *,void *) ;
	int	(*fetch)(void *,const char *,int,void *,char *,int) ;
	int	(*enumerate)(void *,void *,char *,int,char *,int) ;
	int	(*curend)(void *,void *) ;
	int	(*audit)(void *) ;
	int	(*close)(void *) ;
} ;

struct sysvar_flags {
	uint		defaults:1 ;
} ;

struct sysvar_head {
	uint		magic ;
	void		*obj ;			/* object pointer */
	SYSVAR_CALLS	call ;
	SYSVAR_FL	f ;
	MODLOAD		loader ;
	vecstr		defaults ;
	int		objsize ;		/* object size */
	int		cursize ;		/* cursor size */
} ;


#if	(! defined(SYSVAR_MASTER)) || (SYSVAR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sysvar_open(SYSVAR *,const char *,const char *) ;
extern int sysvar_count(SYSVAR *) ;
extern int sysvar_curbegin(SYSVAR *,SYSVAR_CUR *) ;
extern int sysvar_fetch(SYSVAR *,const char *,int,SYSVAR_CUR *,char *,int) ;
extern int sysvar_enum(SYSVAR *,SYSVAR_CUR *,char *,int,char *,int) ;
extern int sysvar_curend(SYSVAR *,SYSVAR_CUR *) ;
extern int sysvar_audit(SYSVAR *) ;
extern int sysvar_close(SYSVAR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSVAR_MASTER */

#endif /* SYSVAR_INCLUDE */


