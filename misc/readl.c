/* program to read in names and call shell with names as arguments */

/*
	David A.D. Morano
	September 85
*/

/* 
	This program is used in the following way :

	ls *.s | reads command [infile]

	which will have the effect of :

	command {}		# executed repeatedly

*/


#include	"localmisc.h"

#include	<fcntl.h>

#include	"bfile.h"

#include	<stdio.h>



#define		BUFL		100


extern int	breadline() ;



int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		errorfile, *efp = &errorfile ;
	bfile		input, *ifp = &input ;

	int		len, rs, i, c ;

	char		buf[BUFL], obuf[BUFL] ;

	char		*bp ;


	bopen(efp,2,"w",0666) ;


	if (argc < 2) {

		bprintf(efp,"reads : not enough arguments\n") ;

		bclose(efp) ;

		return (BAD) ;
	} ;


	if (argc >= 3) {

		rs = bopen(&input,argv[2],"r") ;
		if (rs < 0) return (rs) ;

	} else {

		rs = bopen(&input,BIN,"r") ;
		if (rs < 0) return (rs) ;

	} ;



/* start reading in the argument names */


	while ((len = breadline(ifp,buf,BUFL)) > 0) {

				sprintf(obuf,"%s %s\n",argv[1],buf) ;

				system(obuf) ;

	} ;

	return OK ;
}



