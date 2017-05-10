/* postfile */



#ifndef	POSTFILE_INCLUDE
#define	POSTFILE_INCLUDE	1


#include	<sys/types.h>
#include	<time.h>

#include	<realname.h>

#include	"localmisc.h"



/* object defines */

#define	POSTFILE		struct postfile_head
#define	POSTFILE_INFO		struct postfile_i
#define	POSTFILE_CUR		struct postfile_c
#define	POSTFILE_ENT		struct postfile_e


#define	POSTFILE_FILEMAGIC	"POSTINDEX"
#define	POSTFILE_FILEMAGICLEN	9

#define	POSTFILE_FILEVERSION	0
#define	POSTFILE_FILETYPE	0

#define	POSTFILE_USERNAMELEN	32
#define	POSTFILE_NINDICES	3

#define	POSTFILE_OSEC		0x01		/* use secondard hash */
#define	POSTFILE_ORANDLC	0x02		/* use 'randlc()' */

#define	POSTFILE_FLASTFULL	0x01		/* full last name */




struct postfile_c {
	unsigned long	magic ;
	int	wi ;
	int	i[POSTFILE_NINDICES] ;
} ;

struct postfile_i {
	time_t	writetime ;	/* time DB written */
	uint	writecount ;	/* write counter */
	uint	entries ;	/* total number of entries */
	uint	version ;
	uint	encoding ;
	uint	type ;
	uint	collisions ;
} ;

struct postfile_e {
	uint	username ;
	uint	last ;
	uint	first ;
	uint	m1 ;
	uint	m2 ;
} ;

struct postfile_flags {
	uint	remote : 1 ;
	uint	fileinit : 1 ;
	uint	cursor : 1 ;
	uint	cursorlockbroken : 1 ;
	uint	cursoracc : 1 ;
	uint	held : 1 ;
	uint	lockedread : 1 ;
	uint	lockedwrite : 1 ;
} ;

struct postfile_head {
	unsigned long		magic ;
	char			*fname ;
	caddr_t			mapbuf ;
	const char		*stab ;
	POSTFILE_ENT		*rectab ;
	uint			(*recind[POSTFILE_NINDICES])[2] ;
	struct postfile_flags	f ;
	time_t			mtime ;
	time_t			opentime ;
	time_t			accesstime ;
	uint			pagesize ;
	uint			mapsize ;
	uint			filesize ;
	uint			stlen ;
	uint			rtlen, rilen ;
	uint			collisions ;
	int		fd ;
	int		oflags, operm ;
	int		ropts ;
} ;



#if	(! defined(POSTFILE_MASTER)) || (POSTFILE_MASTER == 0)

extern int	postfile_open(POSTFILE *,const char *) ;
extern int	postfile_info(POSTFILE *,POSTFILE_INFO *) ;
extern int	postfile_curbegin(POSTFILE *,POSTFILE_CUR *) ;
extern int	postfile_curend(POSTFILE *,POSTFILE_CUR *) ;
extern int	postfile_enum(POSTFILE *,POSTFILE_CUR *,
			REALNAME *,char *) ;
extern int	postfile_fetch(POSTFILE *, REALNAME *, POSTFILE_CUR *,
			int,char *) ;
extern int	postfile_close(POSTFILE *) ;

#endif /* POSTFILE_MASTER */


#endif /* POSTFILE_INCLUDE */



