/* cmi */

/* ComMand Index object */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CMI_INCLUDE
#define	CMI_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>
#include	"cmihdr.h"		/* this is the hash-file-header */


#define	CMI_MAGIC	0x88773427
#define	CMI_SUF		"cmi"		/* variable-index */

#define	CMI		struct cmi_head

#define	CMI_OBJ		struct cmi_obj
#define	CMI_CUR		struct cmi_c
#define	CMI_ENT		struct cmi_e
#define	CMI_LINE	struct cmi_l
#define	CMI_INFO	struct cmi_i
#define	CMI_FMI		struct cmi_fmi


/* this is the shared-object description */
struct cmi_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct cmi_i {
	time_t		idxctime ;	/* IDX creation-time */
	time_t		idxmtime ;	/* IDX modification-time */
	time_t		dbtime ;	/* DB-file time */
	size_t		dbsize ;	/* DB-file size */
	size_t		idxsize ;	/* IDX-file size */
	uint		nents ;
	uint		maxent ;
} ;

struct cmi_l {
	uint		loff ;
	uint		llen ;
} ;

struct cmi_e {
	CMI_LINE	*lines ;
	uint		eoff ;
	uint		elen ;
	ushort		nlines ;
	ushort		cn ;
} ;

struct cmi_c {
	int		i ;
} ;

struct cmi_fmi {
	char		*mapdata ;	/* file map */
	time_t		ti_mod ;	/* time file modication */
	time_t		ti_map ;	/* time file map */
	size_t		mapsize ;
	uint		(*vt)[4] ;	/* mapped verses table */
	uint		(*lt)[2] ;	/* mapped lines table */
} ;

struct cmi_head {
	uint		magic ;
	const char 	*dbname ;
	const char	*fname ;
	CMI_FMI		fmi ;		/* file-map information */
	CMIHDR		fhi ;		/* file-header information */
	time_t		ti_lastcheck ;	/* time last check of file */
	int		ncursors ;
} ;


#if	(! defined(CMI_MASTER)) || (CMI_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	cmi_open(CMI *,const char *) ;
extern int	cmi_count(CMI *) ;
extern int	cmi_info(CMI *,CMI_INFO *) ;
extern int	cmi_read(CMI *,CMI_ENT *,char *,int,uint) ;
extern int	cmi_curbegin(CMI *,CMI_CUR *) ;
extern int	cmi_enum(CMI *,CMI_CUR *,CMI_ENT *,char *,int) ;
extern int	cmi_curend(CMI *,CMI_CUR *) ;
extern int	cmi_audit(CMI *) ;
extern int	cmi_close(CMI *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CMI_MASTER */

#endif /* CMI_INCLUDE */


