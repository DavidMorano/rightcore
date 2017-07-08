/* recorder */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	RECORDER_INCLUDE
#define	RECORDER_INCLUDE	1


#include	<sys/types.h>
#include	<localmisc.h>


#define	RECORDER		struct recorder_head
#define	RECORDER_ENT		struct recorder_e
#define	RECORDER_INFO		struct recorder_i
#define	RECORDER_MAGIC		0x12856734
#define	RECORDER_STARTNUM	100	/* starting number records */
#define	RECORDER_NINDICES	5
#define	RECORDER_NCOLLISIONS	10

/* options */

#define	RECORDER_OSEC		(1<<0)
#define	RECORDER_ORANDLC	(1<<1)


struct recorder_i {
	uint		cden[RECORDER_NINDICES][RECORDER_NCOLLISIONS] ;
	uint		c_l1 ;
	uint		c_l3 ;
	uint		c_f ;
	uint		c_fl3 ;
	uint		c_un ;
	uint		ilen ;		/* index length */
} ;

struct recorder_e {
	uint		username ;
	uint		last ;
	uint		first ;
	uint		m1 ;
	uint		m2 ;
} ;

struct recorder_head {
	uint		magic ;
	RECORDER_ENT	*rectab ;
	RECORDER_INFO	s ;
	int		i ;		/* current length */
	int		e ;		/* current buffer extent */
	int		c ;		/* count */
	int		opts ;
} ;


#if	(! defined(RECORDER_MASTER)) || (RECORDER_MASTER == 0)

extern int	recorder_start(RECORDER *,int,int) ;
extern int	recorder_finish(RECORDER *) ;
extern int	recorder_add(RECORDER *,RECORDER_ENT *) ;
extern int	recorder_already(RECORDER *,RECORDER_ENT *) ;
extern int	recorder_gettab(RECORDER *,RECORDER_ENT **) ;
extern int	recorder_rtlen(RECORDER *) ;
extern int	recorder_count(RECORDER *) ;
extern int	recorder_indlen(RECORDER *) ;
extern int	recorder_indsize(RECORDER *) ;
extern int	recorder_mkindun(RECORDER *,const char *,uint (*)[2],int) ;
extern int	recorder_mkindl1(RECORDER *,const char *,uint [][2],int) ;
extern int	recorder_mkindl3(RECORDER *,const char *,uint [][2],int) ;
extern int	recorder_mkindf(RECORDER *,const char *,uint [][2],int) ;
extern int	recorder_mkindfl3(RECORDER *,const char *,uint [][2],int) ;
extern int	recorder_info(RECORDER *,RECORDER_INFO *) ;

#endif /* RECORDER_MASTER */

#endif /* RECORDER_INCLUDE */


