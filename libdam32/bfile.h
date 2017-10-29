/* bfile */

/* header file for the Basic I/O library package */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This file was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	BFILE_INCLUDE
#define	BFILE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<localmisc.h>


#define	BFILE		struct bfile_head
#define	BFILE_BD	struct bfile_bd
#define	BFILE_BDFLAGS	struct bfile_bdflags
#define	BFILE_MAP	struct bfile_map
#define	BFILE_FLAGS	struct bfile_flags

#define	BFILE_MAGIC	0x20052615
#define	BFILE_BUFPAGES	16
#define	BFILE_FDCH	'*'
#define	BFILE_FDNAMELEN	22
#define	BFILE_MAXNEOF	2		/* maximum EOFs on networks */
#define	BFILE_NMAPS	32		/* number of pages mapped at a time */

#define	BFILE_IDXSTDIN		0
#define	BFILE_IDXSTDOUT		1
#define	BFILE_IDXSTDERR		2
#define	BFILE_IDXSTDNULL	3

#define	BFILE_STDIN	bfile_fnames[0]
#define	BFILE_STDOUT	bfile_fnames[1]
#define	BFILE_STDERR	bfile_fnames[2]
#define	BFILE_STDNULL	bfile_fnames[3]
#define	BFILE_NULL	bfile_fnames[3]

#define	BFILE_DEBUGFNAME	"bfile.deb"

#if	defined(SYSHAS_OFFSET) && (SYSHAS_OFFSET != 0)
#define	BFILE_OFF	offset_t
#else
#define	BFILE_OFF	off_t
#endif
#define	BFILE_STAT	struct ustat


/* buffering modes */
enum bfile_bms {
	bfile_bmall,
	bfile_bmwhole,
	bfile_bmline,
	bfile_bmnone,
	bfile_bmoverlast
} ;

struct bfile_mapflags {
	unsigned int	valid:1 ;
	unsigned int	dirty:1 ;
} ;

struct bfile_map {
	BFILE_OFF	offset ;	/* file offset for page */
	char		*buf ;
	struct bfile_mapflags	f ;
} ;

struct bfile_bdflags {
	unsigned int	dirty:1 ;	/* needs to be written back */
} ;

struct bfile_bd {
	BFILE_OFF	boff ;		/* base of buffer */
	BFILE_BDFLAGS	f ;
	char		*bdata ;	/* base of buffer */
	int		bsize ;		/* size of buffer */
	int		blen ;		/* length of data (buffer index) */
} ;

struct bfile_flags {
	uint		created:1 ;
	uint		write:1 ;
	uint		notseek:1 ;
	uint		terminal:1 ;
	uint		network:1 ;
	uint		linein:1 ;
	uint		cancelled:1 ;
	uint		mappable:1 ;
	uint		mapinit:1 ;
	uint		mapped:1 ;
	uint		nullfile:1 ;
} ;

struct bfile_head {
	uint		magic ;
	BFILE_FLAGS	f ;
	BFILE_OFF	offset ; 	/* user view */
	BFILE_MAP	*maps ;		/* array of map pages */
	BFILE_BD	*bds ;		/* buffer descriptors */
	char		*bdata ;	/* allocated buffer space */
	char		*bbp ;		/* base buffer pointer */
	char		*bp ;		/* current pointer into buffer */
	LONG		size ;		/* current? file size */
	uino_t		ino ;
	dev_t		dev ;
	mode_t		mode ;
	int		fd ;
	int		pagesize ;	/* system page size */
	int		bsize ;		/* allocated buffer size */
	int		oflags ;	/* open flags */
	int		len ;		/* data remaining(r) or filled(w) */
	int		bm ;		/* buffer mode */
} ;


typedef struct bfile_head		bfile ;
typedef BFILE_STAT			bfile_stat ;


/* user commands to 'bcontrol' */

#define	BC_NOOP		0
#define	BC_TELL		1
#define	BC_BUF		2		/* perform buffering */
#define	BC_FULLBUF	BC_BUF
#define	BC_LINEBUF	3		/* do line buffering on output */
#define	BC_UNBUF	4		/* do no buffering */
#define	BC_NOBUF	BC_UNBUF
#define	BC_FD		5		/* get the file description */
#define	BC_STAT		6
#define	BC_TRUNCATE	7
#define	BC_CHMOD	8
#define	BC_SETLK	9		/* perform custom locking */
#define	BC_SETLKW	10
#define	BC_GETLK	11
#define	BC_UNLOCK	12		/* unlock any locks */
#define	BC_LOCKREAD	13		/* lock whole file for reads */
#define	BC_LOCKWRITE	14		/* lock whole file for writes */
#define	BC_LOCK		BC_LOCKWRITE
#define	BC_LINEIN	15		/* force input as lines only */
#define	BC_GETFL	16		/* get file flags */
#define	BC_SETFL	17		/* set file flags */
#define	BC_GETFDFL	18		/* get file descriptor flags */
#define	BC_SETFDFL	19		/* set file descriptor flags */
#define	BC_GETFD	BC_GETFDFL
#define	BC_SETFD	BC_SETFDFL
#define	BC_CLOSEONEXEC	20		/* set CLOSE-on-EXEC flag */
#define	BC_BUFSIZE	21		/* set buffer size */
#define	BC_DSYNC	22		/* sync data sync */
#define	BC_SYNC		23		/* sync data and attributes */
#define	BC_ISLINEBUF	24		/* get line-buffer status */
#define	BC_ISTERMINAL	25		/* get terminal status */
#define	BC_MINMOD	26		/* ensure minimum file mode */
#define	BC_SETBUFALL	27
#define	BC_SETBUFWHOLE	28
#define	BC_SETBUFLINE	29
#define	BC_SETBUFNONE	30
#define	BC_SETBUFDEF	31
#define	BC_GETBUFFLAGS	32
#define	BC_SETBUFFLAGS	33
#define	BC_NONBLOCK	34


/* flags */

#define	BFILE_FLINEIN	(1<<0)
#define	BFILE_FTERMINAL (1<<1)
#define	BFILE_FBUFWHOLE	(1<<2)
#define	BFILE_FBUFLINE	(1<<3)
#define	BFILE_FBUFNONE	(1<<4)
#define	BFILE_FBUFDEF	(1<<5)


#if	(! defined(BFILE_MASTER)) || (BFILE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bopen(bfile *,const char *,const char *,mode_t) ;
extern int	bopene(bfile *,const char *,const char *,mode_t,int) ;
extern int	bopenprog(bfile *,cchar *,cchar *,cchar **,cchar **) ;
extern int	bopentmp(bfile *,const char *,const char *,mode_t) ;
extern int	bopenmod(bfile *,const char *,const char *,mode_t) ;
extern int	bopenlock(bfile *,const char *,int,int) ;
extern int	bcontrol(bfile *,int,...) ;
extern int	bstat(bfile *,BFILE_STAT *) ;
extern int	bsize(bfile *) ;
extern int	bseek(bfile *,offset_t,int) ;
extern int	btell(bfile *,offset_t *) ;
extern int	bseek64(bfile *,off64_t,int) ;
extern int	btell64(bfile *,off64_t *) ;
extern int	brewind(bfile *) ;
extern int	breade(bfile *,void *,int,int,int) ;
extern int	bread(bfile *,void *,int) ;
extern int	breadlinetimed(bfile *,char *,int,int) ;
extern int	breadline(bfile *,char *,int) ;
extern int	breadlines(bfile *,char *,int,int *) ;
extern int	bgetc(bfile *) ;
extern int	bwrite(bfile *,const void *,int) ;
extern int	bwriteblock(bfile *,bfile *,int) ;
extern int	bwritefile(bfile *,const char *) ;
extern int	bputc(bfile *,int) ;
extern int	bprintf(bfile *,const char *,...) ;
extern int	bvprintf(bfile *,const char *,va_list) ;
extern int	bprintline(bfile *,const char *,int) ;
extern int	bcopyblock(bfile *,bfile *,int) ;
extern int	bcopyfile(bfile *,bfile *,char *,int) ;
extern int	btruncate(bfile *,offset_t) ;
extern int	breserve(bfile *,int) ;
extern int	bisterm(bfile *) ;
extern int	bminmod(bfile *,mode_t) ;
extern int	bflush(bfile *) ;
extern int	bflushn(bfile *,int) ;
extern int	bclose(bfile *) ;

extern int	bfile_flush(bfile *) ;
extern int	bfile_flushn(bfile *,int) ;
extern int	bfile_mktmpfile() ;
extern int	bfile_pagein() ;

extern int	bfilestat(const char *,int,BFILE_STAT *) ;
extern int	bfilefstat(int,BFILE_STAT *) ;

extern int	mkfdfname(char *,int) ;

extern const char	*bfile_fnames[] ;

#ifdef	__cplusplus
}
#endif

#endif /* BFILE_MASTER */

#endif /* BFILE_INCLUDE */


