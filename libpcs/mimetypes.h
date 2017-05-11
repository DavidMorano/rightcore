/* mimetypes */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MIMETYPES_INCLUDE
#define	MIMETYPES_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<hdb.h>


/* object defines */

#define	MIMETYPES		HDB
#define	MIMETYPES_CUR		HDB_CUR

#define	MIMETYPES_TYPELEN	MAXNAMELEN


#if	(! defined(MIMETYPES_MASTER)) || (MIMETYPES_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	mimetypes_start(MIMETYPES *) ;
extern int	mimetypes_finish(MIMETYPES *) ;
extern int	mimetypes_file(MIMETYPES *,const char *) ;
extern int	mimetypes_curbegin(MIMETYPES *,MIMETYPES_CUR *) ;
extern int	mimetypes_curend(MIMETYPES *,MIMETYPES_CUR *) ;
extern int	mimetypes_enum(MIMETYPES *,MIMETYPES_CUR *,char *,char *) ;
extern int	mimetypes_fetch(MIMETYPES *,const char *,MIMETYPES_CUR *,
			char *) ;
extern int	mimetypes_find(MIMETYPES *,char *,const char *) ;
extern int	mimetypes_get(MIMETYPES *,char *,const char *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MIMETYPES_MASTER */

#endif /* MIMETYPES_INCLUDE */


