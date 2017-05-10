/* configvars */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CONFIGVARS_INCLUDE
#define	CONFIGVARS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<time.h>

#include	<vecitem.h>
#include	<localmisc.h>


/* object defines */

#define	CONFIGVARS_MAGIC	0x04311633
#define	CONFIGVARS		struct configvars_head
#define	CONFIGVARS_ERR		struct configvars_errline
#define	CONFIGVARS_VAR		struct configvars_v
#define	CONFIGVARS_FILE		struct configvars_file
#define	CONFIGVARS_CUR		struct configvars_c

#define	CONFIGVARS_NFILES	(sizeof(int) * 8)


struct paramfile_c {
	int		i ;
} ;

struct configvars_head {
	uint		magic ;		/* magic number */
	vecitem		fes ;		/* file entries */
	vecitem		defines ;	/* collapsed defined variables */
	vecitem		unsets ;	/* unset ENV variables */
	vecitem		exports ;	/* collapsed environment variables */
	vecitem		sets ;		/* "set" variables */
	vecitem		vars ;		/* the user's variables */
	time_t		checktime ;
} ;

struct configvars_file {
	const char	*filename ;
	vecitem		defines ;	/* defined variables */
	vecitem		unsets ;	/* unset ENV variables */
	vecitem		exports ;	/* environment variables */
	time_t		mtime ;
} ;

struct configvars_errline {
	int		line ;
	char		filename[MAXPATHLEN + 1] ;
} ;

struct configvars_vflags {
	uint		varsubed:1 ;	/* variable substituted */
	uint		expanded:1 ;	/* key expanded */
} ;

struct configvars_v {
	char		*ma ;		/* memory allocation (for ARGV) */
	char		*key, *value ;
	char		**argv ;
	struct configvars_vflags	f ;
	int		argc ;
	int		fi ;		/* file index of source */
	int		fmask ;		/* file mask of dependencies */
	int		klen, vlen ;
} ;


#ifdef	COMMENT

typedef struct configvars_head		configvars ;
typedef struct configvars_v		configvars_var ;

#endif /* COMMENT */


#if	(! defined(CONFIGVARS_MASTER)) || (CONFIGVARS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

int configvars_open(CONFIGVARS *,const char *,VECITEM *) ;
int configvars_addfile(CONFIGVARS *,const char *,VECITEM *) ;
int configvars_check(CONFIGVARS *,time_t) ;
int configvars_curbegin(CONFIGVARS *,CONFIGVARS_CUR *) ;
int configvars_curend(CONFIGVARS *,CONFIGVARS_CUR *) ;
int configvars_fetch(CONFIGVARS *,const char *,CONFIGVARS_CUR *,const char **) ;
int configvars_close(CONFIGVARS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CONFIGVARS */

#endif /* CONFIGVARS_INCLUDE */


