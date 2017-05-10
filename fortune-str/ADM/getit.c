/* main */

/* main subroutine to the "fortune-info" program */


#define	F_DEBUGS	0
#define	F_DEBUG		1


/* revision history :

	= Dave Morano, August 1998
	This subroutine was originally written.


*/


/*******************************************************************

	This program is used to print out information about
	fortune data files.

	Synopsis :

	fortune-info [input_file [outfile]] [-id] [-Vv]


*********************************************************************/



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<fcntl.h>
#include	<time.h>
#include	<ctype.h>
#include	<string.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<bio.h>
#include	<bitops.h>

#include	"misc.h"



/* local defines */

#define		MAXARGINDEX	100
#define		NARGGROUPS	(MAXARGINDEX/8 + 1)
#define		LINELEN		200
#define		BUFLEN		(MAXPATHLEN + (2 * LINELEN))



/* external subroutines */

extern int	optmatch() ;
extern int	cfdec() ;
extern int	procfile() ;


/* forward references */

void	helpfile() ;


/* local structures */


/* global data */



/* local data */

/* define command option words */

static char *argopts[] = {
	"ROOT",
	"DEBUG",
	"VERSION",
	"VERBOSE",
	"HELP",
	NULL,
} ;

#define	ARGOPT_ROOT		0
#define	ARGOPT_DEBUG		1
#define	ARGOPT_VERSION		2
#define	ARGOPT_VERBOSE		3
#define	ARGOPT_HELP		4






int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		infile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		errfile, *efp = &errfile ;

	offset_t	off ;

	int	len ;
	int	c ;

	char	linebuf[LINELEN + 1], *lbp ;


	if (bopen(&infile,BIO_STDIN,"dr",0666) < 0) goto badopen ;

	off = (offset_t) 0020630 ;
	bseek(&infile,(off_t) off,SEEK_SET) ;

	bopen(ofp,BIO_STDOUT,"wctd",0666) ;


	while ((c = bgetc(&infile)) != BR_EOF) {

		if (c == '\0') 
			bprintf(ofp,"%%\n") ;

		else
			bputc(ofp,c) ;

	} /* end while */


	bclose(ofp) ;


	bclose(&infile) ;

	return 0 ;

badopen:
	return BAD ;
}



