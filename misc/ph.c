/* ph */

/* print out standard and option headers on COFFs */


#include	<stdio.h>


/* 
	Print out the file header and the optional UNIX header.

*/


#define		EOL	'\n'		/* end of line mark */

#define		BUFL	2048


/* external subroutines */

extern int debugprint(const char *,int) ;


/* local structures */

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
		short		vstamp ;
		long		tsize ;
		long		dsize ;
		long		bsize ;
		long		entry ;
		long		text_start ;
		long		data_start ;
	} ;


/* exported subroutines */


int main()
{
	struct fh	st_fh ;

	struct uh	st_uh ;


	register int	c, i, n ;

	int		ifd, len, count ;


	short		buf[BUFL] ;

	char		*bp, obuf[82] ;



	ifd = 0 ;


/* start by reading in the file header */

	len = read(ifd,&st_fh,sizeof(struct fh)) ;
	if (len <= 0) goto eof ;

	printf("f_magic		%04X\n",st_fh.f_magic) ;
	printf("f_nscns		%04X\n",st_fh.f_nscns) ;
	printf("f_timdat	%08X\n",st_fh.f_timdat) ;
	printf("f_symptr	%08X\n",st_fh.f_symptr) ;
	printf("f_nsyms		%08X\n",st_fh.f_nsyms) ;
	printf("f_opthdr	%04X\n",st_fh.f_opthdr) ;
	printf("f_flags		%04X\n",st_fh.f_flags) ;



/* now read in the optional standard UNIX header if present */

	if (st_fh.f_opthdr) {

		if (st_fh.f_opthdr == sizeof(struct uh)) {

			len = read(ifd,&st_uh,sizeof(struct uh)) ;
			if (len <= 0) goto eof ;

	printf("magic	%04X\n",st_uh.magic) ;
	printf("vstamp	%04X\n",st_uh.vstamp) ;
	printf("tsize	%08X\n",st_uh.tsize) ;
	printf("dsize	%08X\n",st_uh.dsize) ;
	printf("bsize	%08X\n",st_uh.bsize) ;
	printf("entry	%08X\n",st_uh.entry) ;
	printf("tstart	%08X\n",st_uh.text_start) ;
	printf("dstart	%08X\n",st_uh.data_start) ;


		} else {

			debugprint("optioonal header is not standard UNIX\n") ;

			lseek(0,(int) st_fh.f_opthdr,1) ;

		}


	} /* end if */


eof:
	return (0) ;
}
/* end subroutine (main) */


