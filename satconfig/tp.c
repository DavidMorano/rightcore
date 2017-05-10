
#include	<bfile.h>

#include	"localmisc.h"

main()
{
	bfile	outfile, *ofp = &outfile ;

	bopen(ofp,BOUT,"wct",0664) ;

	bputc(ofp,'\"') ;

	bprintf(ofp,"this %% is a \" with '\n","hello") ;

	return OK ;
}


