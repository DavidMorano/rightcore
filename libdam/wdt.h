/* wdt */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	WDT_INCLUDE
#define	WDT_INCLUDE	1


/* mode values for call */

#define	WDT_MFOLLOW	1


/* return values */

#define	WDT_ROK		0
#define	WDT_RBADTMP	SR_ACCESS
#define	WDT_RBADTMPOPEN	SR_NOENT
#define	WDT_RBADWRITE	SR_DQUOT


#ifndef	WDT_MASTER

#ifdef	__cplusplus
extern "C" {
#endif

extern int wdt(const char *,int,int (*)(),void *) ;

#ifdef	__cplusplus
}
#endif

#endif /* WDT_MASTER */

#endif /* WDT_INCLUDE */


