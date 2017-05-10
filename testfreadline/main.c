

#include	<stdio.h>


#define	LINELEN		100




int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	int	rs ;

	char	linebuf[LINELEN + 1] ;


	while ((rs = freadline(stdin,linebuf,LINELEN)) > 0) {

		write(1,linebuf,rs) ;

	}

	return 0 ;
}



