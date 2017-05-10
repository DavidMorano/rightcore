/* main (array) */



#include	<bfile.h>



#define	ROW	4
#define	COL	5



static int	b[ROW][COL] ;




int main()
{
	bfile	outfile, *ofp = &outfile ;

	int	a[ROW][COL] ;
	int	n, i, j ;



	bopen(ofp,BFILE_STDOUT,"wct",0666) ;



	for (i = 0 ; i < (ROW * COL) ; i += 1)
		((int *) a)[i] = i ;

	for (i = 0 ; i < ROW ; i += 1) {

		for (j = 0 ; j < COL ; j += 1)
		bprintf(ofp,"  %2d",a[i][j]) ;

		bprintf(ofp,"\n") ;

	}



#ifdef	COMMENT
	n = 0 ;
	for (i = 0 ; i < 4 ; i += 1)
		for (j = 0 ; j < 4 ; j += 1)
			b[i][j] = n++ ;


	for (i = 0 ; i < 16 ; i += 1)
		bprintf(ofp,"%d\n",((int *) b)[i]) ;
#endif



	bclose(ofp) ;

	return 0 ;
}



