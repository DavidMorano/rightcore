/* main */


#include	<sys/types.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<randomvar.h>

#include	"srvtab.h"



int main()
{
	bfile	outfile ;

	RANDOMVAR	rv ;

	SRVTAB		st ;

	void	*ep, *endp ;

	uint	uiw ;

	int	rs, i ;
	int	j, k, n ;



	bopen(&outfile,BFILE_STDOUT,"wct",0666) ;

	randomvar_start(&rv,FALSE,0) ;

	srvtab_open(&st,"srvtab",NULL) ;


	for (i = 0 ; i < 1000 ; i += 1) {


		for (j = 0 ; j < 100 ; j += 1) {

			rs = srvtab_check(&st,0L,NULL) ;

				if (rs < 0)
				bprintf(&outfile,"srvtab_check() rs=%d\n",rs) ;

			sleep(1) ;

		} /* end for */


		u_sbrk(0,&endp) ;

		bprintf(&outfile,"endp=%08lx\n",endp) ;

		bflush(&outfile) ;

	} /* end for */


	srvtab_close(&st) ;


	u_sbrk(0,&endp) ;

	bprintf(&outfile,"final endp=%08lx\n",endp) ;


	randomvar_finish(&rv) ;

	bclose(&outfile) ;

	return 0 ;
}



