/* strstore */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	STRSTORE_INCLUDE
#define	STRSTORE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vechand.h>
#include	<lookaside.h>
#include	<hdb.h>
#include	<localmisc.h>


/* object defines */

#define	STRSTORE_MAGIC		0x42114682
#define	STRSTORE		struct strstore_head
#define	STRSTORE_CUR		struct strstore_cur
#define	STRSTORE_CHUNK		struct strstore_chunk

#define	STRSTORE_STARTLEN	10
#define	STRSTORE_CHUNKSIZE	512


struct strstore_cur {
	int		i ;
} ;

struct strstore_chunk {
	char		*cdata ;
	int		csize ;		/* allocated buffer length */
	int		i ;		/* index length */
	int		c ;		/* item count within chunk */
} ;

struct strstore_head {
	uint		magic ;
	STRSTORE_CHUNK	*ccp ;		/* current chunk pointer */
	vechand		chunks ;
	vechand		list ;		/* so we can index strings by number */
	LOOKASIDE	imgr ;		/* integer-manager */
	HDB		smgr ;		/* string-manager */
	int		chunksize ;
	int		totalsize ;
	int		c ;		/* total count */
} ;


typedef struct strstore_head	strstore ;
typedef struct strstore_cur	strstore_cur ;


#if	(! defined(STRSTORE_MASTER)) || (STRSTORE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	strstore_start(STRSTORE *,int,int) ;
extern int	strstore_already(STRSTORE *,const char *,int) ;
extern int	strstore_add(STRSTORE *,const char *,int) ;
extern int	strstore_adduniq(STRSTORE *,const char *,int) ;
extern int	strstore_store(STRSTORE *,const char *,int,const char **) ;
extern int	strstore_curbegin(STRSTORE *,STRSTORE_CUR *) ;
extern int	strstore_enum(STRSTORE *,STRSTORE_CUR *,const char **) ;
extern int	strstore_curend(STRSTORE *,STRSTORE_CUR *) ;
extern int	strstore_count(STRSTORE *) ;
extern int	strstore_size(STRSTORE *) ;
extern int	strstore_finish(STRSTORE *) ;

extern int	strstore_strsize(STRSTORE *) ;
extern int	strstore_strmk(STRSTORE *,char *,int) ;
extern int	strstore_recsize(STRSTORE *) ;
extern int	strstore_recmk(STRSTORE *,int *,int) ;
extern int	strstore_indlen(STRSTORE *) ;
extern int	strstore_indsize(STRSTORE *) ;
extern int	strstore_indmk(STRSTORE *,int (*)[3],int,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* STRSTORE_MASTER */

#endif /* STRSTORE_INCLUDE */


