/* shobj */


#ifndef	SHOBJ_INCLUDE
#define	SHOBJ_INCLUDE	1


#include	<envstandards.h>
#include	<vecobj.h>


#define	SHOBJ		struct shobj_head
#define	SHOBJ_CALLS	struct shobj_calls


struct shobj_module {
	void		*handle ;
} ;

struct shobj_calls {
	int		(*init)() ;
	int		(*check)() ;
	int		(*free)() ;
} ;

struct shobj_ent {
	void		*op ;
	SHOBJ_CALLS	c ;
	int		osize ;
} ;

struct shobj_head {
	unsigned long	magic ;
	vecobj		entries ;
} ;


#endif /* SHOBJ_INCLUDE */


