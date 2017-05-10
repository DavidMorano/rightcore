/* cheap */


#ifndef	CHEAP_INCLUDE
#define	CHEAP_INCLUDE	1



#include	<sys/types.h>

#include	"localmisc.h"




#define	CHEAP	struct cheap_head




struct cheap_head {
	ULONG	ow ;
} ;



extern int	cheap_init(CHEAP *,uint) ;
extern int	cheap_free(CHEAP *) ;
extern int	cheap_getulong(CHEAP *,ULONG *) ;


#endif /* CHEAP_INCLUDE */


