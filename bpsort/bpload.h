/* bpload */



#ifndef	BPLOAD_INCLUDE
#define	BPLOAD_INCLUDE	1



#include	<sys/types.h>

#include	"localmisc.h"



/* object define */

#define	BPLOAD			struct bpload




/* this is what is put in the loadable module */

struct bpload {
	char	*name ;
	int	size ;
} ;




#endif /* BPLOAD_INCLUDE */



