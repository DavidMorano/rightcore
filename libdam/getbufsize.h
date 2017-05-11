/* getbufsize */


/* revision history:

	= 2001-04-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

#ifndef	GETBUFSIZE_INCLUDE
#define	GETBUFSIZE_INCLUDE	1


enum getbufsizes {
	getbufsize_args,
	getbufsize_pw,
	getbufsize_sp,
	getbufsize_ua,
	getbufsize_gr,
	getbufsize_pj,
	getbufsize_pe,
	getbufsize_se,
	getbufsize_ne,
	getbufsize_he,
	getbufsize_overlast
} ;


#if	(! defined(GETBUFSIZE_MASTER)) || (GETBUFSIZE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	getbufsize(int) ;

#ifdef	__cplusplus
}
#endif

#endif /* GETBUFSIZE_MASTER */

#endif /* GETBUFSIZE_INCLUDE */


