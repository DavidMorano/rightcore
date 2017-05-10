/* outema */

/* output lines */


#ifndef	OUTEMA_INCLUDE
#define	OUTEMA_INCLUDE	1


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

	= 1999-02-01, David A­D­ Morano
	I added a little code to "post" articles that do not have a valid
	newsgroup to a special "dead article" directory in the BB spool area.

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/****************************************************************************

	This object deals with printing lines.


****************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ema.h>
#include	<filebuf.h>
#include	<localmisc.h>


#define	OUTEMA		struct outema_head
#define	OUTEMA_FL	struct outema_flags


struct outema_flags {
	uint		comma:1 ;
} ;

struct outema_head {
	FILEBUF		*ofp ;
	OUTEMA_FL	f ;
	int		maxlen ;
	int		rlen ;
	int		llen ;
	int		wlen ;
	int		c_values ;
	int		c_items ;
} ;


#if	(! defined(OUTEMA_MASTER)) || (OUTEMA_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	outema_start(OUTEMA *,FILEBUF *,int) ;
extern int	outema_ent(OUTEMA *,EMA_ENT *) ;
extern int	outema_write(OUTEMA *,const char *,int) ;
extern int	outema_hdrkey(OUTEMA *,const char *) ;
extern int	outema_item(OUTEMA *,const char *,int) ;
extern int	outema_value(OUTEMA *,const char *,int) ;
extern int	outema_needlength(OUTEMA *,int) ;
extern int	outema_finish(OUTEMA *) ;

#ifdef	COMMENT
extern int	outema_printf(OUTEMA *,const char *,...) ;
#endif /* COMMENT */

#ifdef	__cplusplus
}
#endif

#endif	/* (! defined(OUTEMA_MASTER)) || (OUTEMA_MASTER == 0) */

#endif	/* OUTEMA_INCLUDE */


