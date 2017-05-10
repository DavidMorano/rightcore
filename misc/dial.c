/* dial */

/* dial out and create a login session */

/*
	= 87/09/01, David A­D­ Morano

*/

/******************************************************************

	This program will dial out on a phone line and
	will then start up a shell on that line.


*********************************************************************/



#include	<unistd.h>
#include	<fcntl.h>
#include	<stdio.h>

#include	<dial.h>


#include	"rel.h"




	static char	*mess[] = {
		"interrupt",
		"dialer hung",
		"no answer",
		"illegal baud rate",
		"can't open ACU device",
		"line problem",
		"can't open L-devices file",
		"device not available",
		"device not known",
		"baud not available",
		"baud not known"
	} ;


	static char	line[] = "isn20" ;





int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	CALL	da ;

	int	pid ;
	int	rc ;
	int	fd, tfd ;


	if (argc < 2) {

		printf("no telephone number specified\n") ;
		return (BAD) ;
	} ;


	if (argc > 1) printf("telephone number %s\n",argv[1]) ;

	if (argc > 2) printf("line device %s\n",argv[2]) ;


	da.speed = 1200 ;		/* high speed */
	da.baud = 1200 ;
	da.line = argv[2] ;
	da.telno = argv[1] ;
	da.modem = 0 ;
	da.attr = 0 ;

	fd = dial(&da) ;


	if (fd < 0) {

		if (fd > -11) printf("dial failed : %s\n",mess[fd + 11]) ;

		else printf("dial failed : rc %d\n",fd) ;

		return (BAD) ;

	} ;

	tfd = fd ;


	fflush(stdout) ;

	close(0) ;

	close(1) ;

	close(2) ;


	if ((pid = fork()) == 0) {


		rc = write(tfd,"can you see this\n",17) ;



	} ;

	undial(tfd) ;

	return (OK) ;
}


