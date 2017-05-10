/* main */

/* special sort program */

/*
	David A.D. Morano
	May 1988
*/



#include	<fcntl.h>
#include	<stdio.h>


#include	"localmisc.h"
#include	"bfile.h"



/* local defines */

#define		LINESIZE	200




int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile	in, out, err ;

	int	i, rs, len ;

	char	buf[LINESIZE] ;
	char	obuf[LINESIZE] ;


	rs = bopen(&err,2,"w",0666) ;
	if (rs < 0) return (rs) ;


	switch (argc) {

case 1:
	rs = bopen(&in,0,"r",0666) ;
	if (rs < 0) return (rs) ;

	rs = bopen(&out,1,"w",0666) ;
	if (rs < 0) return (rs) ;

	break ;

case 2:
	rs = bopen(&in,argv[1],"r",0666) ;
	if (rs < 0) return (rs) ;

	rs = bopen(&out,1,"w",0666) ;
	if (rs < 0) return (rs) ;

	break ;

case 3:
default:
	rs = bopen(&in,argv[1],"r",0666) ;
	if (rs < 0) return (rs) ;

	rs = bopen(&out,argv[2],"w",0666) ;
	if (rs < 0) return (rs) ;

	break ;

	} ; /* end switch */




	while (1) {

		len = breadline(&in,buf,LINESIZE) ;
		if (len <= 0) break ;

		if (len < 8) goto error ;

		obuf[6] = buf[0] ;
		obuf[7] = buf[1] ;

		for (i = 0 ; i < 6 ; i += 1) obuf[i] = buf[i + 2] ;

		for (i = 8 ; i < len ; i += 1) obuf[i] = buf[i] ;

		rs = bwrite(&out,obuf,len) ;

		if (rs < 0) {

			printf("bad return after write to out file %d\n",
				rs) ; 

			return BAD ;
		}

	} ;

exit:
	bclose(&in) ;

	bclose(&out) ;

	bclose(&err) ;

	fflush(stdout) ;

	fflush(stderr) ;

	return OK ;

error:
	printf("bad length on input\n") ;

	goto exit ;
}


