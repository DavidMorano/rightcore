/* main */


#include	<sys/types.h>

#include	<bfile.h>



/* private module data structures */

struct thing {
	int	a, b ;
} ;




struct thing	sub(a,b)
int	a, b ;
{
	struct thing	this ;

	this.a = a ;
	this.b = b ;
	return this ;
}



