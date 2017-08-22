/* srvreg */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SRVREG_INCLUDE
#define	SRVREG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<netdb.h>
#include	<time.h>

#include	<localmisc.h>

#include	"srvrege.h"


/* object defines */

#define	SRVREG_MAGIC		1092837456
#define	SRVREG			struct srvreg_head
#define	SRVREG_CUR		struct srvreg_c
#define	SRVREG_ENT		SRVREGE_ALL

#define	SRVREG_FILEPATH		"/tmp/srvreg"

#define	SRVREG_FILEMAGIC	"SRVREG"
#define	SRVREG_FILEMAGICLEN	strlen(SRVREG_FILEMAGIC)
#define	SRVREG_FILEVERSION	0
#define	SRVREG_ENDIAN		1

#define	SRVREG_TAGLEN		SRVREGE_LTAG
#define	SRVREG_SVCLEN		SRVREGE_LSVC
#define	SRVREG_SSLEN		SRVREGE_LSS
#define	SRVREG_HOSTLEN		SRVREGE_LHOST


/* file buffer state */
struct srvreg_buffer {
	char		*buf ;		/* fixed buffer */
	uint		size ;		/* fixed buffer size */
	uint		len ;		/* length of valid area */
	uint		off ;		/* file offset of valid area */
} ;

/* decoded file magic */
struct srvreg_filemagic {
	char		magic[16] ;
	uchar		vetu[4] ;
} ;

/* decoded file header values */
struct srvreg_filehead {
	uint		wcount ;
	uint		wtime ;
	uint		nentries ;
} ;

struct srvreg_flags {
	uint		fileinit:1 ;		/* file init'ed */
	uint		writable:1 ;
	uint		readlocked:1 ;
	uint		writelocked:1 ;
	uint		cursorlockbroken:1 ;	/* cursor lock broken */
	uint		cursoracc:1 ;		/* accessed while cursored? */
	uint		remote:1 ;		/* remote mounted file */
} ;

struct srvreg_head {
	uint		magic ;
	const char	*fname ;
	struct srvreg_flags	f ;
	struct srvreg_filehead	h ;
	struct srvreg_buffer	b ;	/* file buffer */
	time_t		opentime ;		/* file open time */
	time_t		accesstime ;		/* file access time */
	time_t		mtime ;			/* file modification time */
	int		oflags, operm ;
	int		pagesize ;
	int		filesize ;
	int		fd ;
	int		cursors ;
} ;

struct srvreg_c {
	int		i ;
} ;


#if	(! defined(SRVREG_MASTER)) || (SRVREG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

int srvreg_open(SRVREG *,const char *,int,int) ;
int srvreg_check(SRVREG *,time_t) ;
int srvreg_close(SRVREG *) ;
int srvreg_curbegin(SRVREG *,SRVREG_CUR *) ;
int srvreg_curend(SRVREG *,SRVREG_CUR *) ;
int srvreg_enum(SRVREG *,SRVREG_CUR *,SRVREG_ENT *) ;
int srvreg_fetchsvc(SRVREG *,const char *,SRVREG_CUR *,SRVREG_ENT *) ;
int srvreg_write(SRVREG *,int,SRVREG_ENT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SRVREG_MASTER */

#endif /* SRVREG_INCLUDE */


