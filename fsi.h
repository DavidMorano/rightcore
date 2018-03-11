/* fsi */

/* FIFO-String-Interlocked */


/* revision history:

	= 1994-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Manage interlocked string-FIFO operations.


*******************************************************************************/


#ifndef	FSI_INCLUDE
#define	FSI_INCLUDE	1


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<fifostr.h>
#include	<ptm.h>
#include	<localmisc.h>


#define	FSI		struct fsi


struct fsi {
	FIFOSTR		q ;
	PTM		m ;
} ;


#if	(! defined(FSI_MASTER)) || (FSI_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	fsi_start(FSI *) ;
extern int	fsi_add(FSI *,cchar *,int) ;
extern int	fsi_remove(FSI *,char *,int) ;
extern int	fsi_count(FSI *) ;
extern int	fsi_finish(FSI *) ;

#ifdef	__cplusplus
}
#endif

#endif /* FSI_MASTER */

#endif /* FSI_INCLUDE */


