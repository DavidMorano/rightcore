/* mainsize */
#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdio.h>

#include	<localmisc.h>

struct termout_gch {
	uint		ch ;
	uint		gr ;
} ;

int main() {
	printf("size=%u\n",sizeof( struct termout_gch)) ;
	return 0 ;
}


