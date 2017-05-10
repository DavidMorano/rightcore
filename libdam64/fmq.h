/* fmq */


/* revision history:

	= 1999-07-23, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	FMQ_INCLUDE
#define	FMQ_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>

#include	<localmisc.h>


#define	FMQ		struct fmq_head

#ifndef	UINT
#define	UINT		unsigned int
#endif


#define	FMQ_FILEMAGIC		"FMQ"
#define	FMQ_FILEMAGICLEN	3
#define	FMQ_FILEVERSION		0
#define	FMQ_ENDIAN		0


/* decoded file magic */
struct fmq_filemagic {
	char		magic[16] ;
	char		vetu[4] ;
} ;

/* decoded file header values */
struct fmq_filehead {
	uint		nmsg ;		/* number of messages */
	uint		wtime ;		/* write time */
	uint		wcount ;	/* write count */
	uint		size ;		/* total buffer size */
	uint		blen ;		/* buffer bytes used */
	uint		len ;		/* user bytes used */
	uint		ri, wi ;	/* read and write indices */
} ;

struct fmq_bufdesc {
	char		*buf ;
	uint		size ;		/* allocated size */
	uint		i ;		/* index to valid data */
	uint		len ;		/* length of valid data */
} ;
	
struct fmq_flags {
	uint		fileinit:1 ;
	uint		writable:1 ;
	uint		readlocked:1 ;
	uint		writelocked:1 ;
	uint		cursorlockbroken:1 ;	/* cursor lock broken */
	uint		cursoracc:1 ;		/* accessed while cursored ? */
	uint		remote:1 ;		/* remote mounted file */
	uint		bufvalid:1 ;		/* buffer valid */
	uint		create:1 ;
	uint		ndelay:1 ;
	uint		nonblock:1 ;
} ;

struct fmq_head {
	uint		magic ;
	const char	*fname ;
	struct fmq_flags	f ;
	struct fmq_filemagic	m ;
	struct fmq_filehead	h ;
	struct fmq_bufdesc	b ;
	sigset_t	sigmask ;
	time_t		opentime ;	/* file open time */
	time_t		accesstime ;	/* file access time */
	time_t		mtime ;		/* file modification time */
	int		oflags, operm ;
	int		pagesize ;
	int		filesize ;
	int		bufsize ;	/* user hint at open time */
	int		fd ;
	int		cursors ;
} ;


#if	(! defined(FMQ_MASTER)) || (FMQ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	fmq_open(FMQ *,const char *,int,int,int) ;
extern int	fmq_close(FMQ *) ;
extern int	fmq_send(FMQ *,const void *,int) ;
extern int	fmq_sende(FMQ *,const void *,int,int,int) ;
extern int	fmq_recv(FMQ *,void *,int) ;
extern int	fmq_recve(FMQ *,void *,int,int,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* FMQ_MASTER */

#endif /* FMQ_INCLUDE */


