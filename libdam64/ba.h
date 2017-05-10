/* ba (BitArray) */


/* revision history:

	= 1998-02-15, David A­D­ Morano

	This code was started.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	BA_INCLUDE
#define	BA_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* local object defines */

#define	BA		struct ba_bitarray
#define	BA_NUM		struct ba_num


struct ba_num {
	int		*num ;
} ;

struct ba_bitarray {
	struct ba_num	*cnp ;
	ULONG		*a ;
	int		nbits ;
	int		nwords ;
} ;


#if	(! defined(BA_MASTER)) || (BA_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ba_start(BA *,BA_NUM *,int) ;
extern void	ba_setones(BA *) ;
extern void	ba_zero(BA *) ;
extern void	ba_countdown(BA *) ;
extern void	ba_and(BA *,BA *) ;
extern void	ba_finish(BA *) ;

/* helpers functions */

extern int	banum_numprepare(BA_NUM *) ;
extern void	banum_numforsake(BA_NUM *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BA_MASTER */

#endif /* BA_INCLUDE */


