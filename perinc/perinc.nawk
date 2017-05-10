#!/usr/bin/nawk -f

{

	a = $1 ;
	d = $2 ;
  printf("%10.4f\n", abs(a - d) / d) ;
}


