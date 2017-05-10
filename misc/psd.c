/* print section data */

#include	"localmisc.h"

#include	<stdio.h>

#include	<fcntl.h>


/* 
	This program prints out the data in all sections by long
	words in HEX.
*/



#define		BUFL	4096



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

	struct sh {
		char		name[8] ;
		long		physical ;
		long		virtual ;
		long		size ;
		long		raw ;
		long		rel ;
		long		line ;
		short		nrel ;
		short		nline ;
		long		flags ;
	} ;



main(argc,argv)
int	argc ;
char	*argv[] ;
{
	struct fh	st_fh ;

	struct uh	st_uh ;

	struct sh	st_sh[20] ;

	unsigned long	buf[BUFL] ;
	unsigned long	acc ;

	long		total, text_fp ;

	register int	i, len ;

	int		c, n, load_start, load_off ;

	int		j, sn, lenr, try, count, nsh ;

	int		ifd, ofd ;

	short		*swp ;

	char		*bp, obuf[82] ;


/* interpret arguments */


	load_start = 0x8180000 ;

	load_off = load_start ;


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


/* initialize */

	total = 0 ;


/* start by reading in the file header */

	len = read(ifd,&st_fh,sizeof(struct fh)) ;
	if (len <= 0) goto err ;

	nsh = (int) st_fh.f_nscns ;	/* get number of section headers */



/* now read in the optional standard UNIX header if present */

	if (st_fh.f_opthdr) {

		if (st_fh.f_opthdr == sizeof(struct uh)) {

			len = read(ifd,&st_uh,sizeof(struct uh)) ;
			if (len <= 0) goto err ;

		} else {

			debugprint("optioonal header is not standard UNIX\n") ;

			lseek(0,(int) st_fh.f_opthdr,1) ;
		}

	} 


/* now read in section headers */

	if (nsh > 20) {

	debugprint("can't handle more than 20 sections\n") ;
	return (1) ;

	} ;


	for (sn = 0 ; sn < nsh ; sn++) {


	len = read(ifd,&st_sh[sn],sizeof(struct sh)) ;
	if (len <= 0) goto err ;

#ifdef	PRINT
	sprintf(obuf,"section flags %08X\n",st_sh[sn].flags) ;
	debugprint(obuf) ;
#endif

	if (! (st_sh[sn].flags & 0x3)) {

	st_sh[sn].virtual = load_off ;
	load_off += st_sh[sn].size ;

	} ;



	} ; /* end for loop */



/* swap the bytes in each short word of the sections */

	for (sn = 0 ; sn < nsh ; sn++) {

	if (st_sh[sn].raw == 0) continue ; /* skip if no data in section */


/* we have a section with data to swap */

	lenr = st_sh[sn].size ;

	while (lenr) {

		try = (BUFL < lenr) ? BUFL : lenr ;

		len = read(ifd,buf,try) ;
		if (len <= 0) goto err ;

		for (i = 0 ; i < (len >> 2) ; i += 4) {

		printf("%08X:  %08X  %08X  %08X  %08X\n",total + (i*4),
			buf[i],buf[i+1],buf[i+2],buf[i+3]) ;

		} ;

		total += len ;

		lenr -= len ;
	} ;



	} ; /* end for loop */



eof:
	close(ifd) ;

	close(ofd) ;

	fflush(stdout) ;

	return (0) ;

err:
	debugprint("input file too short - output file unusable\n") ;
	return (BAD) ;
}


int debugprint(s)
char	*s ;
{
	int	len ;


	len = 0 ;
	while (s[len]) len++ ;

	return (write(2,s,len)) ;
}


