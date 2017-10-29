/* termtrans */

/* terminal-character-translation management */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	TERMTRANS_INCLUDE
#define	TERMTRANS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stddef.h>

#include	<uiconv.h>
#include	<localmisc.h>


/* object defines */

#define	TERMTRANS	struct termtrans_head
#define	TERMTRANS_FL	struct termtrans_flags
#define	TERMTRANS_MAGIC	0x67298363
#define	TERMTRANS_NCS	"UCS-4"


struct termtrans_flags {
	uint		dummy:1 ;
} ;

struct termtrans_head {
	uint		magic ;
	TERMTRANS_FL	f ;
	void		*cvp ;		/* character-vector-pointer */
	void		*lvp ;		/* line-vector-pointer */
	UICONV		id ;
	uint		termattr ;	/* mask of terminal attributes */
	int		ncols ;		/* terminal columns */
	int		ncol ;
} ;


#if	(! defined(TERMTRANS_MASTER)) || (TERMTRANS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int termtrans_start(TERMTRANS *,cchar *,cchar *,int,int) ;
extern int termtrans_load(TERMTRANS *,const wchar_t *,int) ;
extern int termtrans_getline(TERMTRANS *,int,cchar **) ;
extern int termtrans_finish(TERMTRANS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TERMTRANS_MASTER */

#endif /* TERMTRANS_INCLUDE */


