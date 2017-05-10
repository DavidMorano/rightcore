/* main */


#define	CF_INTEGRALTYPE		1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdio.h>

#include	<localmisc.h>


/* local structures */


/* forward references */

static int sub(FILE *,int) ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	int		siw ;

	uint		uiw ;

	char		ch = 0xff ;

	signed char	sch = 0xff ;

	uchar		uch = 0xff ;


#if	CF_INTEGRALTYPE
	siw = (sch & 0xff) ;
#endif

	siw = ch ;
	uiw = ch ;
	fprintf(stdout,"default  siw=%08x uiw=%08x\n",siw,uiw) ;

	siw = sch ;
	uiw = sch ;
	fprintf(stdout,"signed   siw=%08x uiw=%08x\n",siw,uiw) ;

	siw = uch ;
	uiw = uch ;
	fprintf(stdout,"unsigned siw=%08x uiw=%08x\n",siw,uiw) ;


/* coerced */

	uch = sch ;

	siw = uch ;
	uiw = uch ;
	fprintf(stdout,"coerced  siw=%08x uiw=%08x\n",siw,uiw) ;


/* switch test */

	switch ((int) uch) {

	case 0xFF:
		fprintf(stdout,"switched on FF\n") ;
		break ;

	default:
		fprintf(stdout,"switched on DEFAULT\n") ;
		break ;

	} /* end switch */


/* subroutine test */

	sub(stdout,uch) ;


/* combination */

	siw = uch | 0 ;
	uiw = uch | 0 ;

	fprintf(stdout,"combination	siw=%08x uiw=%08x\n",siw,uiw) ;


	fclose(stdout) ;
	
	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int sub(fp,uiw)
FILE	*fp ;
int	uiw ;
{

	fprintf(fp,"subroutine	uiw=%08x\n",uiw) ;

	return 0 ;
}
/* end subroutine (sub) */



