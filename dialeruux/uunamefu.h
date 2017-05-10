/* uunamefu */


/* revision history:

	- 1996-02-01, David A­D­ Morano

	This subroutine was originally written.


*/


#ifndef	UUNAMEFU_INCLUDE
#define	UUNAMEFU_INCLUDE	1


#include	<sys/types.h>

#include	"localmisc.h"


#define	UUNAMEFU		struct uunamefu

#define	UUNAMEFU_VERSION	0



struct uunamefu {
	uint	hfsize ;
	uint	tfsize ;
	uint	wtime ;
	uint	sdnoff ;
	uint	sfnoff ;
	uint	listoff ;
	uint	taboff ;
	uint	tablen ;
	uint	taglen ;
	uint	maxtags ;		/* maximum tags in any list */
	uint	minwlen ;
	uint	maxwlen ;
	char	vetu[4] ;
} ;


extern int uunamefu(UUNAMEFU *,int,char *,int) ;


#endif /* UUNAMEFU_INCLUDE */



