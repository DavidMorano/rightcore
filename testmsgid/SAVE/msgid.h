/* msgid */

/* machine status file */


#ifndef	MSGID_INCLUDE
#define	MSGID_INCLUDE	1



#include	<sys/types.h>

#include	<mapstrint.h>

#include	"misc.h"
#include	"msgide.h"



/* object defines */

#define	MSGID			struct msgid_head
#define	MSGID_CURSOR		struct msgid_c
#define	MSGID_ENTRY		MSGIDE_ALL


#define	MSGID_FILEMAGICA	"MSGIDA"
#define	MSGID_FILEMAGICB	"MSGIDB"

#define	MSGID_FILEVERSION	0
#define	MSGID_ENDIAN		1	/* big endian */






struct msgid_vetu {
	uchar	v, e, t, u ;
} ;

struct msgid_fid {
	char			magic[16] ;
	struct msgid_vetu	id ;
} ;

struct msgid_h {
	uint	nentries ;
	uint	wtime ;
	uint	wcount ;
} ;

struct msgid_flags {
	uint	fileinit : 1 ;		/* file init'ed */
	uint	writable : 1 ;
	uint	readlocked : 1 ;
	uint	writelocked : 1 ;
	uint	cursorlockbroken : 1 ;	/* cursor lock broken */
	uint	cursoracc : 1 ;		/* accessed while cursored ? */
	uint	remote : 1 ;		/* remote mounted file */
	uint	bufvalid : 1 ;		/* buffer valid */
} ;

struct msgid_filemap {
	char	*fname ;
	char	*buf ;
	struct msgid_vetu	id ;
	time_t	otime ;
	time_t	mtime ;
	time_t	atime ;			/* our last access */
	int	filesize ;
	int	bufsize ;		/* this is the mapped size */
	int	buflen ;
	int	fd ;
} ;

struct msgid_filelock {
	char	*fname ;
	struct msgid_vetu	id ;
	time_t	otime ;			/* file open time */
	time_t	mtime ;			/* file modification time */
	time_t	atime ;			/* our last access */
	int	fd ;
} ;

struct msgid_head {
	unsigned long	magic ;
	char		*bufheader ;	/* header entries */
	char		*buftable ;	/* table entries */
	struct msgid_flags	f ;
	struct msgid_filemap	fmap ;
	struct msgid_filelock	flock ;
	struct msgid_h		h ;
	MAPSTRINT		ni ;	/* nodename index */
	int	oflags, operm ;
	int	pagesize ;
	int	fda, fdb ;
	int	cursors ;
} ;

struct msgid_c {
	int	i ;
} ;




#if	(! defined(MSGID_MASTER)) || (MSGID_MASTER == 0)

extern int msgid_open(MSGID *,const char *,int,int) ;
extern int msgid_cursorinit(MSGID *,MSGID_CURSOR *) ;
extern int msgid_cursorfree(MSGID *,MSGID_CURSOR *) ;
extern int msgid_enum(MSGID *,MSGID_CURSOR *,MSGID_ENTRY *) ;
extern int msgid_matchid(MSGID *,time_t,const char *,int,MSGID_ENTRY *) ;
extern int msgid_update(MSGID *,time_t,MSGID_ENTRY *) ;
extern int msgid_check(MSGID *,time_t) ;
extern int msgid_count(MSGID *) ;
extern int msgid_close(MSGID *) ;

#endif /* MSGID_MASTER */


#endif /* MSGID_INCLUDE */



