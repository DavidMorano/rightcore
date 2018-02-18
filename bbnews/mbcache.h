/* mbcache */

/* mailbox cache */


/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

#ifndef	MBCACHE_INCLUDE
#define	MBCACHE_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vecint.h>
#include	<strpack.h>
#include	<dater.h>
#include	<date.h>
#include	<localmisc.h>

#include	"mailbox.h"


#define	MBCACHE_MAGIC		0x31415927
#define	MBCACHE			struct mbcache_head
#define	MBCACHE_FLAGS		struct mbcache_flags
#define	MBCACHE_INFO		struct mbcache_info
#define	MBCACHE_SCAN		struct mbcache_scan
#define	MBCACHE_SFLAGS		struct mbcache_sflags

/* open options */
#define	MBCACHE_ORDWR		MAILBOX_ORDWR

#define	MBCACHE_MFVREAD		0		/* MSG has been read */
#define	MBCACHE_MFVDEL		1		/* MSG marked for deletion */
#define	MBCACHE_MFVSPAM		2		/* MSG marked as spam */
#define	MBCACHE_MFVTRASH	3		/* MSG marked as trash */

#define	MBCACHE_MFMREAD		(1<<MBCACHE_MFVREAD)
#define	MBCACHE_MFMDEL		(1<<MBCACHE_MFVDEL)
#define	MBCACHE_MFMSPAM		(1<<MBCACHE_MFVSPAM)
#define	MBCACHE_MFMTRASH	(1<<MBCACHE_MFVTRASH)


enum mbcachemfs {
	mbcachemf_envaddr,
	mbcachemf_envdate,
	mbcachemf_envremote,
	mbcachemf_hdrmid,
	mbcachemf_hdrfrom,
	mbcachemf_hdrdate,
	mbcachemf_hdrsubject,
	mbcachemf_hdrstatus,
	mbcachemf_scanfrom,
	mbcachemf_scansubject,
	mbcachemf_scandate,
	mbcachemf_scanline,
	mbcachemf_overlast
} ;

struct mbcache_info {
	int		nmsgs ;		/* mailbox messages total */
	int		nmsgdels ;	/* mailbox messages deleted */
} ;

struct mbcache_sflags {
	uint		info:1 ;	/* msg-info loaded */
	uint		vs:1 ;		/* values (extended) initialized */
	uint		lineoffs:1 ;	/* 'lineoffs' initialized */
	uint		edate:1 ;	/* date-envelope initialized */
	uint		hdate:1 ;	/* date-header initialized */
	uint		clen:1 ;
	uint		clines:1 ;
	uint		lines:1 ;
	uint		xlines:1 ;
	uint		status:1 ;	/* have STATUS header */
	uint		msgid:1 ;	/* have MSGID header */
	uint		sem:1 ;		/* have SEM header */
	uint		read:1 ;	/* MSG has been read */
	uint		del:1 ;		/* MSG marked for deletion */
	uint		spam:1 ;	/* MSG marked as spam */
	uint		trash:1 ;	/* MSG marked as trash (trashed) */
	uint		scanfrom:1 ;
	uint		scansubject:1 ;
	uint		scandate:1 ;
} ;

struct mbcache_scan {
	const char	*vs[mbcachemf_overlast] ;
	const char	*fname ;	/* processed content file */
	vecint		lineoffs ;
	MBCACHE_SFLAGS	hdr, hdrval, proc, f ;
	DATE		edate ;		/* date-envelope */
	DATE		hdate ;		/* date-header */
	offset_t	moff ;		/* offset message start (envelope) */
	offset_t	hoff ;		/* offset message headers */
	offset_t	boff ;		/* offset message body */
	time_t		etime ;		/* time-envelope */
	time_t		htime ;		/* time-header */
	int		vl[mbcachemf_overlast] ;
	int		mlen ;		/* length message whole */
	int		hlen ;		/* length message headers */
	int		blen ;		/* length message body */
	int		filesize ;	/* processed content file */
	int		nlines ;	/* message lines-native */
	int		vlines ; 	/* message lines-view */
	int		msgi ;		/* message index */
} ;

struct mbcache_flags {
	uint		readonly:1 ;
} ;

struct mbcache_head {
	uint		magic ;
	MAILBOX		*mbp ;
	const char	*mbfname ;
	MBCACHE_SCAN	**msgs ;
	MBCACHE_FLAGS	f ;
	MAILBOX_INFO	mbi ;
	STRPACK		strs ;
	DATER		dm ;		/* date-manager */
	int		mflags ;	/* mailbox open-flags */
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int mbcache_start(MBCACHE *,const char *,int,MAILBOX *) ;
extern int mbcache_mbfile(MBCACHE *,char *,int) ;
extern int mbcache_mbinfo(MBCACHE *,MBCACHE_INFO *) ;
extern int mbcache_count(MBCACHE *) ;
extern int mbcache_countdel(MBCACHE *) ;
extern int mbcache_sort(MBCACHE *) ;
extern int mbcache_msgoff(MBCACHE *,int,offset_t *) ;
extern int mbcache_msglines(MBCACHE *,int,int *) ;
extern int mbcache_msginfo(MBCACHE *,int,MBCACHE_SCAN **) ;
extern int mbcache_msgscan(MBCACHE *,int,MBCACHE_SCAN **) ;
extern int mbcache_msghdrtime(MBCACHE *,int,time_t *) ;
extern int mbcache_msgenvtime(MBCACHE *,int,time_t *) ;
extern int mbcache_msgtimes(MBCACHE *,int,time_t *) ;
extern int mbcache_msgflags(MBCACHE *,int) ;
extern int mbcache_msgsetflag(MBCACHE *,int,int,int) ;
extern int mbcache_msgsetlines(MBCACHE *,int,int) ;
extern int mbcache_msgdel(MBCACHE *,int,int) ;
extern int mbcache_msgdeldup(MBCACHE *) ;
extern int mbcache_finish(MBCACHE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MBCACHE_INCLUDE */


