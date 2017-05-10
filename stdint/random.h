/* random (include) */
 
 
/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a total hack. I tried to find out what is supposed to be in here
        and made up guesses (based on standards printed on the web).


*******************************************************************************/


#ifndef _RANDOM_H_
#define _RANDOM_H_	1


#include	<sys/types.h>

#define	GRAND_RANDOM	(1<<1)
#define	GRAND_NONBLOCK	(1<<2)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	getrandom(void *,size_t,unsigned int) ;
extern int	getentropy(void *,size_t) ;

#ifdef	__cplusplus
}
#endif


#endif /* _RANDOM_H_ */


