/* sub */
/* lang=C++11 */

#include	<stdio.h>

extern "C" void	sub_init() ;
extern "C" void	sub_fini() ;
extern "C" int	sub_other() ;

#pragma		init(sub_init)
#pragma		fini(sub_fini)


struct subobj {
	int		a = 1 ;
	subobj() {
		printf("subobj_ctor: ent a=%d\n",a) ;
	} ;
	~subobj() {
		printf("subobj_dtor: ret a=%d\n",a) ;
	} ;
	void update(int aa) { a = aa ; } ;
	int get() { return a ; } ;
} ;

subobj		obj ;

void sub_init() {
	printf("sub_init: ent\n") ;
}

void sub_fini() {
	printf("sub_fini: ent\n") ;
}

int sub_other() {
	printf("sub_other: ent\n") ;
	obj.update(2) ;
	return 0 ;
}


