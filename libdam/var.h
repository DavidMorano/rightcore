/* var */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VAR_INCLUDE
#define	VAR_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<modload.h>
#include	<localmisc.h>

#include	"vars.h"


#define	VAR_MAGIC	0x99447246
#define	VAR		struct var_head
#define	VAR_CUR		struct var_c
#define	VAR_INFO	struct var_i
#define	VAR_CALLS	struct var_calls

#define	VARINFO		struct varinfo


struct var_i {
	time_t		wtime ;
	time_t		mtime ;
	uint		nvars ;
	uint		nskip ;
} ;

struct var_c {
	uint		magic ;
	void		*scp ;		/* SO-cursor pointer */
} ;

struct var_calls {
	int	(*open)(void *,const char *) ;
	int	(*opena)(void *,const char **) ;
	int	(*count)(void *) ;
	int	(*curbegin)(void *,void *) ;
	int	(*fetch)(void *,const char *,int,void *,char *,int) ;
	int	(*enumerate)(void *,void *,char *,int,char *,int) ;
	int	(*curend)(void *,void *) ;
	int	(*info)(void *,VARS_INFO *) ;
	int	(*audit)(void *) ;
	int	(*close)(void *) ;
} ;

struct var_head {
	uint		magic ;
	MODLOAD		loader ;
	VAR_CALLS	call ;
	void		*obj ;		/* object pointer */
	int		objsize ;	/* object size */
	int		cursize ;	/* cursor size */
} ;

struct varinfo {
	long		size ;
	time_t		mtime ;
} ;


#if	(! defined(VAR_MASTER)) || (VAR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	var_open(VAR *,const char *) ;
extern int	var_opena(VAR *,const char **) ;
extern int	var_count(VAR *) ;
extern int	var_curbegin(VAR *,VAR_CUR *) ;
extern int	var_fetch(VAR *,const char *,int,VAR_CUR *,char *,int) ;
extern int	var_enum(VAR *,VAR_CUR *,char *,int,char *,int) ;
extern int	var_curend(VAR *,VAR_CUR *) ;
extern int	var_info(VAR *,VAR_INFO *) ;
extern int	var_audit(VAR *) ;
extern int	var_close(VAR *) ;

extern int	varinfo(VARINFO *,const char *,int) ;
extern int	varunlink(const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* VAR_MASTER */

#endif /* VAR_INCLUDE */


