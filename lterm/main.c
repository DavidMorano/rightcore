/* deal with digital line terminations */

/*
	David A.D. Morano
	August 1988
*/

/* 

*/


#include	<envstandards.h>

#include	<fcntl.h>
#include	<time.h>

#include	<bfile.h>
#include	<localmisc.h>

#include	<stdio.h>


#define		DATA_EOF	0x1A

#define		BUFLEN		0x1000
#define		LINELEN		100



	extern int	tty_open(), tty_close(), tty_control(), tty_read() ;
	extern int	tty_aread() ;


int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		error, *efp = &error ;
	bfile		output, *ofp = &output ;

	double		vcc = 5.0, vh = 4.4, vl = 0.6 ;
	double		rp, rg, rt, vt, il, ih, aih, vn, ph ;

	int		rs ;


	rs = bopen(efp,BERR,"wt",0666) ;


	if (argc < 3) {

		bprintf(efp,
		"usage : lterm res_power res_gnd current_high\n") ;


		goto bad_exit ;
	}

#ifdef	COMMENT
	rs = bopen(ofp,BOUT,"wt",0666) ;

	if (rs < 0) goto no_open ;
#endif

/* get the values */

	rs = sscanf(argv[1],"%f",&rp) ;

	rs = sscanf(argv[2],"%f",&rg) ;

	printf("resister to power  = %7.2f\n",rp) ;

	printf("resister to ground = %7.2f\n",rg) ;


	rt = (rp * rg) / (rp + rg) ;

	vt = vcc * rg / (rp + rg) ;

	il = (vt - vl) / rt ;

	ih = (vh - vt) / rt ;

	printf("Rt = %7.2f Ohms\n",rt) ;

	printf("Vt = %7.2f V\n",vt) ;

	printf("Il = %7.2f mA\n",il * 1000.0) ;

	printf("Ih = %7.2f mA\n",ih * 1000.0) ;


/* perform optional calculation */

	if (argc >= 4) {

		rs = sscanf(argv[3],"%f",&aih) ;

		aih /= 1000.0 ;

		if (aih <= ih) ih = aih ;

		printf("\nactual Ih possible = %7.2f mA\n",ih * 1000.0) ;

		vn = vt + (ih * rt) ;

		if (vn > vh) vn = vh ;

		printf("actual net voltage = %7.2f V\n",vn) ;

		ph = (vcc - vn) * ih ;

		printf("logic high power dissipation = %7.2f mW\n",
			ph * 1000.0) ;

	}


exit:
	fflush(stdout) ;

	bclose(efp) ;

	return OK ;

no_open:
	bprintf(efp,"can't open output file %d\n",rs) ;

bad_exit:
	bclose(efp) ;

	return BAD ;
}

