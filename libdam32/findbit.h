/* findbit */

/* find bit patterns (whatever!) */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	FINDBIT_INCLUDE
#define FINDBIT_INCLUDE		1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int ffbsi(uint) ;
extern int ffbsl(ulong) ;
extern int ffbsll(ulonglong) ;

extern int ffbci(uint) ;
extern int ffbcl(ulong) ;
extern int ffbcll(ulonglong) ;

extern int flbsi(uint) ;
extern int flbsl(ulong) ;
extern int flbsll(ulonglong) ;

extern int flbci(uint) ;
extern int flbcl(ulong) ;
extern int flbcll(ulonglong) ;

extern int fbscounti(uint) ;
extern int fbscountl(ulong) ;
extern int fbscountll(ulonglong) ;

#ifdef	__cplusplus
}
#endif

#endif /* FINDBIT_INCLUDE */


