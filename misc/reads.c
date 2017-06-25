/* program to read in names and call shell with names as arguments */

/*
	David A.D. Morano
	September 85
*/

/* 
	This program is used in the following way:

	ls *.s | reads command [infile]

	which will have the effect of :

	command {}		# executed repeatedly

*/



#include	<fcntl.h>
#include	<stdio.h>
#include	"bfile.h"
#include	"localmisc.h"



#define		BUFL	100



int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		errorfile, *efp = &errorfile ;
	bfile		input, *ifp = &input ;

	int		rs, i, c ;

	char		buf[BUFL], obuf[BUFL] ;


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


	while ((c = bgetc(ifp)) != BEOF) {

		if (c == ' ' || c == '\v' || c == '\t' || c == '\n')
			continue ;

		i = 0 ;
		buf[i++] = c ;

		while ((c = bgetc(ifp)) != BEOF) {

			if (c == ' ' || c == '\v' || c == '\t' || c == '\n') {

				buf[i] = 0 ;

				sprintf(obuf,"%s %s\n",argv[1],buf) ;

				/*printf("%s\n",obuf) ; */
				system(obuf) ;

				break ;

			} else buf[i++] = c ;

		} ;
	} ;

	return OK ;
}



