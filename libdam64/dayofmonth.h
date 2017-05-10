/* dayofmonth */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	DAYOFMONTH_INCLUDE
#define	DAYOFMONTH_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vechand.h>
#include	<localmisc.h>


#define	DAYOFMONTH_MAGIC	0x99447245
#define	DAYOFMONTH		struct dayofmonth_head
#define	DAYOFMONTH_MON		struct dayofmonth_mon
#define	DAYOFMONTH_NMONS	12


struct dayofmonth_mon {
	signed char	days[6][7] ;
} ;

struct dayofmonth_head {
	uint		magic ;
	DAYOFMONTH_MON	*months[DAYOFMONTH_NMONS] ;
	int		year ;
	int		isdst ;
	int		gmtoff ;
} ;


#if	(! defined(DAYOFMONTH_MASTER)) || (DAYOFMONTH_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int dayofmonth_start(DAYOFMONTH *,int) ;
extern int dayofmonth_lookup(DAYOFMONTH *,int,int,int) ;
extern int dayofmonth_finish(DAYOFMONTH *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DAYOFMONTH_MASTER */

#endif /* DAYOFMONTH_INCLUDE */


