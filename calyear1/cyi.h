/* cyi */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CYI_INCLUDE
#define	CYI_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<localmisc.h>

#include	"cyihdr.h"		/* this is the hash-file-header */
#include	"calcite.h"


#define	CYI_MAGIC	0x88773421
#define	CYI		struct cyi_head
#define	CYI_OBJ		struct cyi_obj
#define	CYI_Q		CALCITE
#define	CYI_QUERY	CALCITE
#define	CYI_CUR		struct cyi_c
#define	CYI_ENT		struct cyi_e
#define	CYI_LINE	struct cyi_l
#define	CYI_INFO	struct cyi_i
#define	CYI_FMI		struct cyi_fmi


/* this is the shared-object description */
struct cyi_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct cyi_i {
	time_t		ctime ;		/* index create-time */
	time_t		mtime ;		/* index modification-time */
	uint		count ;
	uint		year ;		/* year index was made for */
} ;

struct cyi_l {
	uint		loff ;
	uint		llen ;
} ;

struct cyi_e {
	CYI_LINE	*lines ;
	uint		voff ;
	uint		vlen ;
	uint		hash ;
	ushort		y ;
	uchar		nlines, m, d ;	/* maximum 255 lines per entry */
} ;

struct cyi_c {
	uint		magic ;
	uint		citekey ;
	int		i ;
} ;

struct cyi_fmi {
	void		*mapdata ;	/* file map */
	time_t		ti_mod ;	/* time file modication */
	time_t		ti_map ;	/* time file map */
	size_t		mapsize ;
	uint		(*vt)[5] ;	/* mapped entries table */
	uint		(*lt)[2] ;	/* mapped lines table */
} ;

struct cyi_head {
	uint		magic ;
	cchar		*fname ;
	CYI_FMI		fmi ;		/* file-map information */
	CYIHDR		fhi ;		/* file-header information */
	time_t		ti_lastcheck ;	/* time last check of file */
	int		ncursors ;
	int		year ;
} ;


#if	(! defined(CYI_MASTER)) || (CYI_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	cyi_open(CYI *,int,cchar *,cchar *) ;
extern int	cyi_count(CYI *) ;
extern int	cyi_info(CYI *,CYI_INFO *) ;
extern int	cyi_curbegin(CYI *,CYI_CUR *) ;
extern int	cyi_lookcite(CYI *,CYI_CUR *,CYI_QUERY *) ;
extern int	cyi_read(CYI *,CYI_CUR *,CYI_ENT *,char *,int) ;
extern int	cyi_enum(CYI *,CYI_CUR *,CYI_ENT *,char *,int) ;
extern int	cyi_curend(CYI *,CYI_CUR *) ;
extern int	cyi_audit(CYI *) ;
extern int	cyi_close(CYI *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CYI_MASTER */

#endif /* CYI_INCLUDE */


