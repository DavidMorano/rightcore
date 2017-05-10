/* main (unsigned) */


#include	<bfile.h>


int main()
{

	bfile	outfile, *ofp = &outfile ;

	short		s ;

	unsigned short	us ;

	int		i ;

	unsigned int	ui ;


	(void) bopen(ofp,BFILE_STDOUT,"wct",0666) ;


	us = 0xFFFF ;
	ui = us ;
	i = us ;

	bprintf(ofp,"us=%08hX  ui=%08X u=%08X\n",
		us,ui,i) ;


	bclose(ofp) ;

	return 0 ;
}


