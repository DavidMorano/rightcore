/* txtindexmks */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	TXTINDEXMKS_INCLUDE
#define	TXTINDEXMKS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<strtab.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"txtindexhdr.h"		/* this is the hash-file-header */


#define	TXTINDEXMKS_MAGIC	0x88773422
#define	TXTINDEXMKS		struct txtindexmks_head
#define	TXTINDEXMKS_FL		struct txtindexmks_flags
#define	TXTINDEXMKS_OBJ		struct txtindexmks_obj
#define	TXTINDEXMKS_PA		struct txtindexmks_pa
#define	TXTINDEXMKS_TAG		struct txtindexmks_tag
#define	TXTINDEXMKS_KEY		struct txtindexmks_k
#define	TXTINDEXMKS_TI		struct txtindexmks_ti
#define	TXTINDEXMKS_INTOPEN	(10*60)
#define	TXTINDEXMKS_INTSTALE	(5*60)

#define	TXTINDEXMKS_MINWLEN	3
#define	TXTINDEXMKS_MAXWLEN	10


/* this is the shared-object description */
struct txtindexmks_obj {
	const char	*name ;
	uint		objsize ;
} ;

struct txtindexmks_pa {
	const char	*sdn ;
	const char	*sfn ;
	uint		tablen ;	/* hash-table length */
	uint		minwlen ;	/* minimum key-word length */
	uint		maxwlen ;	/* maximum key-word length */
} ;

struct txtindexmks_k {
	const char	*kp ;
	int		kl ;
} ;

struct txtindexmks_tag {
	TXTINDEXMKS_KEY	*keys ;
	const char	*fname ;
	uint		recoff ;
	uint		reclen ;
	uint		nkeys ;
} ;

struct txtindexmks_ti {
	uint		nkeys ;		/* total number of keys */
	uint		ntags ;		/* total number of tags */
	uint		maxtags ;	/* maximum tags per list */
} ;

struct txtindexmks_flags {
	uint		tagopen:1 ;
	uint		ofcreat:1 ;
	uint		ofexcl:1 ;
	uint		inprogress:1 ;
	uint		created:1 ;
	uint		abort:1 ;
} ;

struct txtindexmks_head {
	uint		magic ;
	const char 	*dbname ;
	const char	*idname ;
	char		*nfname ;
	char		*nidxfname ;	/* hash index */
	char		*ntagfname ;	/* tags */
	void		*lists ;	/* the lists */
	TXTINDEXMKS_PA	pi ;
	TXTINDEXMKS_TI	ti ;
	TXTINDEXMKS_FL	f ;
	STRTAB		eigens ;
	bfile		tagfile ;
	mode_t		om ;
	uint		tagoff ;	/* tag-file running offset */
	uint		tagsize ;	/* tag-file size (after completed) */
	int		nfd ;
	int		clists ;
} ;


#if	(! defined(TXTINDEXMKS_MASTER)) || (TXTINDEXMKS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int txtindexmks_open(TXTINDEXMKS *,TXTINDEXMKS_PA *,cchar *,int,mode_t) ;
extern int txtindexmks_addeigens(TXTINDEXMKS *,TXTINDEXMKS_KEY *,int) ;
extern int txtindexmks_addtags(TXTINDEXMKS *,TXTINDEXMKS_TAG *,int) ;
extern int txtindexmks_noop(TXTINDEXMKS *) ;
extern int txtindexmks_abort(TXTINDEXMKS *) ;
extern int txtindexmks_close(TXTINDEXMKS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TXTINDEXMKS_MASTER */

#endif /* TXTINDEXMKS_INCLUDE */


