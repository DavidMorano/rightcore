/* main (times) */


#include	<time.h>

#include	<bfile.h>


extern char	*timestr_buf() ;



int main()
{

	bfile	outfile, *ofp = &outfile ;

	time_t		t = 911730269 ;

	char		buf[100] ;


	(void) bopen(ofp,BFILE_STDOUT,"wct",0666) ;


	bprintf(ofp,"time was %s\n",
		timestr_log(t,buf)) ;


	bclose(ofp) ;

	return 0 ;
}


