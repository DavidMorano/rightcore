/* program to convert magic number to that of the MC20 */


#include	<fcntl.h>

#include	"rel.h"


/* 
	This program converts the input COFF to an output
	COFF with the MC20 magic number.

*/


#define		MAGIC_MC20	0x0150

#define		EOL	'\n'		/* end of line mark */

#define		BUFL	2048



	struct fh {
		unsigned short	f_magic ;
		unsigned short	f_nscns ;
		long int	f_timdat ;
		long int	f_symptr ;
		long int	f_nsyms ;
		unsigned short	f_opthdr ;
		unsigned short	f_flags ;
	} ;


	struct uh {
		short		magic ;
		short		v_stamp ;
		long		tsize ;
		long		d_size ;
		long		b_size ;
		long		entry ;
		long		text_start ;
		long		data_start ;
	} ;




int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	struct fh	st_fh ;

	struct uh	st_uh ;


	register int	c, i, n ;

	int		ifd, ofd ;
	int		len, count ;


	short		buf[BUFL] ;

	char		*bp, obuf[82] ;



	switch (argc) {

case 1:
	ifd = 0 ; ofd = 1 ;

	break ;

case 2:
	ifd = open(argv[1],O_RDONLY,0x0FFF) ;
	if (ifd < 0) return(ifd) ;

	ofd = 1 ;

	break ;

case 3:
default:
	ifd = open(argv[1],O_RDONLY,0x0FFF) ;
	if (ifd < 0) return(ifd) ;

	ofd = open(argv[2],O_WRONLY | O_CREAT | O_TRUNC,0x0FFF) ;
	if (ofd < 0) return(ofd) ;

	break ;

	} ; /* end switch */


/* start by reading in the file header */

	len = read(ifd,&st_fh,sizeof(struct fh)) ;
	if (len <= 0) goto eof ;


	st_fh.f_magic = MAGIC_MC20 ;

	write(ofd,&st_fh,sizeof(struct fh)) ;


loop:
	len = read(ifd,buf,BUFL) ;
	if (len <= 0) goto eof ;

	write(ofd,buf,len) ;

	goto loop ;

eof:
	return (0) ;
}
/* end subroutine (main) */


int debugprint(const char *s)
{
	int	len = 0 ;
	while (s[len]) len++ ;
	return (write(2,s,len)) ;
}


