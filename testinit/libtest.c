
#include	<envstandards.h>
#include	<stdio.h>

#pragma		init(libtest_init)

void libtest_init() {
	printf("libtest_init: ent\n") ;
}

int libtest_other() {
	printf("libtest_other: ent\n") ;
	return 0 ;
}


