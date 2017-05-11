/* vs */

/* virtual system */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VS_INCLUDE
#define	VS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<pthread.h>

#include	<vsystem.h>
#include	<hdb.h>
#include	<vecobj.h>
#include	<pta.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<localmisc.h>

#include	"vstab.h"


/* local defines */

#define	VS_NFDS		20

#define	VSESEM		struct vsesem
#define	VSESEM_IMAGIC	74658392

#define	CSCSB		struct vscsb


struct vsesem {
	int		magic ;
	int		esem ;
} ;

struct vscsb {
	int		cs ;
	int		len ;
} ;

struct vsfd_head {
	int		fd ;
	int		modes ;
} ;

/* main system header */
struct vs_head {
	VSTAB		fds ;	/* file descriptors */
	vecobj		ths ;	/* user threads */
	PTM		pm ;	/* process mutex lock */
	PTC		pcv ;	/* process condition variable */
	int		f_init ;
} ;


/* communication request packet */
struct vscrp_head {
	long		next, prev ;
	pthread_t	tid ;
	ULONG		p1, p2, p3, p4, p5, p6 ;
	void		*intarg ;
	int		(*intsub)() ;
	int		fd ;
	int		fc ;
} ;


#if	(! defined(VS_MASTER)) || (VS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	thrspawn(PTA *,int (*)(),void *,ULONG) ;
extern int	thrjoin(int,int *) ;
extern int	thrid() ;

#ifdef	__cplusplus
}
#endif

#endif /* VS_MASTER */

#endif /* VS_INCLUDE */


