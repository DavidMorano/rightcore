/* upload a JEDEC file from Data I/O */


/* 
	This program will read a JEDEC file as it is uploaded
	from the Data I/O Unisite programmer.

	$ dioup file

	$ dioup > file

*/



#include	"stddef.h"

#include	"vsystem.h"

#include	<fcntl.h>

#include	"bfile.h"

#include	<time.h>

#include	<stdio.h>



#define		DATA_EOF	0x1A

#define		BUFLEN		0x1000
#define		LINELEN		100

#define		TERMTIMEOUT	10L



	extern int	bopen(), bclose(), bflush() ;
	extern int	breadline(), bgetc(), bputc() ;

	extern int	tty_open(), tty_close(), tty_control(), tty_read() ;
	extern int	tty_aread() ;

	extern long	time() ;


int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		error, *efp = &error ;
	bfile		output, *ofp = &output ;

	long		before ;

	int		i ;
	int		term_fd, len, rs ;

	char		buf[BUFLEN + 20], *bp ;
	char		linebuf[LINELEN + 20], *lp ;



	rs = bopen(efp,BERR,"wt",0666) ;

	rs = open("/dev/tty",O_RDWR,0666) ;
	if (rs < 0) goto bad_term ;


	term_fd = rs ;

	switch (argc) {

case 0:
case 1:
	rs = bopen(ofp,BOUT,"wt",0666) ;
	if (rs < 0) goto no_open ;

	break ;

case 2:
	rs = bopen(ofp,argv[1],"wt",0666) ;
	if (rs < 0) goto no_open ;

	break ;

case 3:
default:
	rs = bopen(ofp,argv[1],"wt",0666) ;
	if (rs < 0) goto no_open ;

	break ;

	} ; /* end switch */



/* open the terminal for access */

	tty_open(term_fd) ;

	tty_control(fc_setmode,(long) (fm_noecho | fm_rawin)) ;



/* some other initialization */

	lp = linebuf ;

more:
	before = time(0L) ;

	len = tty_aread(0L,buf,(long) BUFLEN,TERMTIMEOUT,0L,0L,0L) ;

	if ((time(0L) > (before + TERMTIMEOUT - 1)) && (len = 0))
		goto timeout ;


/* scan for stuff */

	bp = buf ;
	i = 0 ;


	while (i < len) {

		if (*bp == '\r') {

			bprintf(ofp,"%W\n",linebuf,(int) (lp - linebuf)) ;

			lp = linebuf ;

		} else if (*bp == '\n') {

		} else if (*bp == DATA_EOF) {

			bwrite(ofp,linebuf,(lp - linebuf)) ;

			goto done ;

		} else *lp++ = *bp++ ;

		if ((lp - linebuf) >= LINELEN) {

			bwrite(ofp,linebuf,LINELEN) ;

			lp = linebuf ;
		}

		i += 1 ;
	}

	goto more ;



done:
	tty_close(term_fd) ;

	bclose(ofp) ;


	bprintf(efp,"done\n") ;

	bclose(efp) ;

	return OK ;

timeout:
	bprintf(efp,"dioup : time out waiting on programmer\n") ;

	goto done ;


no_open:
	bprintf(efp,"dioup : can't open output file %d\n",rs) ;

	goto exit ;

bad_term:
	bprintf(efp,"dioup : can't open terminal\n") ;

exit:
	bclose(efp) ;

	return BAD ;

}


