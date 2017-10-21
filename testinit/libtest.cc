/* libtest */
/* lang=C++11 */

#include	<envstandards.h>
#include	<new>
#include	<utility>
#include	<stdio.h>

using namespace	std ;

extern "C" void	libtest_init() ;
extern "C" void	libtest_fini() ;
extern "C" int	libtest_other() ;

/* this does not work from a shared object */
#pragma		init(libtest_other)

struct libobj {
	int		a = 1 ;
	libobj() {
		printf("libtest/libobj_ctor: ent a=%d\n",a) ;
	} ;
	~libobj() {
		printf("libtest/libobj_dtor: ret a=%d\n",a) ;
	} ;
	int get() {
		return a ;
	} ;
	void update(int aa) {
	    a = aa ;
	} ;
} ;

libobj		obj ;

void libtest_init() {
	printf("libtest_init: ent\n") ;
	new(&obj) libobj ;
}

void libtest_fini() {
	printf("libtest_fini: ent\n") ;
	obj.~libobj() ;
}

int libtest_other() {
	int		a = obj.get() ;
	printf("libtest_other: ent a=%d\n",a) ;
	obj.update(2) ;
	a = obj.get() ;
	printf("libtest_other: ret a=%d\n",a) ;
	return 0 ;
}


