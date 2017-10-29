/* paramfile */

/* object to handle parameter files */


/* revision history:

	= 2000-02-17, David A­D­ Morano
	This code was started for Levo related work.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	PARAMFILE_INCLUDE
#define	PARAMFILE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<vecobj.h>
#include	<vecstr.h>
#include	<varsub.h>
#include	<localmisc.h>


/* object defines */

#define	PARAMFILE		struct paramfile_head
#define	PARAMFILE_FL		struct paramfile_flags
#define	PARAMFILE_ENT		struct paramfile_e
#define	PARAMFILE_CUR		struct paramfile_c
#define	PARAMFILE_ERROR		struct paramfile_errline

#define	PARAMFILE_MAGIC		0x12349876
#define	PARAMFILE_INTCHECK	2	/* file check interval (seconds) */
#define	PARAMFILE_INTCHANGE	2	/* wait change interval (seconds) */


struct paramfile_flags {
	uint		envinit:1 ;
	uint		envload:1 ;
	uint		definit:1 ;
	uint		defload:1 ;
} ;

struct paramfile_head {
	uint		magic ;
	cchar		**envv ;	/* program startup environment */
	cchar		*a ;		/* memory allocation */
	char		*lbuf ;
	char		*fbuf ;
	VECOBJ		files ;
	VECOBJ		entries ;	/* parameter entries */
	VARSUB		d, e ;
	PARAMFILE_FL	f ;
	time_t		ti_check ;	/* time last checked */
	int		llen ;
	int		flen ;
	int		intcheck ;
} ;

struct paramfile_e {
	cchar		*key ;
	cchar		*oval ;
	cchar		*value ;	/* dynamic variable expansion */
	int		fi ;
	int		klen ;
	int		olen ;
	int		vlen ;
} ;

struct paramfile_c {
	int		i ;
} ;


typedef struct paramfile_head	paramfile ;
typedef struct paramfile_e	paramfile_ent ;
typedef struct paramfile_c	paramfile_cur ;


#if	(! defined(PARAMFILE_MASTER)) || (PARAMFILE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int paramfile_open(PARAMFILE *,cchar **,cchar *) ;
extern int paramfile_fileadd(PARAMFILE *,cchar *) ;
extern int paramfile_setdefines(PARAMFILE *,VECSTR *) ;
extern int paramfile_curbegin(PARAMFILE *,PARAMFILE_CUR *) ;
extern int paramfile_curend(PARAMFILE *,PARAMFILE_CUR *) ;
extern int paramfile_fetch(PARAMFILE *,cchar *,PARAMFILE_CUR *,char *,int) ;
extern int paramfile_enum(PARAMFILE *,PARAMFILE_CUR *,
		PARAMFILE_ENT *,char *,int) ;
extern int paramfile_checkint(PARAMFILE *,int) ;
extern int paramfile_check(PARAMFILE *,time_t) ;
extern int paramfile_close(PARAMFILE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PARAMFILE_MASTER */

#endif /* PARAMFILE_INCLUDE */


