/* densitydb */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	DENSITYDB_INCLUDE
#define	DENSITYDB_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<netdb.h>
#include	<time.h>

#include	<sockaddress.h>
#include	<endian.h>
#include	<localmisc.h>

#include	"densitydbe.h"


#define	DENSITYDB_MAGIC		1092847456
#define	DENSITYDB		struct densitydb_head
#define	DENSITYDB_KEY		struct densitydb_k
#define	DENSITYDB_CUR		struct densitydb_c
#define	DENSITYDB_FL		struct densitydb_flags
#define	DENSITYDB_FH		struct densitydb_filehead
#define	DENSITYDB_FM		struct densitydb_filemagic
#define	DENSITYDB_BUF		struct densitydb_buffer

/* other defines */

#define	DENSITYDB_FILEVERSION	0
#define	DENSITYDB_ENDIAN	ENDIAN	/* endian */
#define	DENSITYDB_ENT		DENSITYDBE_ALL


/* file buffer state */
struct densitydb_buffer {
	char	*buf ;			/* fixed buffer */
	uint	size ;			/* fixed buffer size */
	uint	off ;			/* file offset of valid area */
	uint	len ;			/* length of valid area */
} ;

/* decoded file magic */
struct densitydb_filemagic {
	char	magic[16] ;
	uchar	vetu[4] ;
} ;

/* decoded file header values */
struct densitydb_filehead {
	uint	wcount ;
	uint	wtime ;
	uint	nentries ;
} ;

struct densitydb_flags {
	uint		fileinit:1 ;		/* file init'ed */
	uint		writable:1 ;
	uint		readlocked:1 ;
	uint		writelocked:1 ;
	uint		cursorlockbroken:1 ;	/* cursor lock broken */
	uint		cursoracc:1 ;		/* accessed while cursored ? */
	uint		remote:1 ;		/* remote mounted file */
} ;

struct densitydb_head {
	unsigned long	magic ;
	char		*fname ;
	DENSITYDB_FL	f ;
	DENSITYDB_FH	h ;
	DENSITYDB_BUF	b ;
	time_t		opentime ;	/* file open time */
	time_t		accesstime ;	/* file access time */
	time_t		mtime ;		/* file modification time */
	mode_t		om ;
	int		oflags ;
	int		maxentry ;
	int		pagesize ;
	int		filesize ;
	int		fd ;
	int		cursors ;
	int		ebs ;		/* entry buffer size */
} ;


#if	(! defined(DENSITYDB_MASTER)) || (DENSITYDB_MASTER == 0)

extern int densitydb_open(DENSITYDB *,cchar *,int,int,int) ;
extern int densitydb_check(DENSITYDB *,time_t) ;
extern int densitydb_close(DENSITYDB *) ;
extern int densitydb_enum(DENSITYDB *,DENSITYDB_CUR *,DENSITYDB_ENT *) ;
extern int densitydb_match(DENSITYDB *,time_t,int,DENSITYDB_ENT *) ;
extern int densitydb_update(DENSITYDB *,time_t,int,DENSITYDB_ENT *) ;
extern int densitydb_write(DENSITYDB *,time_t,int,uint) ;

#endif /* DENSITYDB_MASTER */

#endif /* DENSITYDB_INCLUDE */


