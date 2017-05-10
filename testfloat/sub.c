

#include	<sys/types.h>
#include	<stdio.h>

#include	"localmisc.h"



union here {
	LONG	lv ;
	double	fv ;
} ;




int sub(fp,v)
FILE	*fp ;
double	v ;
{
	union here	u ;

	LONG	lv ;

	int	n ;


	u.fv = v ;
	n = fprintf(fp,"%016llx\n",u.lv) ;
	
	n = fprintf(fp,"%48.32f\n",u.fv) ;


	return n ;
}



