/* chartrans */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CHARTRANS_INCLUDE
#define	CHARTRANS_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<stddef.h>
#include	<uiconv.h>
#include	<localmisc.h>


#define	CHARTRANS	struct chartrans_head
#define	CHARTRANS_SET	struct chartrans_set
#define	CHARTRANS_FL	struct chartrans_flags
#define	CHARTRANS_MAGIC	0x67298361
#define	CHARTRANS_NCS	"UCS-4"			/* native character-set */


struct chartrans_flags {
	uint		dummy:1 ;
} ;

struct chartrans_set {
	const char	*name ;
	UICONV		id ;			/* converter */
	time_t		ti_access ;		/* access time */
	uint		acount ;		/* access time-stamp */
	uint		uc ;			/* usage-count */
	int		pc ;			/* pass-set */
} ;

struct chartrans_head {
	uint		magic ;
	CHARTRANS_FL	f ;
	CHARTRANS_SET	*sets ;
	const char	*pr ;
	void		*utf8decoder ;
	int		nmax ;
	int		nsets ;
	int		acount ;
} ;


#if	(! defined(CHARTRANS_MASTER)) || (CHARTRANS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int chartrans_open(CHARTRANS *,cchar *,int) ;
extern int chartrans_close(CHARTRANS *) ;
extern int chartrans_transbegin(CHARTRANS *,time_t,cchar *,int) ;
extern int chartrans_transend(CHARTRANS *,int) ;
extern int chartrans_transread(CHARTRANS *,int,wchar_t *,int,cchar *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(CHARTRANS_MASTER)) || (CHARTRANS_MASTER == 0) */

#endif /* CHARTRANS_INCLUDE */


