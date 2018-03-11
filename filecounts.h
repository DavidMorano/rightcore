/* filecounts */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	FILECOUNTS_INCLUDE
#define	FILECOUNTS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#define	FILECOUNTS_MAGIC	0x22436893
#define	FILECOUNTS_NUMDIGITS	9	
#define	FILECOUNTS_LOGZLEN	23	/* total (theoretical) length */

#define	FILECOUNTS		struct filecounts_head
#define	FILECOUNTS_N		struct filecounts_name
#define	FILECOUNTS_INFO		struct filecounts_info
#define	FILECOUNTS_CUR		struct filecounts_cur
#define	FILECOUNTS_II		struct filecounts_ii
#define	FILECOUNTS_FL		struct filecounts_flags


struct filecounts_ii {
	const char	*name ;
	time_t		utime ;
	int		value ;
} ;

struct filecounts_cur {
	uint		magic ;
	FILECOUNTS_II	*list ;
	int		nlist ;
	int		i ;
} ;

struct filecounts_info {
	time_t		utime ;		/* last update time */
	int		value ;		/* counter previous value */
} ;

struct filecounts_name {
	const char	*name ;		/* counter name */
	int		value ;		/* counter previous value */
} ;

struct filecounts_flags {
	uint		rdonly:1 ;
} ;

struct filecounts_head {
	uint		magic ;
	const char	*fname ;	/* file-name */
	FILECOUNTS_FL	f ;
	int		fd ;
	int		ncursors ;
} ;


#if	(! defined(FILECOUNTS_MASTER)) || (FILECOUNTS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int filecounts_open(FILECOUNTS *,const char *,int,mode_t) ;
extern int filecounts_process(FILECOUNTS *,FILECOUNTS_N *) ;
extern int filecounts_curbegin(FILECOUNTS *,FILECOUNTS_CUR *) ;
extern int filecounts_snap(FILECOUNTS *,FILECOUNTS_CUR *) ;
extern int filecounts_read(FILECOUNTS *,FILECOUNTS_CUR *,
		FILECOUNTS_INFO *,char *,int) ;
extern int filecounts_curend(FILECOUNTS *,FILECOUNTS_CUR *) ;
extern int filecounts_close(FILECOUNTS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* FILECOUNTS_MASTER */

#endif /* FILECOUNTS_INCLUDE */


