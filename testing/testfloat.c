/* main */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1


/* revision history:

	= 98/06/06, David A­D­ Morano

	Originally written.


*/


/*******************************************************************

	Synopsis:

	testfloat


*********************************************************************/



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/times.h>
#include	<sys/time.h>
#include	<limits.h>
#include	<fcntl.h>
#include	<time.h>
#include	<string.h>
#include	<stdlib.h>

#include	<bfile.h>
#include	<localmisc.h>


/* local defines */





/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	bfile	outfile, *ofp = &outfile ;

	double	v = 0.031415926 ;
	double	v2 = 314.15926 ;
	double	v3 ;


	bopen(ofp,BFILE_STDOUT,"wct",0666) ;

	bprintf(ofp,"here is some stuff\n") ;


	bprintf(ofp,"8.4=%8.4f\n",
		v) ;

	bprintf(ofp,"6.2=%6.2f\n",
		v) ;

	bprintf(ofp,"4.4=%4.4f\n",
		v) ;

	bprintf(ofp,"6.6=%6.6f\n",
		v) ;

	bprintf(ofp,"6.7=%6.7f\n",
		v) ;


	bprintf(ofp,"8.4=%8.4f\n",
		v2) ;

	bprintf(ofp,"6.2=%6.2f\n",
		v2) ;

	bprintf(ofp,"4.4=%4.4f\n",
		v2) ;

	bprintf(ofp,"6.6=%6.6f\n",
		v2) ;

	bprintf(ofp,"6.7=%6.7f\n",
		v2) ;


	v3 = 0.0031415926 ;
	bprintf(ofp,"=%f\n",
		v3) ;

	bprintf(ofp,"0=%0f\n",
		v3) ;

	bprintf(ofp,"1=%1f\n",
		v3) ;

	bprintf(ofp,"2=%2f\n",
		v3) ;

	bprintf(ofp,"3=%3f\n",
		v3) ;

	bprintf(ofp,"4=%4f\n",
		v3) ;

	bprintf(ofp,".0=%.0f\n",
		v3) ;

	bprintf(ofp,".1=%.1f\n",
		v3) ;

	bprintf(ofp,".2=%.2f\n",
		v3) ;

	bprintf(ofp,".3=%.3f\n",
		v3) ;




	bclose(ofp) ;

	return 0 ;
}


