/* mailalias */


/* revision history:

	= 2003-06-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILALIAS_INCLUDE
#define	MAILALIAS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>		/* for 'uino_t' */
#include	<vecstr.h>
#include	<ids.h>
#include	<localmisc.h>


/* object defines */

#define	MAILALIAS		struct mailalias_head
#define	MAILALIAS_INFO		struct mailalias_i
#define	MAILALIAS_CUR		struct mailalias_c
#define	MAILALIAS_FI		struct mailalias_fi
#define	MAILALIAS_FL		struct mailalias_flags

#define	MAILALIAS_MAGIC		0x23456187
#define	MAILALIAS_FILEMAGIC	"MAILALIAS"
#define	MAILALIAS_FILEMAGICSIZE	16
#define	MAILALIAS_FILEMAGICLEN	sizeof(MAILALIAS_FILEMAGIC)

#define	MAILALIAS_FILEVERSION	0
#define	MAILALIAS_FILETYPE	0

#define	MAILALIAS_DEFAPTAB	"default"

#define	MAILALIAS_OSEC		(1<<0)		/* use secondard hash */
#define	MAILALIAS_ORANDLC	(1<<1)		/* use 'randlc()' */


struct mailalias_c {
	uint		magic ;
	int		i ;
} ;

struct mailalias_i {
	time_t		wtime ;		/* time DB written */
	uint		wcount ;	/* write counter */
	uint		entries ;	/* total number of entries */
	uint		version ;
	uint		encoding ;
	uint		type ;
	uint		collisions ;
} ;

struct mailalias_fi {
	uino_t		ino ;
	dev_t		dev ;
	time_t		mtime ;
	size_t		size ;
} ;

struct mailalias_flags {
	uint		remote:1 ;
	uint		ocreate:1 ;
	uint		owrite:1 ;
	uint		fileinit:1 ;
	uint		cursorlockbroken:1 ;
	uint		cursoracc:1 ;
	uint		held:1 ;
	uint		lockedread:1 ;
	uint		lockedwrite:1 ;
	uint		needcreate:1 ;
} ;

struct mailalias_head {
	uint		magic ;
	const char	**aprofile ;
	const char	*pr ;
	const char	*apname ;
	const char	*dbfname ;
	const char	*skey ;
	const char	*sval ;
	int		*keytab ;
	int		(*rectab)[2] ;
	int		(*indtab)[2] ;
	MAILALIAS_FI	fi ;
	MAILALIAS_FL	f ;
	cchar		*mapdata ;
	vecstr		apfiles ;
	IDS		id ;
	time_t		ti_aprofile ;
	time_t		ti_open ;
	time_t		ti_access ;
	time_t		ti_map ;
	time_t		ti_check ;
	time_t		ti_filecheck ;
	time_t		ti_fileold ;
	size_t		mapsize ;
	mode_t		operm ;
	int		pagesize ;
	int		sklen ;
	int		svlen ;
	int		ktlen ;
	int		rtlen, rilen ;
	int		collisions ;
	int		cursors ;
	int		fd ;
	int		oflags, otype ;
	int		ropts ;
} ;


#if	(! defined(MAILALIAS_MASTER)) || (MAILALIAS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	mailalias_open(MAILALIAS *,cchar *,cchar *,int,mode_t,int) ;
extern int	mailalias_info(MAILALIAS *,MAILALIAS_INFO *) ;
extern int	mailalias_curbegin(MAILALIAS *,MAILALIAS_CUR *) ;
extern int	mailalias_enum(MAILALIAS *,MAILALIAS_CUR *,
			char *,int,char *,int) ;
extern int	mailalias_fetch(MAILALIAS *,int,cchar *,
			MAILALIAS_CUR *,char *,int) ;
extern int	mailalias_curend(MAILALIAS *,MAILALIAS_CUR *) ;
extern int	mailalias_count(MAILALIAS *) ;
extern int	mailalias_indcount(MAILALIAS *) ;
extern int	mailalias_audit(MAILALIAS *) ;
extern int	mailalias_close(MAILALIAS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILALIAS_MASTER */

#endif /* MAILALIAS_INCLUDE */


